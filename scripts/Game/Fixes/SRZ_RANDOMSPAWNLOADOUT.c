modded class ARMST_BasicSpawnLogic : EPF_BasicSpawnLogic
{
    protected ref map<int, ARMST_FACTION_LABEL> m_mPlayerFactions = new map<int, ARMST_FACTION_LABEL>();
	
	   protected ref map<int, SR_STALKER_RANK> m_mPlayerRanks = new map<int, SR_STALKER_RANK>();

    //------------------------------------------------------------------------------------------------
    override void OnPlayerKilled_S(int playerId, IEntity playerEntity, IEntity killerEntity, notnull Instigator killer)
    {
        ARMST_PLAYER_STATS_COMPONENT statsComponent = ARMST_PLAYER_STATS_COMPONENT.Cast(playerEntity.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
        if (statsComponent)
        {
            m_mPlayerFactions.Set(playerId, statsComponent.GetFactionKey());
			m_mPlayerRanks.Set(playerId, statsComponent.SR_GetRank());
            Print(string.Format("[SR Respawn] Player %1 died, saved faction: %2 and rank %3", playerId, statsComponent.GetFactionKey(), statsComponent.SR_GetRank() ), LogLevel.NORMAL);
        }

        super.OnPlayerKilled_S(playerId, playerEntity, killerEntity, killer);
    }

    //------------------------------------------------------------------------------------------------
    override protected void CreateCharacter(int playerId, string characterPersistenceId)
    {
        if (m_mPlayerFactions.Contains(playerId))
        {
            playerFaction = m_mPlayerFactions.Get(playerId);
            Print(string.Format("[SR Respawn] Player %1 respawning with saved faction: %2", playerId, playerFaction), LogLevel.NORMAL);
        }
        else
        {
            playerFaction = ARMST_FACTION_LABEL.FACTION_STALKER;
            Print(string.Format("[SR Respawn] Player %1 is new, defaulting to FACTION_STALKER", playerId), LogLevel.NORMAL);
        }
		
        super.CreateCharacter(playerId, characterPersistenceId);
	
		// ADDED FUNCTION TO SAVE STALKER RANK AFTER DEATH
        PlayerManager playerManager = GetGame().GetPlayerManager();
        if (!playerManager)
            return;
            
        // Get the brand new physical body entity that was just made by 'super'
        IEntity newCharacter = playerManager.GetPlayerControlledEntity(playerId);
        if (!newCharacter)
            return;

        // Find the stats component attached to this brand new body
        ARMST_PLAYER_STATS_COMPONENT newStatsComp = ARMST_PLAYER_STATS_COMPONENT.Cast(newCharacter.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
        if (newStatsComp)
        {
            if (m_mPlayerRanks.Contains(playerId))
            {
                SR_STALKER_RANK savedRank = m_mPlayerRanks.Get(playerId);
                newStatsComp.SR_SetRank(savedRank); // Inject the rank into the component
                m_mPlayerRanks.Remove(playerId);   // Clear the map slot to save memory
                Print(string.Format("[SR Respawn] Player %1 respawning with saved rank: %2", playerId, savedRank), LogLevel.NORMAL);
            }
            else
            {
                newStatsComp.SR_SetRank(SR_STALKER_RANK.ROOKIE); // New player fallback
                Print(string.Format("[SR Respawn] Player %1 is new, defaulting rank to ROOKIE", playerId), LogLevel.NORMAL);
            }
        }

    }

    //------------------------------------------------------------------------------------------------
    override void LoadStartingLoot(IEntity character, int playerId, ARMST_FACTION_LABEL factionKey)
    {
		if (playerFaction != factionKey)
		{
			factionKey = playerFaction;
		}
		
		if (factionKey != ARMST_FACTION_LABEL.FACTION_STALKER) {
		
        array<ref ResourceName> lootConfigs = GetLootConfigForFaction(factionKey, playerId);
        if (!lootConfigs || lootConfigs.IsEmpty())
        {
            Print(string.Format("[SR Respawn] No loot configs found for faction %1, player %2.", factionKey, playerId), LogLevel.WARNING);
            return;
        }

        int randomIndex = Math.RandomInt(0, lootConfigs.Count());
        ResourceName selected = lootConfigs[randomIndex];

        if (selected.IsEmpty())
        {
            Print(string.Format("[SR Respawn] Selected loot config is empty for faction %1, player %2.", factionKey, playerId), LogLevel.WARNING);
            return;
        }

        Print(string.Format("[SR Respawn] Loadout variant %1 of %2 selected for faction %3, player %4.",
            randomIndex + 1, lootConfigs.Count(), factionKey, playerId), LogLevel.NORMAL);

        Helpers.GiveLoot(character, selected);
		}
		else 
		{
            Print(string.Format("[SR Respawn] Player %1 is a Stalker! Fetching rank-based loadout...", playerId), LogLevel.NORMAL);

            // Get the player's current rank from our saved map
            SR_STALKER_RANK playerRank = SR_STALKER_RANK.ROOKIE;
            if (m_mPlayerRanks.Contains(playerId))
            {
                playerRank = m_mPlayerRanks.Get(playerId);
            }

            // PASTE YOUR RANKS CONFIG FILE HERE, COPY THE CONFIG FILES RESOURCE GUID FROM THE RESOURCE BROWSER AND PUT IT IN THE CURLY BRACKETS (or just use the one i made)
            ResourceName rankConfigPath = "{032FD7902BA3FCA1}Configs/Stalker_Ranks/SR_RankLoadoutsConfig.conf";

            Resource resConf = Resource.Load(rankConfigPath);
            if (resConf.IsValid())
            {
                // FIX: Called on 'res' instance directly
                SR_MasterRankLootConfig masterConfig = SR_MasterRankLootConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(resConf.GetResource().ToBaseContainer()));
                if (masterConfig && masterConfig.m_aRankList)
                {
                    foreach (SR_RankLootEntry rankEntry : masterConfig.m_aRankList)
                    {
                        if (rankEntry.m_eRank == playerRank)
                        {
                            if (rankEntry.m_aLootConfigs && !rankEntry.m_aLootConfigs.IsEmpty())
                            {
                                int randIdx = Math.RandomInt(0, rankEntry.m_aLootConfigs.Count());
                                ResourceName selectedRankLoot = rankEntry.m_aLootConfigs[randIdx];

                                if (!selectedRankLoot.IsEmpty())
                                {
                                    Print(string.Format("[SR Respawn] Stalker Rank %1 loot selected for Player %2.", playerRank, playerId), LogLevel.NORMAL);
                                    Helpers.GiveLoot(character, selectedRankLoot);
                                    return; 
                                }
                            }
                            break;
                        }
                    }
				
                }
            }

        }
		
		
	}
		
    }
}