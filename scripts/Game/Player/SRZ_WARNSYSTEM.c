// Warning system for admins to warn players about rule violations
modded class SCR_PlayerController
{
    //------------------------------------------------------------------------------------------------
    void SRZ_ShowMessageToPlayers(array<int> playerIds, string msg)
    {
        Rpc(SRZ_Server_ShowMessageToPlayers, playerIds, msg);
    }

    //------------------------------------------------------------------------------------------------
    [RplRpc(RplChannel.Reliable, RplRcver.Server)]
    void SRZ_Server_ShowMessageToPlayers(array<int> playerIds, string msg)
    {
        int instigatorPlayerId = GetPlayerId();

        PlayerManager pm = GetGame().GetPlayerManager();
        if (!pm)
            return;

        if (instigatorPlayerId <= 0)
            return;

        bool isGm = pm.HasPlayerRole(instigatorPlayerId, EPlayerRole.GAME_MASTER);
        if (!isGm)
            return;

        foreach (int playerId : playerIds)
            SRZ_RPNet.SendToPlayer(playerId, msg);
    }

    //------------------------------------------------------------------------------------------------
    void SRZ_SendWarnCommand(string command)
    {
        if (Replication.IsServer())
            SRZ_ProcessWarnCommand(command);
        else
            Rpc(SRZ_RpcAsk_ProcessWarnCommand, command);
    }

    //------------------------------------------------------------------------------------------------
    [RplRpc(RplChannel.Reliable, RplRcver.Server)]
    protected void SRZ_RpcAsk_ProcessWarnCommand(string command)
    {
        SRZ_ProcessWarnCommand(command);
    }

    //------------------------------------------------------------------------------------------------
    void SRZ_SendWarning(string adminName, string reason)
    {
        if (!Replication.IsServer())
            return;

        Print(string.Format("[SRZ_WARN] Sending warning: %1", reason), LogLevel.NORMAL);
        Rpc(SRZ_RpcAsk_ShowWarning, adminName, reason);
    }

    //------------------------------------------------------------------------------------------------
    [RplRpc(RplChannel.Reliable, RplRcver.Owner)]
    protected void SRZ_RpcAsk_ShowWarning(string adminName, string reason)
    {
        Print(string.Format("[SRZ_WARN] Received warning from %1: %2", adminName, reason), LogLevel.NORMAL);

        SCR_PopUpNotification popup = SCR_PopUpNotification.GetInstance();
        if (popup)
        {
            popup.PopupMsg(reason, 15.0);
            return;
        }

        SCR_HintManagerComponent hintMgr = SCR_HintManagerComponent.GetInstance();
        if (hintMgr)
            hintMgr.ShowCustomHint(string.Format("Reason: %1", reason), "ADMIN WARNING", 15);
    }

    //------------------------------------------------------------------------------------------------
    void SRZ_ProcessWarnCommand(string text)
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

        if (!lower.StartsWith(".warn "))
            return;

        if (!isAdmin)
        {
            SRZ_RPNet.SendToPlayer(playerId, "Admin access required.");
            return;
        }

        string args = trimmed.Substring(6, trimmed.Length() - 6);
        args.Trim();

        if (args.IsEmpty())
        {
            SRZ_RPNet.SendToPlayer(playerId, "Usage: .warn <player> <reason>");
            SRZ_RPNet.SendToPlayer(playerId, "Example: .warn John Breaking RP rules");
            return;
        }

        string targetName;
        string reason;

        if (args.StartsWith("\""))
        {
            if (!SRZ_ParseQuotedWarnArgs(args, targetName, reason))
            {
                SRZ_RPNet.SendToPlayer(playerId, "Invalid format. Use: .warn \"player\" \"reason\"");
                return;
            }
        }
        else
        {
            int spaceIdx = args.IndexOf(" ");
            if (spaceIdx < 0)
            {
                SRZ_RPNet.SendToPlayer(playerId, "Usage: .warn <player> <reason>");
                return;
            }

            targetName = args.Substring(0, spaceIdx);
            reason = args.Substring(spaceIdx + 1, args.Length() - spaceIdx - 1);
        }

        targetName.Trim();
        reason.Trim();

        if (targetName.IsEmpty())
        {
            SRZ_RPNet.SendToPlayer(playerId, "Player name cannot be empty.");
            return;
        }

        if (reason.IsEmpty())
        {
            SRZ_RPNet.SendToPlayer(playerId, "Warning reason cannot be empty.");
            return;
        }

        int targetId = SRZ_FindPlayerForWarn(pm, targetName);
        if (targetId <= 0)
        {
            SRZ_RPNet.SendToPlayer(playerId, string.Format("Player not found: %1", targetName));
            return;
        }

        string targetActualName = pm.GetPlayerName(targetId);
        string adminName = pm.GetPlayerName(playerId);

        SCR_PlayerController targetPc = SCR_PlayerController.Cast(pm.GetPlayerController(targetId));
        if (targetPc)
            targetPc.SRZ_SendWarning(adminName, reason);

        Print(string.Format("[SRZ_WARN] Admin %1 warned %2: %3", adminName, targetActualName, reason), LogLevel.WARNING);
        SRZ_RPNet.SendToPlayer(playerId, string.Format("Warning sent to %1: %2", targetActualName, reason));
    }

    //------------------------------------------------------------------------------------------------
    protected bool SRZ_ParseQuotedWarnArgs(string args, out string name, out string reason)
    {
        name = "";
        reason = "";

        int firstQuote = args.IndexOf("\"");
        if (firstQuote < 0)
            return false;

        int secondQuote = args.IndexOfFrom(firstQuote + 1, "\"");
        if (secondQuote < 0)
            return false;

        name = args.Substring(firstQuote + 1, secondQuote - firstQuote - 1);

        int thirdQuote = args.IndexOfFrom(secondQuote + 1, "\"");
        if (thirdQuote < 0)
        {
            reason = args.Substring(secondQuote + 1, args.Length() - secondQuote - 1);
            reason.Trim();
            return !reason.IsEmpty();
        }

        int fourthQuote = args.IndexOfFrom(thirdQuote + 1, "\"");
        if (fourthQuote < 0)
            return false;

        reason = args.Substring(thirdQuote + 1, fourthQuote - thirdQuote - 1);
        return true;
    }

    //------------------------------------------------------------------------------------------------
    protected int SRZ_FindPlayerForWarn(PlayerManager pm, string searchName)
    {
        if (!pm)
            return -1;

        searchName.ToLower();
        array<int> playerIds = new array<int>();
        pm.GetPlayers(playerIds);

        foreach (int pid : playerIds)
        {
            string playerName = pm.GetPlayerName(pid);
            string lowerName = playerName;
            lowerName.ToLower();

            if (lowerName == searchName || lowerName.Contains(searchName))
                return pid;
        }

        return -1;
    }
}