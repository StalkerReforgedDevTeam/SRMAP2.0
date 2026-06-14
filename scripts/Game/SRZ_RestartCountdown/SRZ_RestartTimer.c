// Restart Timer System - Admin command to show countdown for server restart
// Usage: .rtimer <seconds> - Starts countdown for all players
// Usage: .rtimer stop - Stops the countdown

modded class SCR_PlayerController
{
    // Client sends rtimer command to server
    void SRZ_SendRTimerCommand(string command)
    {
        if (Replication.IsServer())
        {
            SRZ_ProcessRTimerCommand(command);
        }
        else
        {
            Rpc(SRZ_RpcAsk_ProcessRTimerCommand, command);
        }
    }
    
    [RplRpc(RplChannel.Reliable, RplRcver.Server)]
    protected void SRZ_RpcAsk_ProcessRTimerCommand(string command)
    {
        SRZ_ProcessRTimerCommand(command);
    }
    
    // Process rtimer command (runs on SERVER)
    void SRZ_ProcessRTimerCommand(string text)
    {
        if (!Replication.IsServer())
            return;
        
        int playerId = GetPlayerId();
        
        PlayerManager pm = GetGame().GetPlayerManager();
        if (!pm)
            return;
        
        if (playerId <= 0)
            return;
        
        bool isAdmin = pm.HasPlayerRole(playerId, EPlayerRole.ADMINISTRATOR);
        
        string trimmed = text;
        trimmed.Trim();
        string lower = trimmed;
        lower.ToLower();
        
        if (!lower.StartsWith(".rtimer"))
            return;
        
        if (!isAdmin)
        {
            SRZ_RPNet.SendToPlayer(playerId, "[SRZ] Admin access required.");
            return;
        }
        
        // Parse arguments
        string args = "";
        if (trimmed.Length() > 7)
        {
            args = trimmed.Substring(8, trimmed.Length() - 8);
            args.Trim();
        }
        
        if (args.IsEmpty())
        {
            SRZ_RPNet.SendToPlayer(playerId, "Usage: .rtimer <seconds> OR .rtimer stop");
            SRZ_RPNet.SendToPlayer(playerId, "Example: .rtimer 300 (5 minute countdown)");
            return;
        }
        
        // Check for stop command
        string argsLower = args;
        argsLower.ToLower();
        if (argsLower == "stop" || argsLower == "cancel")
        {
            SRZ_RestartTimerManager.GetInstance().StopTimer();
            SRZ_RPNet.SendToPlayer(playerId, "[SRZ] Restart timer cancelled.");
            return;
        }
        
        // Parse seconds
        int seconds = args.ToInt();
        if (seconds <= 0)
        {
            SRZ_RPNet.SendToPlayer(playerId, "Invalid time. Use a positive number of seconds.");
            return;
        }
        
        if (seconds > 3600)
        {
            SRZ_RPNet.SendToPlayer(playerId, "Maximum timer is 3600 seconds (1 hour).");
            return;
        }
        
        // Start the timer
        SRZ_RestartTimerManager.GetInstance().StartTimer(seconds);
        
        string adminName = pm.GetPlayerName(playerId);
        Print(string.Format("[SRZ_RTIMER] Admin %1 started restart timer: %2 seconds", adminName, seconds), LogLevel.WARNING);
        SRZ_RPNet.SendToPlayer(playerId, string.Format("[SRZ] Restart timer started: %1 seconds", seconds));
    }
    
    // Server calls this to update client timer display
    void SRZ_SendTimerUpdate(int secondsRemaining, bool stopped)
    {
        if (!Replication.IsServer())
            return;
        
        Rpc(SRZ_RpcAsk_UpdateTimer, secondsRemaining, stopped);
    }
    
    [RplRpc(RplChannel.Reliable, RplRcver.Owner)]
    protected void SRZ_RpcAsk_UpdateTimer(int secondsRemaining, bool stopped)
    {
        SRZ_RestartTimerUI ui = SRZ_RestartTimerUI.GetInstance();
        if (ui)
        {
            if (stopped)
                ui.StopTimer();
            else
                ui.UpdateTimer(secondsRemaining);
        }
    }
}

// Server-side timer manager
class SRZ_RestartTimerManager
{
    protected static ref SRZ_RestartTimerManager s_Instance;
    protected int m_SecondsRemaining = 0;
    protected bool m_IsRunning = false;
    
    static SRZ_RestartTimerManager GetInstance()
    {
        if (!s_Instance)
            s_Instance = new SRZ_RestartTimerManager();
        return s_Instance;
    }
    
    void StartTimer(int seconds)
    {
        m_SecondsRemaining = seconds;
        m_IsRunning = true;
        
        // Broadcast to all players immediately
        BroadcastTimerUpdate();
        
        // Start tick
        GetGame().GetCallqueue().CallLater(TimerTick, 1000, true);
    }
    
    void StopTimer()
    {
        m_IsRunning = false;
        m_SecondsRemaining = 0;
        
        // Stop the tick
        GetGame().GetCallqueue().Remove(TimerTick);
        
        // Broadcast stop to all players
        BroadcastTimerStop();
    }
    
    protected void TimerTick()
    {
        if (!m_IsRunning)
        {
            GetGame().GetCallqueue().Remove(TimerTick);
            return;
        }
        
        m_SecondsRemaining--;
        
        if (m_SecondsRemaining <= 0)
        {
            m_IsRunning = false;
            GetGame().GetCallqueue().Remove(TimerTick);
            BroadcastTimerUpdate(); // Final update showing 0
            
            // Kick all players
            GetGame().GetCallqueue().CallLater(KickAllPlayers, 1000, false);
            return;
        }
        
        // Broadcast update to all players
        BroadcastTimerUpdate();
    }
    
    protected void KickAllPlayers()
    {
        if (!Replication.IsServer())
            return;
        
        PlayerManager pm = GetGame().GetPlayerManager();
        if (!pm)
            return;
        
        array<int> playerIds = new array<int>();
        pm.GetPlayers(playerIds);
        
        Print(string.Format("[SRZ_RTIMER] Kicking %1 players for server restart", playerIds.Count()), LogLevel.WARNING);
        
        foreach (int pid : playerIds)
        {
            pm.KickPlayer(pid, PlayerManagerKickReason.KICK, 60);
        }
    }
    
    protected void BroadcastTimerUpdate()
    {
        if (!Replication.IsServer())
            return;
        
        PlayerManager pm = GetGame().GetPlayerManager();
        if (!pm)
            return;
        
        array<int> playerIds = new array<int>();
        pm.GetPlayers(playerIds);
        
        foreach (int pid : playerIds)
        {
            SCR_PlayerController pc = SCR_PlayerController.Cast(pm.GetPlayerController(pid));
            if (pc)
            {
                pc.SRZ_SendTimerUpdate(m_SecondsRemaining, false);
            }
        }
    }
    
    protected void BroadcastTimerStop()
    {
        if (!Replication.IsServer())
            return;
        
        PlayerManager pm = GetGame().GetPlayerManager();
        if (!pm)
            return;
        
        array<int> playerIds = new array<int>();
        pm.GetPlayers(playerIds);
        
        foreach (int pid : playerIds)
        {
            SCR_PlayerController pc = SCR_PlayerController.Cast(pm.GetPlayerController(pid));
            if (pc)
            {
                pc.SRZ_SendTimerUpdate(0, true);
            }
        }
    }
    
    int GetSecondsRemaining()
    {
        return m_SecondsRemaining;
    }
    
    bool IsRunning()
    {
        return m_IsRunning;
    }
}

// Client-side UI for displaying the timer using popup at intervals
class SRZ_RestartTimerUI
{
    protected static ref SRZ_RestartTimerUI s_Instance;
    protected int m_SecondsRemaining = 0;
    protected bool m_IsShowing = false;
    
    static SRZ_RestartTimerUI GetInstance()
    {
        if (!s_Instance)
            s_Instance = new SRZ_RestartTimerUI();
        return s_Instance;
    }
    
    void UpdateTimer(int secondsRemaining)
    {
        m_SecondsRemaining = secondsRemaining;
        m_IsShowing = true;
        
        // Show popup at specific intervals
        if (ShouldShowPopup(secondsRemaining))
        {
            ShowTimerPopup();
        }
    }
    
    void StopTimer()
    {
        m_SecondsRemaining = 0;
        m_IsShowing = false;
        
        // Show cancelled message
        SCR_PopUpNotification popup = SCR_PopUpNotification.GetInstance();
        if (popup)
        {
            popup.PopupMsg("Server restart cancelled", 5.0);
        }
    }
    
    protected bool ShouldShowPopup(int seconds)
    {
        // Always show at these key times
        if (seconds == 600) return true;  // 10 min
        if (seconds == 300) return true;  // 5 min
        if (seconds == 180) return true;  // 3 min
        if (seconds == 120) return true;  // 2 min
        if (seconds == 60) return true;   // 1 min
        if (seconds == 30) return true;   // 30 sec
        if (seconds == 15) return true;   // 15 sec
        if (seconds <= 10) return true;   // Last 10 seconds countdown
        
        // Also show every 60 seconds for longer timers
        if (seconds > 60 && seconds % 60 == 0) return true;
        
        return false;
    }
    
    protected void ShowTimerPopup()
    {
        string timeStr = FormatTime(m_SecondsRemaining);
        string title;
        string message;
        float duration = 5.0;
        
        if (m_SecondsRemaining <= 10)
        {
            title = "RESTART IMMINENT";
            message = string.Format("%1", timeStr);
            duration = 1.5;
        }
        else if (m_SecondsRemaining <= 60)
        {
            title = "SERVER RESTART";
            message = string.Format("%1 remaining", timeStr);
            duration = 3.0;
        }
        else
        {
            title = "SERVER RESTART";
            message = string.Format("%1 remaining", timeStr);
            duration = 5.0;
        }
        
        SCR_PopUpNotification popup = SCR_PopUpNotification.GetInstance();
        if (popup)
        {
            popup.PopupMsg(string.Format("%1\n%2", title, message), duration);
        }
    }
    
    protected string FormatTime(int totalSeconds)
    {
        int minutes = totalSeconds / 60;
        int seconds = totalSeconds % 60;
        
        string secStr = seconds.ToString();
        if (seconds < 10)
            secStr = "0" + secStr;
        
        if (minutes > 0)
            return string.Format("%1:%2", minutes, secStr);
        else
            return string.Format("%1 seconds", seconds);
    }
}