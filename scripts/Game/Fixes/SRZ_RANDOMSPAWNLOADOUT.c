modded class ARMST_BasicSpawnLogic : EPF_BasicSpawnLogic
{
	protected ref map<int, ARMST_FACTION_LABEL> m_mPlayerFactions = new map<int, ARMST_FACTION_LABEL>();
	protected ref map<int, SR_STALKER_RANK> m_mPlayerRanks = new map<int, SR_STALKER_RANK>();

	protected const ResourceName SR_RANK_LOOT_CONFIG = "{032FD7902BA3FCA1}Configs/Stalker_Ranks/SR_RankLoadoutsConfig.conf";

	//------------------------------------------------------------------------------------------------
	override void OnPlayerKilled_S(int playerId, IEntity playerEntity, IEntity killerEntity, notnull Instigator killer)
	{
		ARMST_PLAYER_STATS_COMPONENT statsComponent = ARMST_PLAYER_STATS_COMPONENT.Cast(playerEntity.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
		if (statsComponent)
		{
			m_mPlayerFactions.Set(playerId, statsComponent.GetFactionKey());
			m_mPlayerRanks.Set(playerId, statsComponent.SR_GetRank());
			SRZ_RankStore.GetInstance().SetRank(playerId, statsComponent.SR_GetRank());
			Print(string.Format("[SR Respawn] Player %1 died, saved faction: %2 and rank %3", playerId, statsComponent.GetFactionKey(), statsComponent.SR_GetRank()), LogLevel.NORMAL);
		}

		super.OnPlayerKilled_S(playerId, playerEntity, killerEntity, killer);
	}

	//------------------------------------------------------------------------------------------------
	protected SR_STALKER_RANK SR_ResolveRank(int playerId, IEntity character)
	{
		SR_STALKER_RANK rank;
		if (SRZ_RankStore.GetInstance().GetRank(playerId, rank))
			return rank;

		if (m_mPlayerRanks.Find(playerId, rank))
			return rank;

		if (character)
		{
			ARMST_PLAYER_STATS_COMPONENT stats = ARMST_PLAYER_STATS_COMPONENT.Cast(character.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
			if (stats)
				return stats.SR_GetRank();
		}

		return SR_STALKER_RANK.ROOKIE;
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

		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!playerManager)
			return;

		IEntity newCharacter = playerManager.GetPlayerControlledEntity(playerId);
		if (!newCharacter)
			return;

		ARMST_PLAYER_STATS_COMPONENT newStatsComp = ARMST_PLAYER_STATS_COMPONENT.Cast(newCharacter.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
		if (!newStatsComp)
			return;

		SR_STALKER_RANK savedRank = SR_ResolveRank(playerId, null);
		newStatsComp.SR_SetRank(savedRank);
		Print(string.Format("[SR Respawn] Player %1 respawning with rank: %2", playerId, typename.EnumToString(SR_STALKER_RANK, savedRank)), LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	override void LoadStartingLoot(IEntity character, int playerId, ARMST_FACTION_LABEL factionKey)
	{
		if (playerFaction != factionKey)
			factionKey = playerFaction;

		if (factionKey != ARMST_FACTION_LABEL.FACTION_STALKER)
		{
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

			Print(string.Format("[SR Respawn] Loadout variant %1 of %2 selected for faction %3, player %4.", randomIndex + 1, lootConfigs.Count(), factionKey, playerId), LogLevel.NORMAL);
			Helpers.GiveLoot(character, selected);
			return;
		}

		SR_STALKER_RANK playerRank = SR_ResolveRank(playerId, character);

		Resource resConf = Resource.Load(SR_RANK_LOOT_CONFIG);
		if (!resConf.IsValid())
		{
			Print("[SR Respawn] Rank loot config failed to load", LogLevel.ERROR);
			return;
		}

		SR_MasterRankLootConfig masterConfig = SR_MasterRankLootConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(resConf.GetResource().ToBaseContainer()));
		if (!masterConfig || !masterConfig.m_aRankList)
			return;

		// Use the player's rank, or the closest rank below it that has loadouts
		for (int rank = playerRank; rank >= 0; rank--)
		{
			foreach (SR_RankLootEntry rankEntry : masterConfig.m_aRankList)
			{
				if (rankEntry.m_eRank != rank || !rankEntry.m_aLootConfigs || rankEntry.m_aLootConfigs.IsEmpty())
					continue;

				ResourceName selectedRankLoot = rankEntry.m_aLootConfigs[Math.RandomInt(0, rankEntry.m_aLootConfigs.Count())];
				if (selectedRankLoot.IsEmpty())
					continue;

				Print(string.Format("[SR Respawn] Player %1 rank %2, gave %3 loadout %4", playerId, typename.EnumToString(SR_STALKER_RANK, playerRank), typename.EnumToString(SR_STALKER_RANK, rank), selectedRankLoot), LogLevel.NORMAL);
				Helpers.GiveLoot(character, selectedRankLoot);
				return;
			}
		}

		Print(string.Format("[SR Respawn] No rank loadout found for player %1", playerId), LogLevel.WARNING);
	}
}
