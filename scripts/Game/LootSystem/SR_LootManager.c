class SR_LootManagerComponentClass : ScriptComponentClass
{
};

class SR_LootManagerComponent : ScriptComponent
{

	[Attribute(defvalue: "1200", desc: "Time required to delete the old container and spawn a new one")]
	protected float respawnDelay;
	
	protected ref array<SR_LootSpawnerComponent> lootSpawners = new array<SR_LootSpawnerComponent>();

	override void OnPostInit(IEntity owner)
    {
        super.OnPostInit(owner);
        
        SetEventMask(owner, EntityEvent.INIT);
    }

    override void EOnInit(IEntity owner)
    {
		if (!GetGame().InPlayMode())
			return;
		PrintFormat("SR_LootManager: Initialisation");
		super.EOnInit(owner);
    }
	
	void AddToQueue(SR_LootSpawnerComponent spawner) {
	
		if(!spawner) {
			Print("SR_LootManager: Spawner not found", LogLevel.ERROR);
			return;
		}
		if(lootSpawners.Count() <= 0) {
			Print("SR_LootManager: Queue was empty, starting timer");
			StartCooldown();
		}
		if(lootSpawners.Contains(spawner)) {
			PrintFormat("SR_LootManager: lootSpawners already contains %1", spawner, LogLevel.ERROR);
			return;
		}
		
		lootSpawners.Insert(spawner);
		PrintFormat("SR_LootManager: Spawner %1 was added to the queue, Queue is now %2", spawner, lootSpawners.Count(), LogLevel.WARNING);
	}
	
	void ProcessQueue(){
		PrintFormat("SR_LootManager: Queue Processing started with a queue of %1 items", lootSpawners.Count(), LogLevel.WARNING);

		if(lootSpawners.Count() - 1 < 0) {
			Print("SR_LootManager: Queue empty, stopping checks");
			return;
		}
		foreach( SR_LootSpawnerComponent spawner : lootSpawners) {
			PrintFormat("SR_LootManager: Processing spawner %1, queue is %2", spawner, lootSpawners.Count(), LogLevel.WARNING);

			if(!spawner)
				continue;
			spawner.ProcessContainer();
		}
		lootSpawners.Clear();
		StartCooldown();
	}
	
	void StartCooldown()
	{
		PrintFormat("SR_Lootmanager: Timer started");
		GetGame().GetCallqueue().CallLater(
			ResetCooldown,
			respawnDelay * 1000,
			false
		);
	}

	void ResetCooldown() {
		PrintFormat("SR_LootManager: Restarting timer");
		ProcessQueue();
	}
	
	
	
	
	
		
}


