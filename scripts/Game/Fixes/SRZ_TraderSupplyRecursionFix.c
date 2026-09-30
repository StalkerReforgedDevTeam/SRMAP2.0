modded class ARMST_DIALOGS_COMPONENT
{
	protected const int SRZ_SUPPLY_STOCK_CAP = 99;

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

	override protected void AddSupplyToTraderStock(ResourceName supplyPrefabName, int amount)
	{
		if (!Replication.IsServer() || amount <= 0)
			return;

		EnsureStockLoaded();

		string key = GetStockKey(m_Actor, supplyPrefabName);
		int currentStock = 0;

		if (m_mTraderStock.Contains(key))
		{
			currentStock = m_mTraderStock.Get(key);
			if (currentStock < 0)
				return;
		}

		int newStock = Math.Min(currentStock + amount, SRZ_SUPPLY_STOCK_CAP);
		if (newStock == currentStock && m_mTraderStock.Contains(key))
			return;

		m_mTraderStock.Set(key, newStock);
		ARMST_TraderStockFileManager.SaveStockToFile(m_Actor, m_mTraderStock);
		BroadcastStockUpdate(supplyPrefabName, newStock);

		Print("[SRZ SUPPLY] " + m_Actor + " | " + supplyPrefabName + " " + currentStock + " -> " + newStock, LogLevel.NORMAL);
	}
}