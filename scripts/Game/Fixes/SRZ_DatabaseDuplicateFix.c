// Items listed in more than one DB entry: the last entry wins for stats, loot and any trader it lists.
// Older entries only add traders the last entry doesn't mention.
modded class ARMST_EDITOR_GLOBAL_SETTINGS
{
	protected static ref map<ResourceName, ref array<ref ARMST_DATABASE_ITEM>> s_SRZ_ItemEntries = new map<ResourceName, ref array<ref ARMST_DATABASE_ITEM>>();

	override void RegisterItems(ARMST_DATABASE_ITEMS itemsList)
	{
		super.RegisterItems(itemsList);

		array<ref ARMST_DATABASE_ITEM> items = new array<ref ARMST_DATABASE_ITEM>();
		itemsList.GetItemsList(items);

		array<ResourceName> touched = new array<ResourceName>();
		foreach (ARMST_DATABASE_ITEM item : items)
		{
			if (item.m_Prefab.IsEmpty())
				continue;

			SRZ_TrackEntry(item.m_Prefab, item, touched);

			if (item.m_ChildrensPrefabs)
			{
				foreach (ResourceName child : item.m_ChildrensPrefabs)
				{
					if (!child.IsEmpty())
						SRZ_TrackEntry(child, item, touched);
				}
			}
		}

		foreach (ResourceName prefab : touched)
		{
			array<ref ARMST_DATABASE_ITEM> entries = s_SRZ_ItemEntries.Get(prefab);
			if (entries && entries.Count() > 1)
				s_ItemMap.Set(prefab, SRZ_BuildMerged(entries));
		}
	}

	protected void SRZ_TrackEntry(ResourceName prefab, ARMST_DATABASE_ITEM item, array<ResourceName> touched)
	{
		array<ref ARMST_DATABASE_ITEM> entries = s_SRZ_ItemEntries.Get(prefab);
		if (!entries)
		{
			entries = new array<ref ARMST_DATABASE_ITEM>();
			s_SRZ_ItemEntries.Set(prefab, entries);
		}

		if (entries.Find(item) == -1)
			entries.Insert(item);

		if (touched.Find(prefab) == -1)
			touched.Insert(prefab);
	}

	protected ARMST_DATABASE_ITEM SRZ_BuildMerged(array<ref ARMST_DATABASE_ITEM> entries)
	{
		ARMST_DATABASE_ITEM winner = entries[entries.Count() - 1];

		ARMST_DATABASE_ITEM merged = new ARMST_DATABASE_ITEM();
		merged.m_Prefab = winner.m_Prefab;
		merged.m_ChildrensPrefabs = winner.m_ChildrensPrefabs;
		merged.m_ItemStats = winner.m_ItemStats;
		merged.m_ItemConsumable = winner.m_ItemConsumable;
		merged.m_ItemLoot = winner.m_ItemLoot;
		merged.m_ItemArt = winner.m_ItemArt;
		merged.m_SuppplyTrader = winner.m_SuppplyTrader;
		merged.m_ItemTrader = new array<ref ARMST_DATABASE_ITEM_TRADER>();

		map<string, int> rowIndex = new map<string, int>();

		if (winner.m_ItemTrader)
		{
			foreach (ARMST_DATABASE_ITEM_TRADER row : winner.m_ItemTrader)
			{
				string key = string.Format("%1|%2", row.m_Actor, row.m_sTraderCategory);
				if (!rowIndex.Contains(key))
					rowIndex.Set(key, merged.m_ItemTrader.Count());
				merged.m_ItemTrader.Insert(row);
			}
		}

		for (int i = entries.Count() - 2; i >= 0; i--)
		{
			ARMST_DATABASE_ITEM older = entries[i];
			if (!older.m_ItemTrader)
				continue;

			foreach (ARMST_DATABASE_ITEM_TRADER oldRow : older.m_ItemTrader)
			{
				if (oldRow.m_Actor == "ALL")
					continue;

				string oldKey = string.Format("%1|%2", oldRow.m_Actor, oldRow.m_sTraderCategory);
				if (rowIndex.Contains(oldKey))
					continue;

				rowIndex.Set(oldKey, merged.m_ItemTrader.Count());
				merged.m_ItemTrader.Insert(oldRow);
			}
		}

		return merged;
	}
}
