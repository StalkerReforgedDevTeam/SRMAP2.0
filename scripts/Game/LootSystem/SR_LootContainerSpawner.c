class SR_LootSpawnerComponentClass : ScriptComponentClass
{
};

class SR_LootSpawnerComponent : ScriptComponent
{
	[Attribute(desc: "List of prefabs", params: "et")]
	protected ref array<ResourceName> prefabs;

	[Attribute(defvalue: "100", desc: "Chance that the spawner creates a lootable container after respawn timer", params: "0 100")]
	protected int chance;
	
	[Attribute(defvalue: "2", desc: "How many searches before container respawn", params: "0 100")]
	protected int searches;
	
	
	protected IEntity spawnedContainer;
	
	protected SR_LootManagerComponent lootManager;
	
	override void OnPostInit(IEntity owner)
    {
        super.OnPostInit(owner);
        
        // Enable engine event masks (e.g., Frame updates)
        SetEventMask(owner, EntityEvent.INIT);
    }

    // 2. Triggered by EntityEvent.INIT
    override void EOnInit(IEntity owner)
    {
		if (!GetGame().InPlayMode())
			return;
		PrintFormat("SR_LootSpawnerComponent: Initialisation");
		
		super.EOnInit(owner);
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (gameMode)
		{
			lootManager = SR_LootManagerComponent.Cast(gameMode.FindComponent(SR_LootManagerComponent));
			if (!lootManager)
			{
				Print("SR_LootSpawnerComponent: SR_LootManagerComponent not found on GameMode!", LogLevel.ERROR);
			}
		}
		else
		{
			Print("SR_LootSpawnerComponent: Active GameMode not found!", LogLevel.ERROR);
		}
		SpawnLootContainer();
		
    }
	void ProcessContainer() {
		SpawnLootContainer();
	}
	
	int GetSearchAmount() {
		return searches;
	}
	
	protected void SpawnLootContainer() {
		if (spawnedContainer) {
			SCR_EntityHelper.DeleteEntityAndChildren(spawnedContainer);
			spawnedContainer = null;
			PrintFormat("SR_LootSpawnerComponent: Deleting existing container");
		}

		
		if ( Math.RandomIntInclusive(0, 100) >= chance ) {
			SetForRespawn();
			return;
		} 
			
		
		//PrintFormat("SR_LootSpawnerComponent: Creating new container with chance %1", chance);
		ResourceName prefab = prefabs[Math.RandomIntInclusive(0, prefabs.Count() - 1 )];
		EntitySpawnParams params = new EntitySpawnParams();
		params.Transform[3] = GetOwner().GetOrigin();
		Resource resource = Resource.Load(prefab);
		spawnedContainer = GetGame().SpawnEntityPrefab(
			resource,
			GetGame().GetWorld(),
			params
		);
		if (spawnedContainer)
		{
			SR_LootRollComponent lootRoll = SR_LootRollComponent.Cast(spawnedContainer.FindComponent(SR_LootRollComponent));
			if (lootRoll)
			{
				lootRoll.SetRootSpawner(this);
			}
			else
			{
				PrintFormat("SR_LootSpawnerComponent: Spawned entity %1 missing SR_LootRollComponent!", prefab, LogLevel.WARNING);
			}
		};
	}
	
	void SetForRespawn() {
		lootManager.AddToQueue(this);
	}
	
}