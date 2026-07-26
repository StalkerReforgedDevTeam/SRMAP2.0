// scripts/Game/Fixes/SRZ_TeleportRankAndRandomDestination.c

modded class ARMST_TELEPORT_ACTIONS : ScriptedUserAction
{
	[Attribute("0", UIWidgets.ComboBox, "Stalker rank", "", ParamEnumArray.FromEnum(SR_STALKER_RANK), category: "Requirements")]
	protected SR_STALKER_RANK m_eNeededRank;

	[Attribute("false", UIWidgets.CheckBox, "Pick randomly between up to 3 target entities below instead of the single target above", category: "Random Destination")]
	protected bool m_bRandomizeDestination;

	[Attribute("", UIWidgets.EditBox, "Second possible target entity name (only used if Randomize is enabled)", category: "Random Destination")]
	protected string m_sTeleportTarget2;

	[Attribute("", UIWidgets.EditBox, "Third possible target entity name (only used if Randomize is enabled)", category: "Random Destination")]
	protected string m_sTeleportTarget3;

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!CheckStalkerRankRequirements(pUserEntity))
			return;

		super.PerformAction(pOwnerEntity, pUserEntity);
	}

	//------------------------------------------------------------------------------------------------
	bool CheckStalkerRankRequirements(IEntity ent)
	{
		if (!ent)
		{
			Print("[SR Ranks] Ent not found");
			return false;
		}
		ARMST_PLAYER_STATS_COMPONENT SR_APSC = ARMST_PLAYER_STATS_COMPONENT.Cast(ent.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
		if (!SR_APSC)
		{
			Print("[SR Ranks] ARMST_PLAYER_STATS_COMPONENT NOT FOUND!", LogLevel.ERROR);
			return false;
		}
		SR_STALKER_RANK playerRank = SR_APSC.SR_GetRank();

		if (!playerRank)
		{
			Print("[SR Ranks] playerRank not found");
			playerRank = SR_STALKER_RANK.ROOKIE;
		}

		if (playerRank >= m_eNeededRank)
		{
			SCR_PlayerController.ShowNotificationPDA(ent, "Stalker Rank System", "Teleporting in 3...", false);
			return true;
		}
		else
		{
			SCR_PlayerController.ShowNotificationPDA(ent, "Stalker Rank System", "Not correct rank", false);
			PrintFormat("[SR Ranks] playerRank is %1 | m_eNeededRank is %2", playerRank, m_eNeededRank);
			return false;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected string GetChosenTeleportTargetName()
	{
		if (!m_bRandomizeDestination)
			return m_sTeleportTarget;

		array<string> validTargets = {};
		if (!m_sTeleportTarget.IsEmpty())
			validTargets.Insert(m_sTeleportTarget);
		if (!m_sTeleportTarget2.IsEmpty())
			validTargets.Insert(m_sTeleportTarget2);
		if (!m_sTeleportTarget3.IsEmpty())
			validTargets.Insert(m_sTeleportTarget3);

		if (validTargets.IsEmpty())
			return m_sTeleportTarget;

		int idx = Math.RandomInt(0, validTargets.Count());
		return validTargets[idx];
	}

	//------------------------------------------------------------------------------------------------
	override void PerformTeleport(IEntity pUserEntity)
	{
		string chosenTargetName = GetChosenTeleportTargetName();
		IEntity targetEntity = GetGame().FindEntity(chosenTargetName);
		if (!targetEntity)
		{
			m_bTeleportInProgress = false;
			return;
		}

		if (!pUserEntity)
			return;

		SCR_ChimeraCharacter owner2 = SCR_ChimeraCharacter.Cast(pUserEntity);
		if (!owner2)
			return;
		CharacterControllerComponent contr = owner2.GetCharacterController();
		if (!contr)
			return;

		if (contr.GetLifeState() == ECharacterLifeState.DEAD)
			return;

		if (m_iCost > 0)
		{
			ARMST_PLAYER_STATS_COMPONENT currencyComp = ARMST_PLAYER_STATS_COMPONENT.Cast(pUserEntity.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
			int totalCurrency = currencyComp.GetValue();
			if (totalCurrency < m_iCost)
			{
				return;
			}
			else
			{
				currencyComp.ModifyValue(m_iCost, false);
			}
		}

		if (m_sRequiredItem != "")
		{
			SCR_InventoryStorageManagerComponent inventoryManager = SCR_InventoryStorageManagerComponent.Cast(pUserEntity.FindComponent(SCR_InventoryStorageManagerComponent));
			if (inventoryManager)
			{
				array<IEntity> items = new array<IEntity>();
				B_PrefabNamePredicate pred = new B_PrefabNamePredicate();
				pred.prefabName.Insert(m_sRequiredItem);

				if (inventoryManager.FindItems(items, pred) && items.Count() > 0)
				{
					IEntity itemToRemove = items[0];
					if (m_DeleteItem)
						SCR_EntityHelper.DeleteEntityAndChildren(itemToRemove);
				}
				else
				{
					return;
				}
			}
		}

		vector targetPos = targetEntity.GetOrigin();
		if (m_InShelter)
		{
			ARMST_PLAYER_STATS_COMPONENT m_StatsComponent = ARMST_PLAYER_STATS_COMPONENT.Cast(pUserEntity.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
			if (!m_StatsComponent) return;
			targetPos = m_StatsComponent.ArmstPlayerGetShelter();
			if (targetPos == vector.Zero)
				return;
		}

		if (m_InShelterExit)
		{
			ARMST_SHELTER_GUID_COMPONENT Comp = ARMST_SHELTER_GUID_COMPONENT.Cast(GetOwner().FindComponent(ARMST_SHELTER_GUID_COMPONENT));
			if (Comp)
			{
				targetPos = Comp.GetExitPos();
			}
			else
			{
				Print("Не найден компонент");
				return;
			}
		}

		m_UserEntity.SetOrigin(targetPos);
		m_bTeleportInProgress = false;
	}
};