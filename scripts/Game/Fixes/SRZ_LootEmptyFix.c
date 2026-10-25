// A lot of ARMST tier 1 loot points (clothes, medicine, tools, random) and the hidden trash spots have an empty
// tier set in their config. Nothing in the database has an empty tier, so those crates always came up empty
// ("No valid loot items" in the log). An empty tier is read as TIER_1 here.
// ARMST also lets crates roll 0 items (min -1), so many opened empty. Every crate now gives at least one item.
modded class ARMST_OpenStorageAction
{
	protected static const int SRZ_MIN_ITEMS = 1;

	//------------------------------------------------------------------------------------------------
	override void BuildValidLootItemsFromDatabase()
	{
		ARMST_TieredLootSpawnerComponent tierLoot = ARMST_TieredLootSpawnerComponent.Cast(GetOwner().FindComponent(ARMST_TieredLootSpawnerComponent));
		if (tierLoot && tierLoot.m_LootTiersSpawn)
		{
			bool hasEmpty;
			array<string> cleaned = {};
			foreach (string tier : tierLoot.m_LootTiersSpawn)
			{
				if (tier.IsEmpty())
					hasEmpty = true;
				else
					cleaned.Insert(tier);
			}

			if (hasEmpty)
			{
				if (cleaned.IsEmpty())
					cleaned.Insert("TIER_1");

				tierLoot.m_LootTiersSpawn = cleaned;
			}
		}

		super.BuildValidLootItemsFromDatabase();
	}

	//------------------------------------------------------------------------------------------------
	override protected void Spawn()
	{
		if (m_iMinItemsToSpawn < SRZ_MIN_ITEMS)
			m_iMinItemsToSpawn = SRZ_MIN_ITEMS;

		if (m_iMaxItemsToSpawn < m_iMinItemsToSpawn)
			m_iMaxItemsToSpawn = m_iMinItemsToSpawn;

		super.Spawn();
	}
}
