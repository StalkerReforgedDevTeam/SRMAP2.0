[BaseContainerProps(category: "Respawn")]
modded class ARMST_BasicSpawnLogic: EPF_BasicSpawnLogic
{

    //------------------------------------------------------------------------------------------------
    override void HandoverToPlayer(int playerId, IEntity character)
    {
		
		PrintFormat("HandoverToPlayer(%1, %2)", playerId, character);
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
		EDF_ScriptInvokerCallback2<IEntity, IEntity> callback(this, "OnHandoverComplete", new Tuple1<int>(playerId));
		playerController.m_OnControlledEntityChanged.Insert(callback.Invoke);

		playerController.SetInitialMainEntity(character);

		
		const SCR_BaseGameMode gamemode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		gamemode.OnPlayerEntityChanged_S(playerId, null, character);

		
		SCR_RespawnComponent respawn = SCR_RespawnComponent.Cast(playerController.GetRespawnComponent());
		respawn.NotifySpawn(character);

        // Проверка и запуск UI создания персонажа
        ARMST_PLAYER_STATS_COMPONENT statsComponent = ARMST_PLAYER_STATS_COMPONENT.Cast(character.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
	    
        if (statsComponent)
        {
            string name = "";
            string bio = "";
            string head = "";
			
			SR_STALKER_RANK stalkerRank = statsComponent.SR_GetRank();
			ARMST_FACTION_LABEL faction = statsComponent.GetFactionKey();
			
			if(!stalkerRank) {
				stalkerRank = SR_STALKER_RANK.ROOKIE;	
			}
			
			if(!faction) {
            	faction = ARMST_FACTION_LABEL.FACTION_STALKER;
			}

            if (m_mPlayerNames.Contains(playerId))
                name = m_mPlayerNames.Get(playerId);
            
            if (m_mPlayerBiographies.Contains(playerId))
                bio = m_mPlayerBiographies.Get(playerId);

            if (m_mPlayerHeads.Contains(playerId))
                head = m_mPlayerHeads.Get(playerId);
            
            if (m_mPlayerFactions.Contains(playerId))
                faction = m_mPlayerFactions.Get(playerId);

            // Применяем данные
            statsComponent.SetFactionKey(faction);
			statsComponent.SR_SetRank(stalkerRank);
            
            if (!head.IsEmpty())
            {
                statsComponent.ArmstPlayerSetName(name);
                statsComponent.ArmstPlayerSetBiography(bio);
                statsComponent.ArmstPlayerSetHead(head);
            }
            
            // Очищаем данные из карты после использования, чтобы не засорять память
            m_mPlayerNames.Remove(playerId);
            m_mPlayerBiographies.Remove(playerId);
            m_mPlayerHeads.Remove(playerId);
            m_mPlayerFactions.Remove(playerId);
	        //Print("[ARMST_PLAYER_STATS] Отложенный вызов SetHead для установки головы игрока " + playerId, LogLevel.NORMAL);
	    }
		
		GetGame().GetCallqueue().CallLater(DelayedCreateUI, 6000, false, playerId);
        // Перенос денег на нового персонажа
        if (m_mPlayerMoneyToTransfer.Contains(playerId))
        {
            int moneyToTransfer = m_mPlayerMoneyToTransfer.Get(playerId);
            if (moneyToTransfer > 0)
            {
                SCR_InventoryStorageManagerComponent inventoryManager = SCR_InventoryStorageManagerComponent.Cast(character.FindComponent(SCR_InventoryStorageManagerComponent));
                if (inventoryManager)
                {
					GetGame().GetCallqueue().CallLater(DelayedSetHead, 2000, false, playerId, statsComponent.ArmstPlayerGetHead());
                    GetGame().GetCallqueue().CallLater(statsComponent.Rpc_UpdatePlayerFaction, 1000, false);
                    GetGame().GetCallqueue().CallLater(AddCurrency, 1000, false, character, moneyToTransfer);
                    //Print(string.Format("[ARMST_MONEY] Перенесено %1 денег на нового персонажа для игрока %2.", moneyToTransfer, playerId));
                }
            }
            m_mPlayerMoneyToTransfer.Remove(playerId);
        }

        // Перенос данных о квестах на нового персонажа
        if (m_mPlayerQuestDataToTransfer.Contains(playerId))
        {
            string questDataToTransfer = m_mPlayerQuestDataToTransfer.Get(playerId);
            if (!questDataToTransfer.IsEmpty())
            {
                ARMST_PLAYER_QUEST questComponent = ARMST_PLAYER_QUEST.Cast(character.FindComponent(ARMST_PLAYER_QUEST));
                if (questComponent)
                {
                    questComponent.m_player_quest_data = questDataToTransfer;
                    //Print(string.Format("[ARMST_QUEST] Перенесены данные о квестах для игрока %1: %2", playerId, questDataToTransfer));
                }
                else
                {
                    Print(string.Format("[ARMST_QUEST] Ошибка: Компонент ARMST_PLAYER_QUEST не найден на новом персонаже игрока %1.", playerId), LogLevel.ERROR);
                }
            }
            m_mPlayerQuestDataToTransfer.Remove(playerId);
        }
		
        if (m_mPlayerReputationToTransfer.Contains(playerId))
        {
            ref map<ARMST_FACTION_LABEL, float> reputationToTransfer = m_mPlayerReputationToTransfer.Get(playerId);
            if (reputationToTransfer && !reputationToTransfer.IsEmpty())
            {
                ARMST_PLAYER_REPUTATIONS_COMPONENT reputationComponent = ARMST_PLAYER_REPUTATIONS_COMPONENT.Cast(character.FindComponent(ARMST_PLAYER_REPUTATIONS_COMPONENT));
                if (reputationComponent)
                {
                    // Применяем сохранённые значения репутации
                    if (reputationToTransfer.Contains(reputationComponent.m_FactionKey1))
                        reputationComponent.SetReputation(reputationComponent.m_FactionKey1, reputationToTransfer.Get(reputationComponent.m_FactionKey1));
                    if (reputationToTransfer.Contains(reputationComponent.m_FactionKey2))
                        reputationComponent.SetReputation(reputationComponent.m_FactionKey2, reputationToTransfer.Get(reputationComponent.m_FactionKey2));
                    if (reputationToTransfer.Contains(reputationComponent.m_FactionKey3))
                        reputationComponent.SetReputation(reputationComponent.m_FactionKey3, reputationToTransfer.Get(reputationComponent.m_FactionKey3));
                    if (reputationToTransfer.Contains(reputationComponent.m_FactionKey4))
                        reputationComponent.SetReputation(reputationComponent.m_FactionKey4, reputationToTransfer.Get(reputationComponent.m_FactionKey4));
                    if (reputationToTransfer.Contains(reputationComponent.m_FactionKey5))
                        reputationComponent.SetReputation(reputationComponent.m_FactionKey5, reputationToTransfer.Get(reputationComponent.m_FactionKey5));
                    if (reputationToTransfer.Contains(reputationComponent.m_FactionKey6))
                        reputationComponent.SetReputation(reputationComponent.m_FactionKey6, reputationToTransfer.Get(reputationComponent.m_FactionKey6));
                    if (reputationToTransfer.Contains(reputationComponent.m_FactionKey7))
                        reputationComponent.SetReputation(reputationComponent.m_FactionKey7, reputationToTransfer.Get(reputationComponent.m_FactionKey7));
                    if (reputationToTransfer.Contains(reputationComponent.m_FactionKey8))
                        reputationComponent.SetReputation(reputationComponent.m_FactionKey8, reputationToTransfer.Get(reputationComponent.m_FactionKey8));
                    if (reputationToTransfer.Contains(reputationComponent.m_FactionKey9))
                        reputationComponent.SetReputation(reputationComponent.m_FactionKey9, reputationToTransfer.Get(reputationComponent.m_FactionKey9));
                    
                    Print(string.Format("[ARMST_REPUTATION] Перенесены данные о репутации для игрока %1.", playerId));
                }
                else
                {
                    Print(string.Format("[ARMST_REPUTATION] Ошибка: Компонент ARMST_PLAYER_REPUTATIONS_COMPONENT не найден на новом персонаже игрока %1.", playerId), LogLevel.ERROR);
                }
            }
            m_mPlayerReputationToTransfer.Remove(playerId);
        }
		
		
        FactionAffiliationComponent factionComponent = FactionAffiliationComponent.Cast(character.FindComponent(FactionAffiliationComponent));
        if (!factionComponent)
            return;
		
    }

   
   
   
}