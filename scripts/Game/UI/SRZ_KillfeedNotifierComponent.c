class SRZ_KillfeedNotifierComponentClass : SCR_BaseGameModeComponentClass {}

class SRZ_KillfeedNotifierComponent : SCR_BaseGameModeComponent
{
    protected const int FLUSH_INTERVAL_MS = 2000;
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

        GetGame().GetCallqueue().CallLater(FlushKillfeedQueue, FLUSH_INTERVAL_MS, true);
        Print("[SRZ_Killfeed] Killfeed notifier started.", LogLevel.NORMAL);
    }

    //------------------------------------------------------------------------------------------------
    protected string GetDamageTypeName(EDamageType dmgType)
    {
        if (dmgType == EDamageType.KINETIC)                 return "Gunshot";
        if (dmgType == EDamageType.EXPLOSIVE)               return "Explosion";
        if (dmgType == EDamageType.MELEE)                   return "Melee";
        if (dmgType == EDamageType.INCENDIARY)              return "Fire";
        if (dmgType == EDamageType.FIRE)                    return "Fire";
        if (dmgType == EDamageType.COLLISION)               return "Collision";
        if (dmgType == EDamageType.BLEEDING)                return "Bleeding";
        if (dmgType == EDamageType.TRUE)                    return "Environmental";
        if (dmgType == EDamageType.FRAGMENTATION)           return "Fragmentation";
        if (dmgType == EDamageType.PROCESSED_FRAGMENTATION) return "Surge/Anomaly";
        if (dmgType == EDamageType.REGENERATION)            return "Regeneration";
        if (dmgType == EDamageType.HEALING)                 return "Healing";
        return "Unknown";
    }

    //------------------------------------------------------------------------------------------------
    protected string GetKillerName(IEntity killerEntity)
    {
        if (!killerEntity)
            return "Unknown";

        string entName = killerEntity.GetName();
        if (!entName.IsEmpty())
            return entName;

        EntityPrefabData prefabData = killerEntity.GetPrefabData();
        if (prefabData)
        {
            string prefabName = FilePath.StripExtension(FilePath.StripPath(prefabData.GetPrefabName()));
            
            prefabName.Replace("armst_monster_", "");
            prefabName.Replace("armst_mutant_", "");
            prefabName.Replace("armst_anomaly_", "");
            prefabName.Replace("armst_", "");
            prefabName.Replace("_", " ");
            
            return prefabName;
        }

        return killerEntity.ClassName();
    }

    //------------------------------------------------------------------------------------------------
    protected string GetARMSTCauseOfDeath(IEntity victimEntity)
    {
        if (!victimEntity)
            return "";

        ARMST_PLAYER_STATS_COMPONENT stats = ARMST_PLAYER_STATS_COMPONENT.Cast(victimEntity.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
        if (!stats)
            return "";

        if (stats.ArmstPlayerStatGetToxic() >= 99)  return "Toxic Poisoning";
        if (stats.ArmstPlayerStatGetRadio() >= 99)  return "Radiation Poisoning";
        if (stats.ArmstPlayerStatGetPsy() <= 1)     return "Psy Breakdown";
        if (stats.ArmstPlayerStatGetWater() <= 1)   return "Dehydration";
        if (stats.ArmstPlayerStatGetEat() <= 1)     return "Starvation";

        return "";
    }

    //------------------------------------------------------------------------------------------------
    protected string BuildDamageLog(array<ref SRZ_DamageEntry> log)
    {
        if (!log || log.IsEmpty())
            return "";

        string result = "\n```";

        for (int i = 0; i < log.Count(); i++)
        {
            SRZ_DamageEntry entry = log[i];
            if (!entry)
                continue;

            string headshot = "";
            if (entry.isHeadshot)
                headshot = " HEADSHOT";

            string weapon = "";
            if (!entry.weaponName.IsEmpty())
                weapon = " (" + entry.weaponName + ")";

            result += "\n  " + entry.killerName + weapon + " -> " + entry.damagePart + " (" + entry.damageAmount + " dmg) [" + GetDamageTypeName(entry.damageType) + "]" + headshot;
        }

        result += "\n```";
        return result;
    }

    //------------------------------------------------------------------------------------------------
    override void OnPlayerKilled(notnull SCR_InstigatorContextData instigatorContextData)
    {
        super.OnPlayerKilled(instigatorContextData);

        if (!Replication.IsServer())
            return;

        if (!SRZ_KillfeedConfigManager.GetBoolValue("m_bEnableKillfeedWebhook", true))
            return;

        PlayerManager pm = GetGame().GetPlayerManager();
        if (!pm)
            return;

        int victimId = instigatorContextData.GetVictimPlayerID();
        string victimName = "These"; 
        IEntity victimEntity = null;
        if (victimId > 0)
        {
            string lookupName = pm.GetPlayerName(victimId);
            if (!lookupName.IsEmpty())
                victimName = lookupName;
                
            victimEntity = pm.GetPlayerControlledEntity(victimId);
        }

        int killerId        = instigatorContextData.GetKillerPlayerID();
        bool killerIsPlayer = instigatorContextData.HasAnyKillerCharacterControlType(SCR_ECharacterControlType.PLAYER);
        bool killerIsAI     = instigatorContextData.HasAnyKillerCharacterControlType(SCR_ECharacterControlType.AI);
        IEntity killerEntity = instigatorContextData.GetKillerEntity();
        
        Instigator killerInstigator = instigatorContextData.GetInstigator();
        
        // Bypassed EGMPInstigatorType to prevent compiler undeclared identifier error
        bool isGMKill = false; 

        array<ref SRZ_DamageEntry> damageLog = SRZ_KillfeedDamageTracker.GetAndClearLog(victimId);

        string cause = "Unknown";
        string extractedWeapon = "";
        bool wasHeadshotKill = false;

        if (damageLog && !damageLog.IsEmpty())
        {
            SRZ_DamageEntry lastEntry = damageLog[damageLog.Count() - 1];
            if (lastEntry)
            {
                cause = GetDamageTypeName(lastEntry.damageType);
                extractedWeapon = lastEntry.weaponName;
                wasHeadshotKill = lastEntry.isHeadshot;
            }
        }

        string armstCause = GetARMSTCauseOfDeath(victimEntity);
        if (!armstCause.IsEmpty())
            cause = armstCause;

        string distanceStr = "";
        if (victimEntity && killerEntity && !isGMKill)
        {
            float distance = vector.Distance(victimEntity.GetOrigin(), killerEntity.GetOrigin());
            distanceStr = " | " + Math.Round(distance) + "m";
        }

        string headshotTag = "";
        if (wasHeadshotKill)
            headshotTag = " 🎯 [HEADSHOT]";

        string damageLogStr = BuildDamageLog(damageLog);
        string content;

        if (isGMKill)
        {
            content = "⚡ **" + victimName + "** was terminated by the **Game Master**";
        }
        else if (killerIsPlayer && killerId > 0)
        {
            string killerName = pm.GetPlayerName(killerId);
            if (killerName.IsEmpty())
                killerName = "Unknown Player";
                
            string weaponContext = "";
            if (!extractedWeapon.IsEmpty())
                weaponContext = " with **" + extractedWeapon + "**";

            content = "⚔️ **" + killerName + "** killed **" + victimName + "**" + weaponContext + " [" + cause + distanceStr + "]" + headshotTag + damageLogStr;
        }
        else if (killerIsAI || killerEntity)
        {
            string killerName = GetKillerName(killerEntity);
            string weaponContext = "";
            if (!extractedWeapon.IsEmpty())
                weaponContext = " with **" + extractedWeapon + "**";

            content = "🤖 **" + victimName + "** was killed by **" + killerName + "**" + weaponContext + " [" + cause + distanceStr + "]" + headshotTag + damageLogStr;
        }
        else
        {
            content = "💀 **" + victimName + "** died [" + cause + "]" + damageLogStr;
        }

        content.Replace("\"", "");

        if (!m_aPendingMessages)
            m_aPendingMessages = new array<string>();

        m_aPendingMessages.Insert(content);
    }

    //------------------------------------------------------------------------------------------------
    protected void FlushKillfeedQueue()
    {
        if (!Replication.IsServer())
            return;

        if (!m_aPendingMessages || m_aPendingMessages.IsEmpty())
            return;

        string webhookUrl = SRZ_KillfeedConfigManager.GetStringValue("m_sKillfeedWebhookURL", "");
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

        combinedContent.Replace("\"", "");
        SendDiscordWebhook(webhookUrl, combinedContent);
    }

    //------------------------------------------------------------------------------------------------
    protected void SendDiscordWebhook(string webhookUrl, string content)
    {
        RestApi api = GetGame().GetRestApi();
        if (!api)
        {
            Print("[SRZ_Killfeed] RestApi not available.", LogLevel.ERROR);
            return;
        }

        RestContext ctx = api.GetContext("https://discord.com");
        if (!ctx)
        {
            Print("[SRZ_Killfeed] Could not create RestContext for Discord.", LogLevel.ERROR);
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
            Print("[SRZ_Killfeed] Invalid webhook URL format.", LogLevel.ERROR);
            return;
        }

        ctx.SetHeaders("Content-Type,application/json");
        string body = "{ \"content\": \"" + content + "\" }";
        ctx.POST_now(pathAndToken, body);
    }
}