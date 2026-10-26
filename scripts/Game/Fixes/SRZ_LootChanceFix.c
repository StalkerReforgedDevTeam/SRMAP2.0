// ARMST cast loot chances to int, so anything under 1 became 0 and was then treated as 100.
// Same weighted pick, but with the real decimal chance.
class SRZ_LootChance
{
	static float Weight(ARMST_LootItemWithPrefab item)
	{
		float w = item.LootInfo.m_iProbability;
		if (w <= 0)
			w = 100;

		return w;
	}

	static ARMST_LootItemWithPrefab Pick(array<ref ARMST_LootItemWithPrefab> items)
	{
		if (!items || items.IsEmpty())
			return null;

		float total = 0;
		foreach (ARMST_LootItemWithPrefab item : items)
		{
			total += Weight(item);
		}

		float roll = Math.RandomFloat(0, total);
		float current = 0;
		foreach (ARMST_LootItemWithPrefab item : items)
		{
			current += Weight(item);
			if (roll < current)
				return item;
		}

		return items[items.Count() - 1];
	}
}

modded class ARMST_OpenStorageAction
{
	override ARMST_LootItemWithPrefab GetRandomValidLootItemWithPrefab()
	{
		return SRZ_LootChance.Pick(m_ValidLootItemsWithPrefabs);
	}
}

modded class ARMST_RANDOM_CLOTHES_NEW
{
	override ARMST_LootItemWithPrefab GetRandomValidLootItemWithPrefab()
	{
		return SRZ_LootChance.Pick(m_ValidLootItemsWithPrefabs);
	}

	// ARMST queues these with a delay; the bot can be gone by the time they run
	override protected void EquipRandomItem(IEntity character, SCR_InventoryStorageManagerComponent inventoryManager, array<ref ResourceName> configs, string category)
	{
		if (!character || !inventoryManager)
			return;

		super.EquipRandomItem(character, inventoryManager, configs, category);
	}

	override protected void EquipRandomWeapon(IEntity character, SCR_InventoryStorageManagerComponent inventoryManager, array<ref ResourceName> configs, string category)
	{
		if (!character || !inventoryManager)
			return;

		super.EquipRandomWeapon(character, inventoryManager, configs, category);
	}

	override protected void AddInventoryItems(IEntity character, SCR_InventoryStorageManagerComponent inventoryManager, array<ref ResourceName> configs)
	{
		if (!character || !inventoryManager)
			return;

		super.AddInventoryItems(character, inventoryManager, configs);
	}

	override void ApplyLoadout2(IEntity character)
	{
		if (!character)
			return;

		super.ApplyLoadout2(character);
	}
}
