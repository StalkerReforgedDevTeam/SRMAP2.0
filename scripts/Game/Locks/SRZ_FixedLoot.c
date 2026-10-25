// Crates and stashes that give one themed loot set instead of random single items.
// A shared list of items that loot sets can pick from. Edit one pool and every set using it changes.
[BaseContainerProps(configRoot: true)]
class SRZ_LootPool
{
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Items in this pool", "et")]
	ref array<ResourceName> m_aItems;

	protected static ref map<ResourceName, ref SRZ_LootPool> s_mCache = new map<ResourceName, ref SRZ_LootPool>();

	//------------------------------------------------------------------------------------------------
	static array<ResourceName> Load(ResourceName poolConf)
	{
		if (poolConf.IsEmpty())
			return null;

		SRZ_LootPool pool;
		if (!s_mCache.Find(poolConf, pool))
		{
			Resource res = BaseContainerTools.LoadContainer(poolConf);
			if (res && res.IsValid())
				pool = SRZ_LootPool.Cast(BaseContainerTools.CreateInstanceFromContainer(res.GetResource().ToBaseContainer()));

			if (!pool)
				Print("[SRZ Loot] Not a loot pool: " + poolConf, LogLevel.WARNING);

			s_mCache.Insert(poolConf, pool);
		}

		if (!pool)
			return null;

		return pool.m_aItems;
	}

	//------------------------------------------------------------------------------------------------
	static void PickOne(array<ResourceName> inlineItems, ResourceName poolConf, notnull array<ResourceName> outItems)
	{
		array<ResourceName> choices = {};
		if (inlineItems)
			choices.InsertAll(inlineItems);

		array<ResourceName> pooled = Load(poolConf);
		if (pooled)
			choices.InsertAll(pooled);

		for (int i = choices.Count() - 1; i >= 0; i--)
		{
			if (choices[i].IsEmpty())
				choices.Remove(i);
		}

		if (!choices.IsEmpty())
			outItems.Insert(choices.GetRandomElement());
	}
}

// An item (or one item from a pool) with its own chance and a random count
[BaseContainerProps()]
class SRZ_LootExtra
{
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Item", "et")]
	ResourceName m_sItem;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Or a random item from this pool, rolled for each copy", "conf")]
	ResourceName m_sPool;

	[Attribute("1", desc: "0 to 1")]
	float m_fChance;

	[Attribute("1", desc: "Fewest copies when it drops")]
	int m_iMin;

	[Attribute("1", desc: "Most copies when it drops")]
	int m_iMax;

	//------------------------------------------------------------------------------------------------
	void Collect(notnull array<ResourceName> outItems)
	{
		if (Math.RandomFloat01() >= m_fChance)
			return;

		int low = Math.ClampInt(m_iMin, 1, 1000);
		int high = Math.ClampInt(m_iMax, low, 1000);
		int count = Math.RandomInt(low, high + 1);
		for (int i = 0; i < count; i++)
		{
			if (!m_sPool.IsEmpty())
				SRZ_LootPool.PickOne(null, m_sPool, outItems);
			else if (!m_sItem.IsEmpty())
				outItems.Insert(m_sItem);
		}
	}
}

[BaseContainerProps()]
class SRZ_LootSet
{
	[Attribute("", desc: "Name, for your own reference")]
	string m_sName;

	[Attribute("10", desc: "Higher = picked more often")]
	int m_iWeight;

	[Attribute(uiwidget: UIWidgets.ComboBox, desc: "Tiers this set can appear in, empty = all", enums: ParamEnumArray.FromEnum(ARMST_LOOT_TIER_SPAWN))]
	ref array<string> m_aTiers;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Always spawned, repeat an entry for more than one", "et")]
	ref array<ResourceName> m_aItems;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "One of these is always added", "et")]
	ref array<ResourceName> m_aOneOf;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Second pick: one of these is always added", "et")]
	ref array<ResourceName> m_aOneOfB;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Third pick: one of these is always added", "et")]
	ref array<ResourceName> m_aOneOfC;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Pool for the first pick", "conf")]
	ResourceName m_sOneOfPool;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Pool for the second pick", "conf")]
	ResourceName m_sOneOfPoolB;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Pool for the third pick", "conf")]
	ResourceName m_sOneOfPoolC;

	[Attribute(desc: "Items with their own chance and count")]
	ref array<ref SRZ_LootExtra> m_aExtras;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Each one rolls Bonus Chance on its own", "et")]
	ref array<ResourceName> m_aBonus;

	[Attribute("0.3", desc: "0 to 1")]
	float m_fBonusChance;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "One of these added with Rare Chance", "et")]
	ref array<ResourceName> m_aRareOneOf;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Pool for the rare pick", "conf")]
	ResourceName m_sRarePool;

	[Attribute("0", desc: "0 to 1")]
	float m_fRareChance;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Pocket filler: each pick takes a random item from a random pool in this list", "conf")]
	ref array<ResourceName> m_aFillerPools;

	[Attribute("0", desc: "Fewest filler picks")]
	int m_iFillerMin;

	[Attribute("0", desc: "Most filler picks")]
	int m_iFillerMax;

	[Attribute("100", desc: "Lowest condition for weapons and clothing, 0 to 100")]
	int m_iConditionMin;

	[Attribute("100", desc: "Highest condition for weapons and clothing, 0 to 100")]
	int m_iConditionMax;

	[Attribute("1", desc: "Lowest magazine fill, 0 to 1")]
	float m_fAmmoMin;

	[Attribute("1", desc: "Highest magazine fill, 0 to 1")]
	float m_fAmmoMax;

	//------------------------------------------------------------------------------------------------
	bool FitsTiers(array<string> tiers)
	{
		if (!m_aTiers || m_aTiers.IsEmpty() || !tiers || tiers.IsEmpty())
			return true;

		foreach (string t : tiers)
		{
			if (m_aTiers.Contains(t))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	void Collect(notnull array<ResourceName> outItems)
	{
		if (m_aItems)
		{
			foreach (ResourceName item : m_aItems)
			{
				if (!item.IsEmpty())
					outItems.Insert(item);
			}
		}

		if ((m_aOneOf && !m_aOneOf.IsEmpty()) || !m_sOneOfPool.IsEmpty())
			SRZ_LootPool.PickOne(m_aOneOf, m_sOneOfPool, outItems);

		if ((m_aOneOfB && !m_aOneOfB.IsEmpty()) || !m_sOneOfPoolB.IsEmpty())
			SRZ_LootPool.PickOne(m_aOneOfB, m_sOneOfPoolB, outItems);

		if ((m_aOneOfC && !m_aOneOfC.IsEmpty()) || !m_sOneOfPoolC.IsEmpty())
			SRZ_LootPool.PickOne(m_aOneOfC, m_sOneOfPoolC, outItems);

		if (m_aExtras)
		{
			foreach (SRZ_LootExtra extra : m_aExtras)
			{
				if (extra)
					extra.Collect(outItems);
			}
		}

		if (m_aBonus)
		{
			foreach (ResourceName bonus : m_aBonus)
			{
				if (!bonus.IsEmpty() && Math.RandomFloat01() < m_fBonusChance)
					outItems.Insert(bonus);
			}
		}

		if (Math.RandomFloat01() < m_fRareChance)
			SRZ_LootPool.PickOne(m_aRareOneOf, m_sRarePool, outItems);

		if (m_aFillerPools && !m_aFillerPools.IsEmpty() && m_iFillerMax > 0)
		{
			int fillLow = Math.ClampInt(m_iFillerMin, 0, 100);
			int fillHigh = Math.ClampInt(m_iFillerMax, fillLow, 100);
			int picks = Math.RandomInt(fillLow, fillHigh + 1);
			for (int i = 0; i < picks; i++)
			{
				SRZ_LootPool.PickOne(null, m_aFillerPools.GetRandomElement(), outItems);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	// Server only. Partial magazines now, worn condition after ARMST has loaded the item's stats.
	void Wear(IEntity item)
	{
		if (!item)
			return;

		if (m_fAmmoMax < 1 || m_fAmmoMin < 1)
		{
			MagazineComponent mag = MagazineComponent.Cast(item.FindComponent(MagazineComponent));
			if (mag)
			{
				int maxAmmo = mag.GetMaxAmmoCount();
				float fillLow = Math.Clamp(m_fAmmoMin, 0, 1);
				float fillHigh = Math.Clamp(m_fAmmoMax, fillLow, 1);
				float fill = Math.RandomFloat(fillLow, fillHigh);
				int ammo = Math.Round(maxAmmo * fill);
				if (maxAmmo > 0)
					mag.SetAmmoCount(Math.ClampInt(ammo, 1, maxAmmo));
			}
		}

		if (m_iConditionMin >= 100 && m_iConditionMax >= 100)
			return;

		if (!item.FindComponent(BaseWeaponComponent) && !item.FindComponent(BaseLoadoutClothComponent))
			return;

		int low = Math.ClampInt(m_iConditionMin, 1, 100);
		int high = Math.ClampInt(m_iConditionMax, low, 100);
		int target = Math.RandomInt(low, high + 1);
		GetGame().GetCallqueue().CallLater(ApplyCondition, 2500, false, item, target);
	}

	//------------------------------------------------------------------------------------------------
	protected void ApplyCondition(IEntity item, int target)
	{
		if (!item || item.IsDeleted())
			return;

		ARMST_ITEMS_STATS_COMPONENTS stats = ARMST_ITEMS_STATS_COMPONENTS.Cast(item.FindComponent(ARMST_ITEMS_STATS_COMPONENTS));
		if (stats && target < stats.GetConditionLevel())
			stats.SetConditionLevel(target);
	}
}

[BaseContainerProps(configRoot: true)]
class SRZ_LootTable
{
	[Attribute()]
	ref array<ref SRZ_LootSet> m_aSets;
}

[ComponentEditorProps(category: "SRZ", description: "Gives one themed loot set from a table")]
class SRZ_FixedLootComponentClass : ScriptComponentClass
{
}

class SRZ_FixedLootComponent : ScriptComponent
{
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Loot table", "conf")]
	protected ResourceName m_sTable;

	[Attribute(desc: "Sets used when no table is given")]
	ref array<ref SRZ_LootSet> m_aSets;

	protected static ref map<ResourceName, ref SRZ_LootTable> s_mTables = new map<ResourceName, ref SRZ_LootTable>();

	//------------------------------------------------------------------------------------------------
	protected array<ref SRZ_LootSet> GetSets()
	{
		if (m_sTable.IsEmpty())
			return m_aSets;

		SRZ_LootTable table;
		if (!s_mTables.Find(m_sTable, table))
		{
			Resource res = BaseContainerTools.LoadContainer(m_sTable);
			if (res && res.IsValid())
				table = SRZ_LootTable.Cast(BaseContainerTools.CreateInstanceFromContainer(res.GetResource().ToBaseContainer()));

			s_mTables.Insert(m_sTable, table);
		}

		if (!table)
			return m_aSets;

		return table.m_aSets;
	}

	//------------------------------------------------------------------------------------------------
	SRZ_LootSet PickSet()
	{
		array<ref SRZ_LootSet> sets = GetSets();
		if (!sets || sets.IsEmpty())
			return null;

		array<string> tiers;
		ARMST_TieredLootSpawnerComponent tierComp = ARMST_TieredLootSpawnerComponent.Cast(GetOwner().FindComponent(ARMST_TieredLootSpawnerComponent));
		if (tierComp)
			tiers = tierComp.m_LootTiersSpawn;

		int total;
		foreach (SRZ_LootSet s : sets)
		{
			if (s.FitsTiers(tiers) && s.m_iWeight > 0)
				total += s.m_iWeight;
		}

		if (total <= 0)
			return null;

		int roll = Math.RandomInt(0, total);
		foreach (SRZ_LootSet s : sets)
		{
			if (!s.FitsTiers(tiers) || s.m_iWeight <= 0)
				continue;

			roll -= s.m_iWeight;
			if (roll < 0)
				return s;
		}

		return null;
	}
}

modded class ARMST_OpenStorageAction
{
	//------------------------------------------------------------------------------------------------
	override void DetermineLootPool()
	{
		if (GetOwner() && GetOwner().FindComponent(SRZ_FixedLootComponent))
			return;

		super.DetermineLootPool();
	}

	//------------------------------------------------------------------------------------------------
	override protected void Spawn()
	{
		SRZ_FixedLootComponent fixed = SRZ_FixedLootComponent.Cast(GetOwner().FindComponent(SRZ_FixedLootComponent));
		if (!fixed)
		{
			super.Spawn();
			return;
		}

		InventoryStorageManagerComponent storageManager = InventoryStorageManagerComponent.Cast(GetOwner().FindComponent(InventoryStorageManagerComponent));
		if (!storageManager)
			return;

		array<IEntity> existing = {};
		storageManager.GetItems(existing);
		if (!existing.IsEmpty())
			return;

		SRZ_LootSet lootSet = fixed.PickSet();
		if (!lootSet)
			return;

		array<ResourceName> prefabs = {};
		lootSet.Collect(prefabs);

		// Plain items first so there are no clothes or bags yet for them to fall into,
		// then clothes and bags go into the crate itself
		array<IEntity> wearables = {};
		foreach (ResourceName prefab : prefabs)
		{
			Resource res = Resource.Load(prefab);
			if (!res.IsValid())
			{
				Print("[SRZ Loot] Missing prefab in set " + lootSet.m_sName + ": " + prefab, LogLevel.WARNING);
				continue;
			}

			IEntity item = GetGame().SpawnEntityPrefab(res);
			if (!item)
				continue;

			if (item.FindComponent(BaseLoadoutClothComponent))
			{
				wearables.Insert(item);
				continue;
			}

			if (!storageManager.TryInsertItem(item))
			{
				Print("[SRZ Loot] No room in " + lootSet.m_sName + " for " + prefab, LogLevel.WARNING);
				SCR_EntityHelper.DeleteEntityAndChildren(item);
				continue;
			}

			lootSet.Wear(item);
		}

		BaseInventoryStorageComponent crateStorage = BaseInventoryStorageComponent.Cast(GetOwner().FindComponent(BaseInventoryStorageComponent));
		foreach (IEntity wearable : wearables)
		{
			bool placed = crateStorage && storageManager.TryInsertItemInStorage(wearable, crateStorage);
			if (!placed)
				placed = storageManager.TryInsertItem(wearable);

			if (!placed)
			{
				SCR_EntityHelper.DeleteEntityAndChildren(wearable);
				continue;
			}

			lootSet.Wear(wearable);
		}
	}
}
