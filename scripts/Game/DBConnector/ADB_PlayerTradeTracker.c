[ComponentEditorProps(category: "ADB/Components/Gamemode", description: "")]
class ADB_PlayerTradeTrackerComponentClass : SCR_BaseGameModeComponentClass {}

class ADB_PlayerTradeTrackerComponent : SCR_BaseGameModeComponent
{
	protected ref ADB_Supabase m_db;
	
    static ref ADB_PlayerTradeTrackerComponent GetInstance()
	{
		ChimeraGame game = GetGame();
		if (!game)
			return null;
		
		BaseGameMode gamemode = game.GetGameMode();
		if (!gamemode)
			return null;
		
		return ADB_PlayerTradeTrackerComponent.Cast(gamemode.FindComponent(ADB_PlayerTradeTrackerComponent));
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
	
	void OnPlayerBuy(IEntity user, ResourceName boughtItem, int buyCount, float totalCost)
	{
		if (!m_db)
			return;
		
		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(user);
		PlayerController controller = GetGame().GetPlayerManager().GetPlayerController(playerId);
		if (!controller)
			return;
		
		ADB_BackendInfoComponent backendInfo = ADB_BackendInfoComponent.Cast(controller.FindComponent(ADB_BackendInfoComponent));
		if (!backendInfo)
			return;
		
		int backend_id = backendInfo.GetBackendId();
		
		auto args = new ADB_DtoRpcArgsInsertPlayerNpcTraderTransaction();
		args.p_player_id = backend_id;
		args.p_resource_path = boughtItem;
		args.p_quantity = buyCount;
		args.p_total_price = totalCost;
		args.p_transaction_type = "BUY";
		m_db.CallFunc("insert_player_npc_trader_transaction", args);
	}
	
	void OnPlayerSell(IEntity user, ResourceName boughtItem, int buyCount, float totalCost)
	{
		if (!m_db)
			return;
		
		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(user);
		PlayerController controller = GetGame().GetPlayerManager().GetPlayerController(playerId);
		if (!controller)
			return;
		
		ADB_BackendInfoComponent backendInfo = ADB_BackendInfoComponent.Cast(controller.FindComponent(ADB_BackendInfoComponent));
		if (!backendInfo)
			return;
		
		int backend_id = backendInfo.GetBackendId();
		
		auto args = new ADB_DtoRpcArgsInsertPlayerNpcTraderTransaction();
		args.p_player_id = backend_id;
		args.p_resource_path = boughtItem;
		args.p_quantity = buyCount;
		args.p_total_price = totalCost;
		args.p_transaction_type = "SELL";
		m_db.CallFunc("insert_player_npc_trader_transaction", args);
	}
}

class ADB_DtoRpcArgsInsertPlayerNpcTraderTransaction : JsonApiStruct
{
	int p_player_id; // Backend ID
	string p_resource_path;
	int p_quantity;
	int p_total_price;
	string p_transaction_type;
	
	void ADB_DtoRpcArgsInsertPlayerNpcTraderTransaction()
	{
		RegAll();
	}
}