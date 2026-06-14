// A component attached to a player controller that tracks balance and syncs to/from the database provider.
[ComponentEditorProps(category: "ADB/Components/PlayerController", description: "Holds player's balance info, and syncs to/from the database provider")]
class ADB_PlayerWalletComponentClass : ScriptComponentClass {}

class ADB_PlayerWalletComponent : ScriptComponent
{
	protected ref ADB_Supabase m_db;
	
	protected override void OnPostInit(IEntity owner)
	{
		if (!GetGame().InPlayMode())
			return;
		
		if (!Replication.IsServer())
			return;
		
		ADB_DatabaseManagerComponent dbManager = ADB_DatabaseManagerComponent.Get();
		if (!dbManager)
		{
			ADB_Logger.Error("No DB Manager available");
			return;
		}
		
		dbManager.CreateDatabaseConnector("default", m_db);
	}
	
	void UpdateBalance(int newBalance)
	{
		if (!m_db)
			return;
		
		// Fetch the backend ID for the player
		ADB_BackendInfoComponent backendInfoComponent = ADB_BackendInfoComponent.Cast(GetOwner().FindComponent(ADB_BackendInfoComponent));
		if (!backendInfoComponent)
		{
			ADB_Logger.Error("No backend info component found, cannot update balance");
			return;
		}

		PlayerController pc = PlayerController.Cast(GetOwner());
		if (!pc)
			return;
		
		int backendId = backendInfoComponent.GetBackendId();
		if (!ADB_BackendInfoComponent.IsValidBackendId(backendId))
		{
			ADB_Logger.Error("Backend ID (" + backendId + ") for pid " + pc.GetPlayerId() + " is invalid");
			return;
		}

		ADB_DtoRpcArgsUpdatePlayerBalance rpcArgs = new ADB_DtoRpcArgsUpdatePlayerBalance(backendId, newBalance);
		m_db.CallFunc("update_player_balance", rpcArgs);
	}
}

class ADB_DtoRpcArgsUpdatePlayerBalance : JsonApiStruct
{
	int p_player_id; // Backend ID
	int p_new_balance;
	
	void ADB_DtoRpcArgsUpdatePlayerBalance(int backendId, int newBalance)
	{
		p_player_id = backendId;
		p_new_balance = newBalance;
		
		RegAll();
	}
}