[ComponentEditorProps(category: "ADB/Components/Gamemode", description: "")]
class ADB_PlayerDeathReporterComponentClass : SCR_BaseGameModeComponentClass {}

class ADB_PlayerDeathReporterComponent : SCR_BaseGameModeComponent
{
	protected ref ADB_Supabase m_db;
	
    static ref ADB_PlayerDeathReporterComponent GetInstance()
	{
		ChimeraGame game = GetGame();
		if (!game)
			return null;
		
		BaseGameMode gamemode = game.GetGameMode();
		if (!gamemode)
			return null;
		
		return ADB_PlayerDeathReporterComponent.Cast(gamemode.FindComponent(ADB_PlayerDeathReporterComponent));
	}
	
	protected override void OnPostInit(IEntity owner)
	{
		ADB_DatabaseManagerComponent dbManager = ADB_DatabaseManagerComponent.Get();
		if (!dbManager)
		{
			ADB_Logger.Error("No DB Manager available");
			return;
		}
		
		bool dbCreateOk = dbManager.CreateDatabaseConnector("default", m_db);
		if (!dbCreateOk)
		{
			ADB_Logger.Error("Could not create default database");
			return;
		}
	}

    void ReportDeath(IEntity dead, EDamageType damageType, InstigatorType instigatorType, IEntity instigator, int instigatorPlayerId, string instigatorName, string instigatorFactionKey, int instigatorBackendId)
    {
		if (!m_db)
			return;
		
		// Get the death playerId
		int deadPlayerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(dead);
		if (deadPlayerId == 0)
		{
			ADB_Logger.Error("Could not record player's death as their player ID isn't valid");
			return;
		}
		
		PlayerController deadPlayerController = GetGame().GetPlayerManager().GetPlayerController(deadPlayerId);
		if (!deadPlayerController)
		{
			ADB_Logger.Error("Dead player doesn't have a controller");
			return;
		}
		
		int deadBackendId = -1;
		ADB_BackendInfoComponent backendInfoComponent = ADB_BackendInfoComponent.Cast(deadPlayerController.FindComponent(ADB_BackendInfoComponent));
		if (backendInfoComponent)
		{
			deadBackendId = backendInfoComponent.GetBackendId();
		}
		
		if (deadBackendId == -1)
		{
			ADB_Logger.Error("Could not record player's death as their backend ID isn't valid");
			return;
		}
		
		vector deadTransform[4];
		dead.GetWorldTransform(deadTransform);
		vector instigatorTransform[4];
		if (instigator)
			instigator.GetWorldTransform(instigatorTransform);
		
		ADB_DtoRpcArgsRecordPlayerDeath args = new ADB_DtoRpcArgsRecordPlayerDeath();
		args.p_player_id = deadBackendId;
		args.p_instigator_player_id = instigatorBackendId;
		args.p_instigator_name = instigatorName;
		args.p_position = deadTransform[3];
		args.p_instigator_position = instigatorTransform[3];
		args.p_damage_type = damageType;
		args.p_instigator_type = instigatorType;
		args.p_yaw = deadTransform[2].ToYaw();
		args.p_instigator_yaw = instigatorTransform[2].ToYaw();
		m_db.CallFunc("record_player_death", args);
    }
}

class ADB_DtoRpcArgsRecordPlayerDeath : JsonApiStruct
{
	int p_player_id;
    int p_instigator_player_id;
    string p_instigator_name;
    vector p_position;
    vector p_instigator_position;
    float p_yaw;
    float p_instigator_yaw;
	
    int p_damage_type;
	int p_instigator_type;
	
	void ADB_DtoRpcArgsRecordPlayerDeath()
	{
		RegV("p_player_id");
		RegV("p_instigator_player_id");
		RegV("p_instigator_name");
		RegV("p_yaw");
		RegV("p_instigator_yaw");
		
		RegV("p_damage_type");
		RegV("p_instigator_type");
	}
	
	override void OnPack()
	{
		super.OnPack();
		StoreString("p_position", "(" + p_position[0] + "," + p_position[1] + "," + p_position[2] + ")");
		StoreString("p_instigator_position", "(" + p_instigator_position[0] + "," + p_instigator_position[1] + "," + p_instigator_position[2] + ")");
	}
}