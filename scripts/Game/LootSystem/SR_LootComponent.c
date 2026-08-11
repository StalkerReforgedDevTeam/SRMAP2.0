// SR_LootRollComponent.c
//
// Attach to any loot container prefab. Points at a tier + a subset of that
// tier's categories, and rolls independent items into the container.
//
// COMPLEXITY NOTES (see chat for full discussion):
// - BuildItemPool() walks tier->categories->items ONCE (on first use) and
//   flattens everything this container is allowed to roll into one list.
//   Cost: O(C + I) across the allowed categories, paid once, not per roll.
// - RollLoot() only ever touches that flattened pool: O(I_pool) per roll,
//   where I_pool = items in the allowed categories, not the whole config.
// - No per-roll re-scanning of tiers or categories, no summed-weight
//   recompute (chance is an independent 1-100 roll per item, not a
//   relative weight, so there's nothing to sum).
//
// THINGS YOU MUST VERIFY / ADAPT FOR YOUR PROJECT (marked TODO below):
// 1. Resource loading call (BaseContainerTools.LoadContainer + CreateInstanceFromContainer)
//    - this is the standard pattern for BaseContainerProps configs in Reforger,
//      but exact usage can vary by game version. Confirm against a working
//      example elsewhere in your project (or Bohemia's own config loaders).
// 2. Math.RandomIntInclusive bounds - confirm whether your engine version's
//    RandomInt/RandomIntInclusive is inclusive or exclusive on the upper bound.
//    Getting this wrong shifts your chance rolls by a hair, not catastrophic,
//    but worth a quick test.
// 3. SpawnItems() is a stub - wire it up to however your project spawns
//    prefabs into inventory (e.g. SCR_InventoryStorageManagerComponent,
//    or GetGame().SpawnEntityPrefab + manual insertion). I don't know your
//    inventory framework so I left this as a clearly marked TODO rather
//    than guessing at an API and having it silently be wrong.

class SR_LootRollComponentClass : ScriptComponentClass
{
};

class SR_LootRollComponent : ScriptComponent
{
	[Attribute(desc: "Loot config resource (.conf) containing tiers/categories/items", params: "conf")]
	protected ResourceName m_sLootConfig;

	[Attribute(defvalue: "", desc: "Name of the SR_LootTier this container rolls from (must match SR_LootTier.lootTier exactly)")]
	protected string m_sTierName;

	[Attribute(desc: "Names of categories (within the chosen tier) this container is allowed to roll from")]
	protected ref array<string> m_aCategoryNames;

	[Attribute(defvalue: "1", desc: "Minimum unique items guaranteed to spawn in this container")]
	protected int m_iSlotMin;

	[Attribute(defvalue: "4", desc: "Maximum unique items allowed to spawn in this container")]
	protected int m_iSlotMax;
	
	[Attribute(defvalue: "60.0", desc: "Respawn time of loot")]
	protected float m_fLootRespawnTime;


	protected ref array<ref SR_LootItem> m_aPooledItems = new array<ref SR_LootItem>(); // to prevent lag we need to cache the items
	protected bool m_bPoolBuilt = false;
	protected bool m_bLootAvailable = true;
	protected bool m_bSetForRespawn;
	SR_LootSpawnerComponent rootSpawner;
	int m_itimesSearched;


	protected void BuildItemPool()
	{
		if (m_bPoolBuilt)
			return;
		
		m_bPoolBuilt = true; 


		Resource holder = BaseContainerTools.LoadContainer(m_sLootConfig);
		if (!holder)
		{
			Print(string.Format("SR_LootRollComponent: failed to load config '%1'", m_sLootConfig), LogLevel.WARNING);
			return;
		}

		SR_LootConfig config = SR_LootConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(holder.GetResource().ToBaseContainer()));
		if (!config)
			return;
		SR_LootTier tier = FindTier(config, m_sTierName);
		if (!tier)
		{
			Print(string.Format("SR_LootRollComponent: tier '%1' not found in config", m_sTierName), LogLevel.WARNING);
			return;
		}

		foreach (SR_LootCategory category : tier.categories) // Go through each category and check if theyre on the allowed list
		{
			if (m_aCategoryNames.Find(category.category) == -1)
				continue; // this container isnt allowed to roll this category

			foreach (SR_LootItem item : category.items)
			{
				m_aPooledItems.Insert(item); // insert item into the pool
			}
		}
	}

	//------------------------------------------------------------------------------------------
	protected SR_LootTier FindTier(SR_LootConfig config, string tierName)
	{
		foreach (SR_LootTier tier : config.tiers)
		{
			if (tier.lootTier == tierName)
				return tier;
		}
		return null;
	}

	// Fisher-Yates shuffle
	protected void ShuffleArray(notnull array<ref SR_LootItem> arr)
	{
		int count = arr.Count();
		for (int i = count - 1; i > 0; i--)
		{
			int j = Math.RandomIntInclusive(0, i); 
			SR_LootItem temp = arr[i];
			arr[i] = arr[j];
			arr[j] = temp;
		}
	}


	array<ref SR_LootItem> RollLoot()
	{
		array<ref SR_LootItem> results = new array<ref SR_LootItem>();

		if (!m_bPoolBuilt)
			BuildItemPool();

		if (m_aPooledItems.IsEmpty())
			return results;

		ShuffleArray(m_aPooledItems); // We shuffle the pooled items to prevent bias for the first created item on the list


		foreach (SR_LootItem item : m_aPooledItems) // iterate through each item
		{

			int roll = Math.RandomIntInclusive(1, 100);
			if (roll <= item.chance){ // Pass 1, see if the item should be sent to pass 2
				roll = Math.RandomIntInclusive(item.minCount, item.maxCount);
				if(roll > 0) { // pass 2, if the item is generated with amount of 0 dont add it to the pool becuase we cant add 0 of an item
					item.spawnCount = roll; // set the amount of item to add to the loot container
					results.Insert(item); // add item to queue
				}
			}
		}


		if (results.Count() < m_iSlotMin) // If the minimum of amount of items is not met then 
		{
			foreach (SR_LootItem item : m_aPooledItems) // take every item from the cached item set
			{
				if (results.Count() >= m_iSlotMin) // check if the loot queue is at or above the minimum amount of items and if it is break the loop
					break;

				if (results.Find(item) == -1) { // if the item is not located in the spawn loot queue then
					item.spawnCount = Math.RandomIntInclusive(1, item.maxCount); // generate its amount with a minimum of 1
					results.Insert(item); // add it to the queue
				}
			}
		}
		
		ShuffleArray(results); // shuffle the queue again
		
		if (results.Count() > m_iSlotMax)  // check if the queue is bigger than max allowed item size
			results.Resize(m_iSlotMax); // if its bigger then lower the queue to the max allowed size
		
		SpawnItems(results); // call spawn function
		
		if(m_fLootRespawnTime > 0) { // if the loot respawn time is set to more than 0 then start cooldown
			StartLootCooldown();
		}
		
		if(!rootSpawner) {
			Print("SR_LootRollComponent: RootSpawner not found", LogLevel.ERROR);
			return results;
		}
		if( m_itimesSearched >= rootSpawner.GetSearchAmount() ) {
			PrintFormat("SR_LootRollComponent: Container has been searched %1 out of %2 times.", m_itimesSearched, rootSpawner.GetSearchAmount(), LogLevel.NORMAL);
			if(m_bSetForRespawn == true)
				return results;
			
			rootSpawner.SetForRespawn();
			m_bSetForRespawn = true;
			
		} else {
			m_itimesSearched++;
			PrintFormat("SR_LootRollComponent: Search count: %1.", m_itimesSearched, LogLevel.NORMAL);
		}
		
		return results;
	}
	void SetRootSpawner(SR_LootSpawnerComponent spawner) {
		rootSpawner = spawner;
	}
	protected void SpawnItems(notnull array<ref SR_LootItem> items)
	{
		if (!Replication.IsServer()) 
			return;
		
		IEntity owner = GetOwner();
 	
		InventoryStorageManagerComponent invManager = InventoryStorageManagerComponent.Cast(owner.FindComponent(InventoryStorageManagerComponent));
		if (!invManager)
		{
			Print("SR_LootRollComponent: no InventoryStorageManagerComponent found on owner - cannot insert items", LogLevel.WARNING);
			return;
		}
		
		foreach (SR_LootItem item : items)
		{
			if (!item) {
				Print("SR_LootRollComponent: Loot item was not found", LogLevel.WARNING);
				continue;
			}
	
			if (!item.prefab)
			{
				Print("SR_LootRollComponent: Loot item has no prefab", LogLevel.WARNING);
				continue;
			}

		
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
	
	bool IsLootAvailable()
	{
		return m_bLootAvailable;
	}
	
	void StartLootCooldown()
	{
		m_bLootAvailable = false;
	
		GetGame().GetCallqueue().CallLater(
			ResetLoot,
			m_fLootRespawnTime * 1000,
			false
		);
	}
	
	int GetTimeSearched() {
		return m_itimesSearched;
	}
	
	protected void ResetLoot()
	{
		m_bLootAvailable = true;
	
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
		super.EOnInit(owner);
		m_itimesSearched = 1;
		m_bSetForRespawn = false;
		BuildItemPool();
    }

};