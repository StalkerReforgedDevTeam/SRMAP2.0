// ARMST_SURGE_ChatCommand.c
// Surge command handler - add ".surge" to your SRZ_RPChatIntercept.c

modded class ARMST_SURGE_GamemodeComponent
{
    void TriggerSurgePublic()
    {
        StartSurge();
    }
}

modded class SCR_PlayerController
{
    void SRZ_SendSurgeCommand(string command)
    {
        if (Replication.IsServer())
        {
            SRZ_ProcessSurgeCommand(command);
        }
        else
        {
            Rpc(SRZ_RpcAsk_ProcessSurgeCommand, command);
        }
    }
    
    [RplRpc(RplChannel.Reliable, RplRcver.Server)]
    protected void SRZ_RpcAsk_ProcessSurgeCommand(string command)
    {
        SRZ_ProcessSurgeCommand(command);
    }
    
    void SRZ_ProcessSurgeCommand(string text)
    {
        if (!Replication.IsServer())
            return;
        
        int playerId = GetPlayerId();
        
        PlayerManager pm = GetGame().GetPlayerManager();
        if (!pm)
            return;
        
        if (playerId <= 0)
            return;
        
        bool isGm = pm.HasPlayerRole(playerId, EPlayerRole.GAME_MASTER);
        bool isAdmin = pm.HasPlayerRole(playerId, EPlayerRole.ADMINISTRATOR);
        
        if (!isGm && !isAdmin)
        {
            SRZ_RPNet.SendToPlayer(playerId, "GM or Admin access required.");
            return;
        }
        
        ARMST_SURGE_GamemodeComponent surgeComponent = ARMST_SURGE_GamemodeComponent.GetInstance();
        if (surgeComponent)
        {
            surgeComponent.TriggerSurgePublic();
            string playerName = pm.GetPlayerName(playerId);
            Print(string.Format("[ARMST_SURGE] %1 triggered surge via command", playerName), LogLevel.NORMAL);
            SRZ_RPNet.SendToPlayer(playerId, "Surge triggered!");
        }
        else
        {
            SRZ_RPNet.SendToPlayer(playerId, "Surge component not found.");
        }
    }
}