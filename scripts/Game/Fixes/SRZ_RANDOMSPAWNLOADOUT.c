modded class ARMST_BasicSpawnLogic : EPF_BasicSpawnLogic
{
    protected ref map<int, ARMST_FACTION_LABEL> m_mPlayerFactions = new map<int, ARMST_FACTION_LABEL>();

    //------------------------------------------------------------------------------------------------
    override void OnPlayerKilled_S(int playerId, IEntity playerEntity, IEntity killerEntity, notnull Instigator killer)
    {
        ARMST_PLAYER_STATS_COMPONENT statsComponent = ARMST_PLAYER_STATS_COMPONENT.Cast(playerEntity.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
        if (statsComponent)
        {
            m_mPlayerFactions.Set(playerId, statsComponent.GetFactionKey());
            Print(string.Format("[SRZ_SpawnLogicFix] Player %1 died, saved faction: %2", playerId, statsComponent.GetFactionKey()), LogLevel.NORMAL);
        }

        super.OnPlayerKilled_S(playerId, playerEntity, killerEntity, killer);
    }

    //------------------------------------------------------------------------------------------------
    override protected void CreateCharacter(int playerId, string characterPersistenceId)
    {
        if (m_mPlayerFactions.Contains(playerId))
        {
            playerFaction = m_mPlayerFactions.Get(playerId);
            m_mPlayerFactions.Remove(playerId);
            Print(string.Format("[SRZ_SpawnLogicFix] Player %1 respawning with saved faction: %2", playerId, playerFaction), LogLevel.NORMAL);
        }
        else
        {
            playerFaction = ARMST_FACTION_LABEL.FACTION_STALKER;
            Print(string.Format("[SRZ_SpawnLogicFix] Player %1 is new, defaulting to FACTION_STALKER", playerId), LogLevel.NORMAL);
        }

        super.CreateCharacter(playerId, characterPersistenceId);
    }

    //------------------------------------------------------------------------------------------------
    override void LoadStartingLoot(IEntity character, int playerId, ARMST_FACTION_LABEL factionKey)
    {
        array<ref ResourceName> lootConfigs = GetLootConfigForFaction(factionKey, playerId);
        if (!lootConfigs || lootConfigs.IsEmpty())
        {
            Print(string.Format("[SRZ_LOOT] No loot configs found for faction %1, player %2.", factionKey, playerId), LogLevel.WARNING);
            return;
        }

        int randomIndex = Math.RandomInt(0, lootConfigs.Count());
        ResourceName selected = lootConfigs[randomIndex];

        if (selected.IsEmpty())
        {
            Print(string.Format("[SRZ_LOOT] Selected loot config is empty for faction %1, player %2.", factionKey, playerId), LogLevel.WARNING);
            return;
        }

        Print(string.Format("[SRZ_LOOT] Loadout variant %1 of %2 selected for faction %3, player %4.",
            randomIndex + 1, lootConfigs.Count(), factionKey, playerId), LogLevel.NORMAL);

        Helpers.GiveLoot(character, selected);
    }
}