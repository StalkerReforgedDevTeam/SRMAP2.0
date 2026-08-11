class SR_LootManagerComponentClass : ScriptComponentClass
{
};

class SR_LootManagerComponent : ScriptComponent
{

	[Attribute(defvalue: "1200", desc: "Time required to delete the old container and spawn a new one")]
	protected float respawnDelay;
	
	protected ref array<SR_LootSpawnerComponent> lootSpawners = new array<SR_LootSpawnerComponent>();
	protected ref array<SR_LootSpawnerComponent> emptySpawners = new array<SR_LootSpawnerComponent>();
	
	protected bool timerRunning;
	

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
		if(lootSpawners.Count() <= 0 && timerRunning == false) {
			Print("SR_LootManager: Queue was empty, starting timer");
			StartCooldown();
		}
		if(lootSpawners.Contains(spawner)) {
			PrintFormat("SR_LootManager: lootSpawners already contains %1", spawner, LogLevel.ERROR);
			return;
		}
		
		lootSpawners.Insert(spawner);
		PrintFormat("SR_LootManager:Queue is now %1", lootSpawners.Count(), LogLevel.WARNING);
	}
	
	
	
	
	
	
	void ProcessQueue(){

	
		array<SR_LootSpawnerComponent> processQueue = new array<SR_LootSpawnerComponent>();
	
		foreach (SR_LootSpawnerComponent spawner : lootSpawners)
		{
			if (spawner)
				processQueue.Insert(spawner);
		}
	
		lootSpawners.Clear();

		foreach (SR_LootSpawnerComponent spawner : processQueue)
		{
			PrintFormat(
				"SR_LootManager: Processing respawn spawner %1",
				spawner
			);
	
			spawner.ProcessContainer();
		}
	
	
		if (processQueue.Count() > 0 && timerRunning == false)
		{
			PrintFormat(
				"SR_LootManager: New queue entries detected. Starting next cooldown."
			);
	
			StartCooldown();
		}
		else
		{
			Print("SR_LootManager: Queue empty. No cooldown required.");
		}
	}
	

	
	void StartCooldown()
	{
		if (timerRunning) return;
		timerRunning = true;
		PrintFormat("SR_Lootmanager: Timer started");
		GetGame().GetCallqueue().CallLater(
			ResetCooldown,
			respawnDelay * 1000,
			false
		);
	}

	void ResetCooldown() {
		PrintFormat("SR_LootManager: Restarting timer");
		timerRunning = false;
		ProcessQueue();
	}
	
	
	
	
	
		
}


