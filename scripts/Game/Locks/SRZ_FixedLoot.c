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

		if (!choices.IsEmpty())
			outItems.Insert(choices.GetRandomElement());
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
			outItems.InsertAll(m_aItems);

		if ((m_aOneOf && !m_aOneOf.IsEmpty()) || !m_sOneOfPool.IsEmpty())
			SRZ_LootPool.PickOne(m_aOneOf, m_sOneOfPool, outItems);

		if ((m_aOneOfB && !m_aOneOfB.IsEmpty()) || !m_sOneOfPoolB.IsEmpty())
			SRZ_LootPool.PickOne(m_aOneOfB, m_sOneOfPoolB, outItems);

		if ((m_aOneOfC && !m_aOneOfC.IsEmpty()) || !m_sOneOfPoolC.IsEmpty())
			SRZ_LootPool.PickOne(m_aOneOfC, m_sOneOfPoolC, outItems);

		if (m_aBonus)
		{
			foreach (ResourceName bonus : m_aBonus)
			{
				if (Math.RandomFloat01() < m_fBonusChance)
					outItems.Insert(bonus);
			}
		}

		if (Math.RandomFloat01() < m_fRareChance)
			SRZ_LootPool.PickOne(m_aRareOneOf, m_sRarePool, outItems);
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

		foreach (ResourceName prefab : prefabs)
		{
			Resource res = Resource.Load(prefab);
			if (!res.IsValid())
			{
				Print("[SRZ Loot] Missing prefab in set " + lootSet.m_sName + ": " + prefab, LogLevel.WARNING);
				continue;
			}

			IEntity item = GetGame().SpawnEntityPrefab(res);
			if (item && !storageManager.TryInsertItem(item))
				SCR_EntityHelper.DeleteEntityAndChildren(item);
		}
	}
}
