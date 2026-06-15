// Restart Timer System - Admin command to show countdown for server restart
// Usage: .rtimer <seconds> - Starts countdown for all players
// Usage: .rtimer stop - Stops the countdown

modded class SCR_PlayerController
{
    void SRZ_SendRTimerCommand(string command)
    {
        if (Replication.IsServer())
            SRZ_ProcessRTimerCommand(command);
        else
            Rpc(SRZ_RpcAsk_ProcessRTimerCommand, command);
    }

    [RplRpc(RplChannel.Reliable, RplRcver.Server)]
    protected void SRZ_RpcAsk_ProcessRTimerCommand(string command)
    {
        SRZ_ProcessRTimerCommand(command);
    }

    void SRZ_ProcessRTimerCommand(string text)
    {
        if (!Replication.IsServer())
            return;

        int playerId = GetPlayerId();

        PlayerManager pm = GetGame().GetPlayerManager();
        if (!pm)
            return;

        bool isAdmin = pm.HasPlayerRole(playerId, EPlayerRole.GAME_MASTER);
        if (!isAdmin)
        {
            SRZ_RPNet.SendToPlayer(playerId, "[SRZ] Admin access required.");
            return;
        }

        string lower = text;
        lower.ToLower();
        lower.Trim();

        // .rtimer stop
        if (lower == ".rtimer stop")
        {
            SRZ_RestartTimerManager mgr = SRZ_RestartTimerManager.GetInstance();
            if (mgr)
                mgr.StopTimer();

            SRZ_RPNet.BroadcastToAll("[SRZ] Restart timer cancelled.");
            return;
        }

        // .rtimer <seconds>
        if (lower.StartsWith(".rtimer "))
        {
            string args = text.Substring(8, text.Length() - 8);
            args.Trim();

            int seconds = args.ToInt();
            if (seconds <= 0)
            {
                SRZ_RPNet.SendToPlayer(playerId, "Usage: .rtimer <seconds> OR .rtimer stop");
                return;
            }

            SRZ_RestartTimerManager mgr = SRZ_RestartTimerManager.GetInstance();
            if (!mgr)
                mgr = new SRZ_RestartTimerManager();

            mgr.StartTimer(seconds);
            SRZ_RPNet.BroadcastToAll(string.Format("[SRZ] Server restart in %1 seconds.", seconds));
            return;
        }

        SRZ_RPNet.SendToPlayer(playerId, "Usage: .rtimer <seconds> OR .rtimer stop");
    }
}

class SRZ_RestartTimerManager
{
    protected static ref SRZ_RestartTimerManager s_Instance;
    protected int m_SecondsRemaining = 0;
    protected bool m_IsRunning = false;

    static SRZ_RestartTimerManager GetInstance()
    {
        return s_Instance;
    }

    void StartTimer(int seconds)
    {
        s_Instance = this;
        m_SecondsRemaining = seconds;
        m_IsRunning = true;
        GetGame().GetCallqueue().CallLater(Tick, 1000, false);
    }

    void StopTimer()
    {
        m_IsRunning = false;
        GetGame().GetCallqueue().Remove(Tick);
    }

    protected void Tick()
    {
        if (!m_IsRunning)
            return;

        m_SecondsRemaining--;

        if (m_SecondsRemaining == 300 || m_SecondsRemaining == 120 ||
            m_SecondsRemaining == 60  || m_SecondsRemaining == 30  ||
            m_SecondsRemaining == 10  || m_SecondsRemaining == 5   ||
            m_SecondsRemaining == 4   || m_SecondsRemaining == 3   ||
            m_SecondsRemaining == 2   || m_SecondsRemaining == 1)
        {
            SRZ_RPNet.BroadcastToAll(string.Format("[SRZ] Server restart in %1 seconds!", m_SecondsRemaining));
        }

        if (m_SecondsRemaining <= 0)
        {
            m_IsRunning = false;
            SRZ_RPNet.BroadcastToAll("[SRZ] Server restarting now!");
            return;
        }

        GetGame().GetCallqueue().CallLater(Tick, 1000, false);
    }
}