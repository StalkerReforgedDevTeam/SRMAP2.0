[BaseContainerProps()]
class ADB_AttributeString
{
	[Attribute()]
	string key;
	
	[Attribute()]
	string value;
	
	static ADB_AttributeString FromDto(ADB_DtoPlayerAttrString dto)
	{
		auto result = new ADB_AttributeString();
		result.key = dto.key;
		result.value = dto.value;
		return result;
	}
}

class ADB_DtoPlayerAttrString : JsonApiStruct
{
	int player_id;
	string key;
	string value;
	
	void ADB_DtoPlayerAttrString()
	{
		RegAll();
	}
}

// This manager is a gamemode component.
// Once a player connects, and has a valid ADB ID assigned to it, it will reach out and fetch the player's attributes.
// Also: every few minutes, it checks for updates and if there is one, it will update the player's attributes.
[BaseContainerProps()]
class ADB_PlayerAttrManagerComponentClass : SCR_BaseGameModeComponentClass {}

class ADB_PlayerAttrManagerComponent : SCR_BaseGameModeComponent
{
	protected ref ADB_Supabase m_db;
	
	// Holds REST callbacks so they don't get garbage collected
	protected ref array<ref RestCallback> m_callbacks;
	
	// How long, in minutes, to wait between polling the database for attributes
	[Attribute("5", UIWidgets.Slider, "How long, in minutes, to wait between each poll", "1 100 1")]
	int pollTime;
	
	void ADB_PlayerAttrManagerComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_callbacks = new array<ref RestCallback>();
	}
	
	override protected void OnPostInit(IEntity owner)
	{
		if (!GetGame().InPlayMode())
			return;
		
		if (!Replication.IsServer())
			return;
		
		ADB_DatabaseManagerComponent dbMan = ADB_DatabaseManagerComponent.Get();
		if (!dbMan)
			return;
		
		dbMan.CreateDatabaseConnector("default", m_db);
		
		ADB_PlayerManagerComponent pm = ADB_PlayerManagerComponent.Cast(owner.FindComponent(ADB_PlayerManagerComponent));
		if (!pm)
		{
			return;
		}
		
		GetGame().GetCallqueue().CallLater(OnTimer, pollTime * 60 * 1000);
	}
	
	override void OnPlayerConnected(int playerId)
	{
		ref ADB_PlayerManagerComponent playerManagerComponent = ADB_PlayerManagerComponent.GetInstance();
		if (!playerManagerComponent)
		{
			return;
		}
		
		ref ADB_BackendInfoComponent backendInfoComponent = playerManagerComponent.GetBackendInfoFromPlayerId(playerId);
		if (!backendInfoComponent)
		{
			return;
		}
		
		// Listen to when this player's backend info has been filled out
		backendInfoComponent.GetInvokerOnBackendInfoReceived().Insert(OnBackendInfoReceived);
	}
	
	void OnBackendInfoReceived(ADB_BackendInfoComponent backendInfoComponent)
	{
		PlayerController playerController = PlayerController.Cast(backendInfoComponent.GetOwner());
		if (!playerController)
		{
			return;
		}
		
		int playerId = playerController.GetPlayerId();
		
		ref RestCallback cb = new RestCallback();
		cb.SetOnSuccess(OnPlayerAttrStringRecvSuccess);
		cb.SetOnError(OnPlayerAttrStringRecvError);
		
		m_callbacks.Insert(cb);
		
		m_db.Select("OD_PlayerAttributes_Strings", "player_id,key,value&player_id=eq." + string.ToString(backendInfoComponent.GetBackendId()), string.Empty, cb);
	}
	
	void OnTimer()
	{
		PlayerManager playerMan = GetGame().GetPlayerManager();
		
		// Gather up all the active player's backend IDs up so we can throw a one shot query to the database
		array<int> backendIds = {};
		array<int> playerIds = {};
		playerMan.GetPlayers(playerIds);
		foreach (int playerId : playerIds)
		{
			// Check if they have a controller available first
			PlayerController playerController = playerMan.GetPlayerController(playerId);
			if (!playerController)
				continue;
			
			// Pull the backend info component so we can fetch the backend ID for querying the database
			ADB_BackendInfoComponent backendInfoComponent = ADB_BackendInfoComponent.Cast(playerController.FindComponent(ADB_BackendInfoComponent));
			if (!backendInfoComponent)
				continue;
			
			backendIds.Insert(backendInfoComponent.GetBackendId());
		}
		
		ref RestCallback cb = new RestCallback();
		cb.SetOnSuccess(OnPlayerAttrStringRecvSuccess);
		cb.SetOnError(OnPlayerAttrStringRecvError);
		
		m_callbacks.Insert(cb);
		
		string url = "player_id,key,value&player_id=in.(" + SCR_StringHelper.Join(",", backendIds) + ")";
		m_db.Select("OD_PlayerAttributes_Strings", url, string.Empty, cb);
		
		// Restart the timer
		GetGame().GetCallqueue().CallLater(OnTimer, pollTime * 60 * 1000);
	}
	
	// We've recv'd the string attributes from the database
	protected void OnPlayerAttrStringRecvSuccess(RestCallback restCb)
	{
		// Stop referencing it in long term memory
		m_callbacks.RemoveItem(restCb);
		
		// Parse the incoming data
		string data = restCb.GetData();	
		ref array<ref ADB_DtoPlayerAttrString> attributes = new array<ref ADB_DtoPlayerAttrString>();
		bool parseOk = ADB_SupabaseSelectArrayResponse<ADB_DtoPlayerAttrString>.Parse(data, attributes);
		if (!parseOk)
		{
			Print("[ADB] Could not parse Supabase select array response for player attributes", LogLevel.ERROR);
			return;
		}
		
		// Because I don't trust SQL `group`ing, we pack attributes per backend ID, then iterate over it later to garantee
		// we've collected all the attributes properly to do a batch update.
		map<int, ref array<ref ADB_DtoPlayerAttrString>> attributesPerBackendId = new map<int, ref array<ref ADB_DtoPlayerAttrString>>();
		foreach (ref ADB_DtoPlayerAttrString attribute : attributes)
		{
			if (!attributesPerBackendId.Contains(attribute.player_id))
				attributesPerBackendId.Insert(attribute.player_id, new array<ref ADB_DtoPlayerAttrString>());
			
			attributesPerBackendId[attribute.player_id].Insert(attribute);
		}
		
		ADB_PlayerManagerComponent playerManager = ADB_PlayerManagerComponent.GetInstance();
		foreach (int backendId, ref array<ref ADB_DtoPlayerAttrString> attribute : attributesPerBackendId)
		{
			int playerId = playerManager.GetPlayerIdFromBackendId(backendId);
			if (playerId == -1)
				continue;
			
			PlayerController controller = GetGame().GetPlayerManager().GetPlayerController(playerId);
			if (!controller)
			{
				return;
			}
			
			ADB_PlayerAttrComponent playerAttrComponent = ADB_PlayerAttrComponent.Cast(controller.FindComponent(ADB_PlayerAttrComponent));
			if (!playerAttrComponent)
			{
				Print("[ADB] Player doesn't have the attribute component", LogLevel.ERROR);
				return;
			}
			
			playerAttrComponent.SetStringAttributesFromDto(attribute);
		}
	}
	
	protected void OnPlayerAttrStringRecvError(RestCallback restCb)
	{
		string data = restCb.GetData();
		Print("[ADB] Recieved error when attempting to request player's string attributes: " + data, LogLevel.ERROR);
		
		m_callbacks.RemoveItem(restCb);
	}
}