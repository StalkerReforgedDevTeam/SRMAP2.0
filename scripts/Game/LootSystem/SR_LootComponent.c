[BaseContainerProps()]
class SR_CategoryWeight
{
	[Attribute(desc: "Name of category matching the master config (e.g. 'Food', 'Ammo')")]
	string categoryName;

	[Attribute(defvalue: "100", desc: "Weight of this category for this container instance")]
	int weight;

	string GetCategoryName() { return categoryName; }
	int GetWeight() { return weight; }
}

class SR_LootRollComponentClass : ScriptComponentClass
{
};

class SR_LootRollComponent : ScriptComponent
{

	[Attribute(defvalue: "", desc: "Name of the SR_LootTier this container rolls from (must match SR_LootTier.lootTier exactly)")]
	protected string tierName;

	[Attribute(desc: "Names of categories (within the chosen tier) this container is allowed to roll from")]
	protected ref array<ref SR_CategoryWeight> categories;

	[Attribute(defvalue: "1", desc: "Minimum unique items guaranteed to spawn in this container")]
	protected int slotMin;

	[Attribute(defvalue: "4", desc: "Maximum unique items allowed to spawn in this container")]
	protected int slotMax;
	
	[Attribute(defvalue: "60.0", desc: "Respawn time of loot")]
	protected float m_fLootRespawnTime;

	protected bool lootAvailable;
	void CreateLoot() 
	{
		Print("[SR LOOT] CreateLoot called!");
		StartLootCooldown();
		SpawnItems();
	}
	protected void SpawnItems()
	{
		Print("[SR LOOT] Spawnitems called!");
		if (!Replication.IsServer()) 
			return;
		Print("[SR LOOT] isserver passed!");
		IEntity owner = GetOwner();
 		
		
		InventoryStorageManagerComponent invManager = InventoryStorageManagerComponent.Cast(owner.FindComponent(InventoryStorageManagerComponent));
		if (!invManager)
		{
			Print("SR_LootRollComponent: no InventoryStorageManagerComponent found on owner - cannot insert items", LogLevel.WARNING);
			return;
		}
		
		SR_LootManagerComponent lootManager = SR_LootManagerComponent.GetInstance();
		if(!lootManager) 
		{
			Print("[SR LOOT] Loot manger not found");
			return;
		}
		ref array<SR_LootItem> items = new array<SR_LootItem>();
		int slots = Math.RandomIntInclusive(slotMin, slotMax);
		if (slots <= 0) return;
		
		items = lootManager.RequestLoot(tierName, categories, slots);
		Print("[SR LOOT] Items are here!");
		if (!items) return;
		Print("[SR LOOT] Items are correct!");
		foreach (SR_LootItem item : items)
		{
			Print("[SR LOOT] for each started!");
			if (!item) 
			{
				Print("SR_LootRollComponent: Loot item was not found", LogLevel.WARNING);
				continue;
			}
			
			if (!item.prefab)
			{
				Print("SR_LootRollComponent: Loot item has no prefab", LogLevel.WARNING);
				continue;
			}

			Print("[SR LOOT] Spawning!");
			PrintFormat(
				"SR_LootRollComponent: Spawning %1 x%2",
				FilePath.StripPath(item.prefab),
				item.spawnCount
			);
			for (int i = 0; i < item.spawnCount; i++) {
				EntitySpawnParams params = new EntitySpawnParams();
				params.Transform[3] = owner.GetOrigin();
				Resource resource = Resource.Load(item.prefab);
				IEntity spawnedItem = GetGame().SpawnEntityPrefab(
					resource,
					GetGame().GetWorld(),
					params
				);
		
				if (!spawnedItem)
				{
					PrintFormat(
						"SR_LootRollComponent: Failed to spawn %1",
						item.prefab,
						LogLevel.WARNING
					);
					continue;
				}
		

				ARMST_ITEMS_STATS_COMPONENTS stats =
					ARMST_ITEMS_STATS_COMPONENTS.Cast(
						spawnedItem.FindComponent(
							ARMST_ITEMS_STATS_COMPONENTS
						)
					);
		
				if (!stats)
				{
					PrintFormat(
						"SR_LootRollComponent: Failed to find stats component on %1",
						item.prefab,
						LogLevel.WARNING
					);
				}
				else
				{
					stats.SetConditionLevel(Math.RandomFloatInclusive(item.conditionMin, item.conditionMax));
				}

				bool inserted = invManager.TryInsertItem(spawnedItem);
		
				if (!inserted)
				{
					PrintFormat(
						"SR_LootRollComponent: Failed to insert %1 into storage, deleting",
						item.prefab,
						LogLevel.WARNING
					);
		
					SCR_EntityHelper.DeleteEntityAndChildren(spawnedItem);
					continue;
				}
		
				PrintFormat(
					"SR_LootRollComponent: Successfully inserted %1",
					item.prefab
				);
			}
		}
		
	}
	
	void StartLootCooldown()
	{
		lootAvailable = false;
	
		GetGame().GetCallqueue().CallLater(
			ResetLoot,
			m_fLootRespawnTime * 1000,
			false
		);
	}
	
	bool IsLootAvailable() 
	{
		return lootAvailable;
	}
	protected void ResetLoot()
	{
		lootAvailable = true;
		Print("SR_LootRollComponent: Loot is available again");
	}

	override void OnPostInit(IEntity owner)
    {
        super.OnPostInit(owner);
        
        SetEventMask(owner, EntityEvent.INIT);
    }

    override void EOnInit(IEntity owner)
    {
		if (!GetGame().InPlayMode())
			return;
		PrintFormat("SR_LootRollComponent: Initialisation");
		lootAvailable = true;
		super.EOnInit(owner);
    }

};