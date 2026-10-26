// Items with an "ALL" trader row sell at every trader. ARMST added a per trader category filter that hid them
// anywhere the item's category wasn't switched on, so mutant parts only sold at Duty.
modded class ARMST_TRADER_UI
{
	override bool IsItemInTraderCategory(ResourceName prefabName, array<ARMST_TRADER_CATEGORY> traderCategories)
	{
		if (super.IsItemInTraderCategory(prefabName, traderCategories))
			return true;

		ARMST_EDITOR_GLOBAL_SETTINGS db = ARMST_EDITOR_GLOBAL_SETTINGS.GetInstance();
		if (!db)
			return false;

		ARMST_DATABASE_ITEM dbItem = db.FindItemByPrefab(prefabName);
		if (!dbItem || !dbItem.m_ItemTrader)
			return false;

		foreach (ARMST_DATABASE_ITEM_TRADER traderInfo : dbItem.m_ItemTrader)
		{
			if (traderInfo.m_Actor == "ALL" && traderInfo.m_EnableSell)
				return true;
		}

		return false;
	}
}
