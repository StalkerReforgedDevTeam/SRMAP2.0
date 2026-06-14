typedef func ADB_PlayerIdRegistered_Event;
void ADB_PlayerIdRegistered_Event(int playerId, int backendId);
typedef ScriptInvokerBase<ADB_PlayerIdRegistered_Event> ADB_PlayerIdRegistered_Invoker;

class ADB_DtoRpcPlayerJoined : JsonApiStruct
{
	string p_player_guid;
	string p_player_name;
	string p_platform;
	
	void ADB_DtoRpcPlayerJoined()
	{
		RegAll();
	}
}

class ADB_DtoRpcPlayerDisconnected : JsonApiStruct
{
	// Backend ID
	int p_player_id;
	int p_session_id;
	string p_leave_group;
	string p_leave_reason;
	
	void ADB_DtoRpcPlayerDisconnected()
	{
		RegAll();
	}
}

class ADB_DtoRpcPlayerJoinedResponse : JsonApiStruct
{
	// Backend ID
	int player_id;
	int session_id;
	
	void ADB_DtoRpcPlayerJoinedResponse()
	{
		RegAll();
	}
}

[ComponentEditorProps(category: "ADB/Components/Gamemode", description: "Main component involved in maintaining the important backend info for each player controller")]
class ADB_PlayerManagerComponentClass : SCR_BaseGameModeComponentClass {}

class ADB_PlayerManagerComponent : SCR_BaseGameModeComponent
{
	ref ADB_Supabase m_db;
	
	// Maps callback references to Arma player IDs (essentially a crude user data binder)
	ref map<ref RestCallback, int> m_callbacks;
	
	// Maps backend ID to player ID so we can do a backward search
	ref map<int, int> m_backendIdToPlayerIdMap;
	
	void ADB_PlayerManagerComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_callbacks = new map<ref RestCallback, int>();
		m_backendIdToPlayerIdMap = new map<int, int>();
	}
	
	static ref ADB_PlayerManagerComponent GetInstance()
	{
		ChimeraGame game = GetGame();
		if (!game)
			return null;
		
		BaseGameMode gamemode = game.GetGameMode();
		if (!gamemode)
			return null;
		
		ADB_PlayerManagerComponent playerManager = ADB_PlayerManagerComponent.Cast(gamemode.FindComponent(ADB_PlayerManagerComponent));
		
		return playerManager;
	}
	
	override protected void OnPostInit(IEntity owner)
	{
		// Don't attempt an initialization when in editor
		if (!GetGame().InPlayMode())
			return;
		
		if (!Replication.IsServer())
			return;
		
		ADB_DatabaseManagerComponent dbMan = ADB_DatabaseManagerComponent.Get();
		if (!dbMan)
		{
			ADB_Logger.Error("No database manager component attached to the gamemode");
			return;
		}
		
		bool success = dbMan.CreateDatabaseConnector("default", m_db);
		if (!success)
		{
			ADB_Logger.Error(string.Format("Could not create connector from key %1", "default"));
			return;
		}
	}
	
	// When a player manages to log in correctly
	override void OnPlayerAuditSuccess(int playerId)
	{
		if (!Replication.IsServer())
			return;

		if (!m_db)
			return;
		
		// Gather argument values
		ArmaReforgerScripted game = GetGame();
		string playerGuid = game.GetBackendApi().GetPlayerIdentityId(playerId);
		string playerName = SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(playerId);
		PlatformKind platform = game.GetPlayerManager().GetPlatformKind(playerId);
		
		// Create and bind the REST callback
		RestCallback restCb = new RestCallback();
		restCb.SetOnSuccess(OnRpcPlayerJoinedSuccess);
		restCb.SetOnError(OnRpcPlayerJoinedError);
		
		// Bind the data to the callback
		m_callbacks.Insert(restCb, playerId);
		
		// Upsert their GUID to the table
		ADB_DtoRpcPlayerJoined rpcArgs = new ADB_DtoRpcPlayerJoined();
		rpcArgs.p_player_guid = playerGuid;
		rpcArgs.p_player_name = playerName;
		rpcArgs.p_platform = typename.EnumToString(PlatformKind, platform);
#ifdef WORKBENCH
		rpcArgs.p_platform = "WORKBENCH";
#endif
		m_db.CallFunc("player_joined", rpcArgs, restCb);

		ADB_Logger.Info(string.Format("Sent a player joined RPC to the database for player with GUID %1 (pid %2)", playerGuid, playerId));
	}
	
	override void OnPlayerDisconnected(int playerId, KickCauseCode cause, int timeout)
	{
		if (!m_db)
			return;

		ADB_BackendInfoComponent backendInfoComponent = GetBackendInfoFromPlayerId(playerId);
		if (!backendInfoComponent)
		{
			ADB_Logger.Error("No ADB_BackendInfoComponent attached to the player controller for player pid " + playerId);
			return;
		}
		
		int backendId = backendInfoComponent.GetBackendId();
		int sessionId = backendInfoComponent.GetSessionId();
		
		string groupId, reasonId;
		KickCauseGroup2 groupInt;
		int reasonInt;
		GetGame().GetFullKickReason(cause, groupInt, reasonInt, groupId, reasonId);
		
		ADB_DtoRpcPlayerDisconnected rpcArgs = new ADB_DtoRpcPlayerDisconnected();
		rpcArgs.p_player_id = backendId;
		rpcArgs.p_session_id = sessionId;
		rpcArgs.p_leave_group = groupId;
		rpcArgs.p_leave_reason = reasonId;
		m_db.CallFunc("player_disconnected", rpcArgs);
		
		// Remove the mapping
		m_backendIdToPlayerIdMap.Remove(backendId);
	}
	
	protected void OnRpcPlayerJoinedSuccess(RestCallback restCb)
	{
		int playerId = -1;
		if (!m_callbacks.Find(restCb, playerId))
		{
			Print("[ADB] Could not clean up REST callback, potential memory leak", LogLevel.ERROR);
			return;
		}
		
		m_callbacks.Remove(restCb);
		
		string data = restCb.GetData();
		ADB_DtoRpcPlayerJoinedResponse responseDto = new ADB_DtoRpcPlayerJoinedResponse();
		responseDto.ExpandFromRAW(data);
		
		// -1 would mean that it didn't generate
		if (responseDto.player_id == -1)
		{
			return;
		}
		
		// Map the backend ID to the player's ID so we can search backwards too
		m_backendIdToPlayerIdMap.Insert(responseDto.player_id, playerId);
		
		// Fetch the player's controller, and inject the backend ID into it via the player info component
		ADB_BackendInfoComponent backendInfoComponent = GetBackendInfoFromPlayerId(playerId);
		if (!backendInfoComponent)
		{
			Print("[ADB] No ADB_BackendInfoComponent attached to the player controller", LogLevel.ERROR);
			return;
		}
		
		backendInfoComponent.SetInfo(responseDto.player_id, responseDto.session_id);
	}
	
	protected void OnRpcPlayerJoinedError(RestCallback restCb)
	{
		m_callbacks.Remove(restCb);
		Print("[ADB] Could not recover backend ID for player", LogLevel.ERROR);
	}
	
	ref ADB_BackendInfoComponent GetBackendInfoFromPlayerId(int playerId)
	{
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
		if (!playerController)
			return null;
		
		return ADB_BackendInfoComponent.Cast(playerController.FindComponent(ADB_BackendInfoComponent));
	}
	
	int GetPlayerIdFromBackendId(int backendId)
	{
		int result = -1;
		if (!m_backendIdToPlayerIdMap.Find(backendId, result))
			result = -1;
		
		return result;
	}
}