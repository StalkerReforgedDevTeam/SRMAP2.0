modded class ARMST_DIALOGS_COMPONENT
{
	protected const int SRZ_SUPPLY_STOCK_CAP = 99;
	protected const int SRZ_STOCK_SAVE_DELAY_MS = 5000;

	protected ref map<string, int> m_mTraderStockMax = new map<string, int>();
	protected bool m_bInSupplyCascade;
	protected bool m_bSRZ_StockSaveDirty;
	protected bool m_bSRZ_StockSavePending;

	//------------------------------------------------------------------------------------------------
	int GetTraderStockMaxLocal(ResourceName prefabName)
	{
		string key = GetStockKey(m_Actor, prefabName);
		if (m_mTraderStockMax.Contains(key))
			return m_mTraderStockMax.Get(key);
		return -1;
	}

	//------------------------------------------------------------------------------------------------
	override void LoadInitialTraderStock()
	{
		if (!Replication.IsServer())
		{
			return;
		}
		if (m_Actor.IsEmpty())
		{
			return;
		}
		m_mTraderStock.Clear();
		m_mTraderStockMax.Clear();

		bool fileExisted = ARMST_TraderStockFileManager.StockFileExists(m_Actor);
		bool addedNewEntries = false;

		if (fileExisted)
		{
			ref map<ResourceName, int> fileStock = ARMST_TraderStockFileManager.LoadStockFromFile(m_Actor);
			foreach (ResourceName prefab, int count : fileStock)
			{
				m_mTraderStock.Set(prefab, count);
			}
			Print("[ARMST TRADER] Кэш загружен из файла: " + m_mTraderStock.Count() + " товаров", LogLevel.NORMAL);
		}

		ARMST_EDITOR_GLOBAL_SETTINGS db = ARMST_EDITOR_GLOBAL_SETTINGS.GetInstance();
		if (db)
		{
			array<ref array<ref ResourceName>> allCategories = {
				m_sTraderCategory1, m_sTraderCategory2, m_sTraderCategory3,
				m_sTraderCategory4, m_sTraderCategory5, m_sTraderCategory6,
				m_sTraderCategory7, m_sTraderCategory8, m_sTraderCategory9,
				m_sTraderCategory10, m_sTraderCategory11
			};

			foreach (array<ref ResourceName> category : allCategories)
			{
				if (!category || category.IsEmpty())
					continue;

				foreach (ResourceName configResource : category)
				{
					if (configResource.IsEmpty() || !configResource.StartsWith("{"))
						continue;

					ARMST_DATABASE_ITEM dbItem = db.FindItemByPrefab(configResource);
					if (!dbItem || !dbItem.m_ItemTrader)
						continue;

					ARMST_DATABASE_ITEM_TRADER specificTraderInfo = null;
					ARMST_DATABASE_ITEM_TRADER allTraderInfo = null;
					foreach (ARMST_DATABASE_ITEM_TRADER traderInfo : dbItem.m_ItemTrader)
					{
						if (traderInfo.m_Actor == m_Actor)
						{
							specificTraderInfo = traderInfo;
							break;
						}
						else if (traderInfo.m_Actor == "ALL")
						{
							allTraderInfo = traderInfo;
						}
					}

					ARMST_DATABASE_ITEM_TRADER selectedTraderInfo = specificTraderInfo;
					if (!selectedTraderInfo)
						selectedTraderInfo = allTraderInfo;

					if (!selectedTraderInfo || selectedTraderInfo.m_fTraderCount < 0)
						continue;

					int configuredMax = (int)selectedTraderInfo.m_fTraderCount;
					m_mTraderStockMax.Set(configResource, configuredMax);

					if (!m_mTraderStock.Contains(configResource))
					{
						m_mTraderStock.Set(configResource, configuredMax);
						addedNewEntries = true;
					}
				}
			}
		}

		ref array<string> keysToRemove = new array<string>();
		foreach (string existingKey, int existingCount : m_mTraderStock)
		{
			if (!m_mTraderStockMax.Contains(existingKey))
				keysToRemove.Insert(existingKey);
		}
		foreach (string removeKey : keysToRemove)
		{
			m_mTraderStock.Remove(removeKey);
			addedNewEntries = true; // reuse this flag to mean "cache changed, needs re-save"
			Print("[ARMST TRADER] Позиция больше не ограничена в БД, снята с учёта: " + removeKey, LogLevel.NORMAL);
		}

		if (!fileExisted)
		{
			ARMST_TraderStockFileManager.CreateInitialStockFile(m_Actor, m_mTraderStock);
			Print("[ARMST TRADER] Создан файл: " + m_mTraderStock.Count() + " товаров", LogLevel.NORMAL);
		}
		else if (addedNewEntries)
		{
			ARMST_TraderStockFileManager.SaveStockToFile(m_Actor, m_mTraderStock);
			Print("[ARMST TRADER] Файл обновлён по текущей БД: итого " + m_mTraderStock.Count() + " товаров", LogLevel.NORMAL);
		}

		m_bStockLoaded = true;
	}

	//------------------------------------------------------------------------------------------------
	override void ChangeTraderStock(ResourceName prefabName, int delta)
	{
		if (!Replication.IsServer())
			return;

		EnsureStockLoaded();

		string key = GetStockKey(m_Actor, prefabName);
		int currentStock = GetTraderStockLocal(prefabName);

		if (currentStock != -1)
		{
			int newStock = currentStock + delta;
			if (newStock < 0)
				newStock = 0;

			if (m_mTraderStockMax.Contains(key))
			{
				int maxStock = m_mTraderStockMax.Get(key);
				if (newStock > maxStock)
					newStock = maxStock;
			}

			if (newStock != currentStock)
			{
				m_mTraderStock.Set(key, newStock);
				SRZ_ScheduleStockSave();
				BroadcastStockUpdate(prefabName, newStock);
				Print("[ARMST TRADER] Stock: " + prefabName + " " + currentStock + " -> " + newStock, LogLevel.NORMAL);
			}
		}

		if (delta > 0 && !m_bInSupplyCascade)
		{
			m_bInSupplyCascade = true;
			ProcessSupplyItems(prefabName, delta);
			m_bInSupplyCascade = false;
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void ProcessSupplyItems(ResourceName mainPrefabName, int soldCount)
	{
		if (!Replication.IsServer() || soldCount <= 0)
			return;

		ARMST_EDITOR_GLOBAL_SETTINGS db = ARMST_EDITOR_GLOBAL_SETTINGS.GetInstance();
		if (!db)
			return;

		ARMST_DATABASE_ITEM dbItem = db.FindItemByPrefab(mainPrefabName);
		if (!dbItem)
			return;

		array<ref ARMST_DATABASE_ITEM_SUPPLY> supplyList = new array<ref ARMST_DATABASE_ITEM_SUPPLY>();
		dbItem.GetItemSupplyList(supplyList);
		if (supplyList.IsEmpty())
			return;

		foreach (ARMST_DATABASE_ITEM_SUPPLY supply : supplyList)
		{
			if (!supply || !supply.m_SupplyPrefabs)
				continue;

			int supplyCount = supply.m_fCountSupply;
			if (supplyCount <= 0)
				continue;

			foreach (ResourceName supplyPrefab : supply.m_SupplyPrefabs)
			{
				if (supplyPrefab.IsEmpty() || supplyPrefab == mainPrefabName)
					continue;

				AddSupplyToTraderStock(supplyPrefab, supplyCount * soldCount);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void AddSupplyToTraderStock(ResourceName supplyPrefabName, int amount)
	{
		if (!Replication.IsServer() || amount <= 0)
			return;

		EnsureStockLoaded();

		string key = GetStockKey(m_Actor, supplyPrefabName);
		if (!m_mTraderStock.Contains(key))
			return;

		int currentStock = m_mTraderStock.Get(key);
		if (currentStock < 0)
			return;

		int cap = SRZ_SUPPLY_STOCK_CAP;
		if (m_mTraderStockMax.Contains(key))
			cap = m_mTraderStockMax.Get(key);

		int newStock = Math.Min(currentStock + amount, cap);
		if (newStock <= currentStock)
			return;

		m_mTraderStock.Set(key, newStock);
		SRZ_ScheduleStockSave();
		BroadcastStockUpdate(supplyPrefabName, newStock);

		Print("[SRZ SUPPLY] " + m_Actor + " | " + supplyPrefabName + " " + currentStock + " -> " + newStock, LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	protected void SRZ_ScheduleStockSave()
	{
		m_bSRZ_StockSaveDirty = true;

		if (m_bSRZ_StockSavePending)
			return;

		m_bSRZ_StockSavePending = true;
		GetGame().GetCallqueue().CallLater(SRZ_DoDeferredStockSave, SRZ_STOCK_SAVE_DELAY_MS, false);
	}

	//------------------------------------------------------------------------------------------------
	protected void SRZ_DoDeferredStockSave()
	{
		m_bSRZ_StockSavePending = false;

		if (!m_bSRZ_StockSaveDirty)
			return;

		m_bSRZ_StockSaveDirty = false;
		ARMST_TraderStockFileManager.SaveStockToFile(m_Actor, m_mTraderStock);
	}
}