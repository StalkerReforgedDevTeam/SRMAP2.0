// ARMST cast loot chances to int, so anything under 1 became 0 and was then treated as 100.
// Same weighted pick, but with the real decimal chance.
// Items set to 0 never spawn, and the reroll loop gives up after too many misses
// (a pool of near zero chances used to spin forever and freeze the server).
class SRZ_LootChance
{
	static const int MAX_MISSES = 5000;

	static float Weight(ARMST_LootItemWithPrefab item)
	{
		if (!item || !item.LootInfo)
			return 0;

		float w = item.LootInfo.m_iProbability;
		if (w < 0)
			w = 0;

		return w;
	}

	static ARMST_LootItemWithPrefab Pick(array<ref ARMST_LootItemWithPrefab> items, int misses)
	{
		if (!items || items.IsEmpty() || misses >= MAX_MISSES)
			return null;

		float total = 0;
		foreach (ARMST_LootItemWithPrefab item : items)
		{
			total += Weight(item);
		}

		if (total <= 0)
			return null;

		float roll = Math.RandomFloat(0, total);
		float current = 0;
		foreach (ARMST_LootItemWithPrefab item : items)
		{
			float w = Weight(item);
			if (w <= 0)
				continue;

			current += w;
			if (roll < current)
				return item;
		}

		for (int i = items.Count() - 1; i >= 0; i--)
		{
			if (Weight(items[i]) > 0)
				return items[i];
		}

		return null;
	}
}

modded class ARMST_OpenStorageAction
{
	protected int m_iSRZ_Misses;

	override protected void Spawn()
	{
		m_iSRZ_Misses = 0;
		super.Spawn();
	}

	override bool AddToLoot(float probability)
	{
		bool hit = super.AddToLoot(probability);
		if (hit)
			m_iSRZ_Misses = 0;
		else
			m_iSRZ_Misses++;

		return hit;
	}

	override ARMST_LootItemWithPrefab GetRandomValidLootItemWithPrefab()
	{
		return SRZ_LootChance.Pick(m_ValidLootItemsWithPrefabs, m_iSRZ_Misses);
	}
}

modded class ARMST_RANDOM_CLOTHES_NEW
{
	protected int m_iSRZ_Misses;

	override void SpawnLootToLoadout(IEntity character)
	{
		m_iSRZ_Misses = 0;
		super.SpawnLootToLoadout(character);
	}

	override bool AddToLoot(float probability)
	{
		bool hit = super.AddToLoot(probability);
		if (hit)
			m_iSRZ_Misses = 0;
		else
			m_iSRZ_Misses++;

		return hit;
	}

	override ARMST_LootItemWithPrefab GetRandomValidLootItemWithPrefab()
	{
		return SRZ_LootChance.Pick(m_ValidLootItemsWithPrefabs, m_iSRZ_Misses);
	}
}
