// scripts/Game/Fixes/SRZ_TraderSupplyRecursionFix.c
//
// Fixes: ChangeTraderStock() -> ProcessSupplyItems() -> ChangeTraderStock() has
// no cycle detection. If your item database ever ends up with a supply loop
// (even an indirect one through several items), this is infinite recursion and
// a hard crash the instant someone sells the triggering item. Adds a guard set
// that tracks prefabs already being processed in the current top-level call.
// Also strips the leftover Print("2") / Print(supply) / Print(supplyCount) /
// Print(supplyPrefab) debug lines that spam the log on every supply sale.

modded class ARMST_DIALOGS_COMPONENT
{
	protected ref set<ResourceName> m_aSupplyChainGuard;

	override void ChangeTraderStock(ResourceName prefabName, int delta)
	{
		if (!Replication.IsServer())
			return;

		if (!m_aSupplyChainGuard)
			m_aSupplyChainGuard = new set<ResourceName>();

		bool isTopLevelCall = m_aSupplyChainGuard.IsEmpty();

		EnsureStockLoaded();

		string key = GetStockKey(m_Actor, prefabName);
		int currentStock = GetTraderStockLocal(prefabName);
		int newStock = currentStock + delta;
		if (newStock < 0)
			newStock = 0;

		if (currentStock == -1)
		{
			if (delta > 0 && !m_aSupplyChainGuard.Contains(prefabName))
			{
				m_aSupplyChainGuard.Insert(prefabName);
				ProcessSupplyItems(prefabName, delta);
			}
		}
		else
		{
			m_mTraderStock.Set(key, newStock);
			ARMST_TraderStockFileManager.SaveStockToFile(m_Actor, m_mTraderStock);
			BroadcastStockUpdate(prefabName, newStock);
			Print("[ARMST TRADER] Stock: " + prefabName + " " + currentStock + " -> " + newStock, LogLevel.NORMAL);

			if (delta > 0 && !m_aSupplyChainGuard.Contains(prefabName))
			{
				m_aSupplyChainGuard.Insert(prefabName);
				ProcessSupplyItems(prefabName, delta);
			}
		}

		if (isTopLevelCall)
			m_aSupplyChainGuard.Clear();
	}

	override protected void ProcessSupplyItems(ResourceName mainPrefabName, int soldCount)
	{
		ARMST_EDITOR_GLOBAL_SETTINGS db = ARMST_EDITOR_GLOBAL_SETTINGS.GetInstance();
		if (!db)
		{
			Print("[ARMST TRADER] Ошибка: ARMST_EDITOR_GLOBAL_SETTINGS не найден.", LogLevel.ERROR);
			return;
		}

		ARMST_DATABASE_ITEM dbItem = db.FindItemByPrefab(mainPrefabName);
		if (!dbItem)
		{
			Print("[ARMST TRADER] Предмет не найден в БД: " + mainPrefabName, LogLevel.WARNING);
			return;
		}

		array<ref ARMST_DATABASE_ITEM_SUPPLY> supplyList = new array<ref ARMST_DATABASE_ITEM_SUPPLY>();
		dbItem.GetItemSupplyList(supplyList);
		if (supplyList.Count() > 0)
		{
			foreach (ARMST_DATABASE_ITEM_SUPPLY supply : supplyList)
			{
				if (!supply)
					continue;

				int supplyCount = supply.m_fCountSupply;
				if (supplyCount == 0)
					continue;

				foreach (ResourceName supplyPrefab : supply.m_SupplyPrefabs)
				{
					if (supplyPrefab.IsEmpty())
						continue;

					int totalSupplyToAdd = supplyCount * soldCount;

					Print("[ARMST TRADER] Supply: добавляем " + supplyPrefab + " x" + totalSupplyToAdd.ToString() + " (от продажи " + mainPrefabName + " x" + soldCount.ToString() + ")", LogLevel.NORMAL);

					ChangeTraderStock(supplyPrefab, totalSupplyToAdd);
				}
			}
		}
	}
}