class SRZ_PDALoggerComponentClass : SCR_BaseGameModeComponentClass {}

class SRZ_PDALoggerComponent : SCR_BaseGameModeComponent
{
    protected const int FLUSH_INTERVAL_MS = 3000;
    protected const int MAX_BATCH_SIZE = 10;
    protected ref array<string> m_aPendingMessages;

    //------------------------------------------------------------------------------------------------
    override void OnPostInit(IEntity owner)
    {
        super.OnPostInit(owner);

        if (!Replication.IsServer())
            return;

        SRZ_KillfeedConfigManager.EnsureDefaults();

        if (!m_aPendingMessages)
            m_aPendingMessages = new array<string>();

        GetGame().GetCallqueue().CallLater(FlushQueue, FLUSH_INTERVAL_MS, true);
        Print("[SRZ_PDALogger] PDA Logger started.", LogLevel.NORMAL);
    }

    //------------------------------------------------------------------------------------------------
    static void LogPDAMessage(string sender, string message, string channel = "")
    {
        BaseGameMode gameMode = BaseGameMode.Cast(GetGame().GetGameMode());
        if (!gameMode)
            return;

        SRZ_PDALoggerComponent logger = SRZ_PDALoggerComponent.Cast(gameMode.FindComponent(SRZ_PDALoggerComponent));
        if (!logger)
            return;

        logger.QueueMessage(sender, message, channel);
    }

    //------------------------------------------------------------------------------------------------
    void QueueMessage(string sender, string message, string channel = "")
    {
        if (!Replication.IsServer())
            return;

        if (!m_aPendingMessages)
            m_aPendingMessages = new array<string>();

        sender = sender + ResolveSenderDetails(sender);

        message.Replace("\"", "");
        sender.Replace("\"", "");

        string entry;
        if (!channel.IsEmpty())
            entry = "[" + channel + "] " + sender + ": " + message;
        else
            entry = sender + ": " + message;

        m_aPendingMessages.Insert(entry);
        Print("[SRZ_PDALogger] Queued: " + entry, LogLevel.NORMAL);
    }

    //------------------------------------------------------------------------------------------------
    protected string ResolveSenderDetails(string sender)
    {
        PlayerManager pm = GetGame().GetPlayerManager();
        if (!pm)
            return "";

        SRZ_RPNameProfileManager profileMgr = SRZ_RPNameProfileManager.GetInstance();

        array<int> playerIds = {};
        pm.GetPlayers(playerIds);

        int bestId = 0;
        int bestLength = 0;
        foreach (int pid : playerIds)
        {
            string rpName = profileMgr.GetNameForPlayer(pid);
            if (rpName.IsEmpty() || rpName.Length() <= bestLength)
                continue;

            if (!sender.Contains(rpName))
                continue;

            bestId = pid;
            bestLength = rpName.Length();
        }

        if (bestId <= 0)
            return "";

        string factionName = "Unknown";
        IEntity entity = pm.GetPlayerControlledEntity(bestId);
        if (entity)
        {
            ARMST_PLAYER_STATS_COMPONENT stats = ARMST_PLAYER_STATS_COMPONENT.Cast(entity.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
            if (stats)
            {
                factionName = typename.EnumToString(ARMST_FACTION_LABEL, stats.GetFactionKey());
                factionName.Replace("FACTION_", "");
            }
        }

        return string.Format(" (Gamertag: %1 | Faction: %2)", pm.GetPlayerName(bestId), factionName);
    }

    //------------------------------------------------------------------------------------------------
    protected void FlushQueue()
    {
        if (!Replication.IsServer())
            return;

        if (!m_aPendingMessages || m_aPendingMessages.IsEmpty())
            return;

        string webhookUrl = SRZ_KillfeedConfigManager.GetStringValue("m_sPDALogWebhookURL", "");
        if (webhookUrl.IsEmpty())
        {
            m_aPendingMessages.Clear();
            return;
        }

        int total = m_aPendingMessages.Count();
        int batchCount = total;
        if (batchCount > MAX_BATCH_SIZE)
            batchCount = MAX_BATCH_SIZE;

        string combinedContent;
        for (int i = 0; i < batchCount; i++)
        {
            if (i > 0)
                combinedContent += "\n";
            combinedContent += m_aPendingMessages[i];
        }

        for (int j = 0; j < batchCount; j++)
            m_aPendingMessages.RemoveOrdered(0);

        combinedContent.Replace("\\", "");
        combinedContent.Replace("\"", "");
        SendDiscordWebhook(webhookUrl, combinedContent);
    }

    //------------------------------------------------------------------------------------------------
    protected void SendDiscordWebhook(string webhookUrl, string content)
    {
        RestApi api = GetGame().GetRestApi();
        if (!api)
        {
            Print("[SRZ_PDALogger] RestApi not available.", LogLevel.ERROR);
            return;
        }

        RestContext ctx = api.GetContext("https://discord.com");
        if (!ctx)
        {
            Print("[SRZ_PDALogger] Could not create RestContext for Discord.", LogLevel.ERROR);
            return;
        }

        string pathAndToken = "";
        int apiPathIndex = webhookUrl.IndexOf("/api/webhooks/");
        if (apiPathIndex != -1)
        {
            int length = webhookUrl.Length() - apiPathIndex;
            pathAndToken = webhookUrl.Substring(apiPathIndex, length);
        }

        if (pathAndToken.IsEmpty() || pathAndToken.Length() < 16)
        {
            Print("[SRZ_PDALogger] Invalid webhook URL format.", LogLevel.ERROR);
            return;
        }

        ctx.SetHeaders("Content-Type,application/json");
        string body = "{ \"content\": \"" + content + "\" }";
        ctx.POST_now(pathAndToken, body);
    }
}