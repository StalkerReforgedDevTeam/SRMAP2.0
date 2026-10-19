// Rank lock for ARMST teleport doors

modded class ARMST_TELEPORT_ACTIONS : ScriptedUserAction
{
	[Attribute("0", UIWidgets.ComboBox, "Stalker rank", "", ParamEnumArray.FromEnum(SR_STALKER_RANK), category: "Requirements")]
	protected SR_STALKER_RANK m_eNeededRank;

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!CheckStalkerRankRequirements(pUserEntity))
			return;

		super.PerformAction(pOwnerEntity, pUserEntity);
	}

	//------------------------------------------------------------------------------------------------
	// Options above the player's rank are hidden. Lower rank options stay available
	override bool CanBeShownScript(IEntity user)
	{
		if (!SR_HasNeededRank(user))
			return false;

		return super.CanBeShownScript(user);
	}

	//------------------------------------------------------------------------------------------------
	protected bool SR_HasNeededRank(IEntity user)
	{
		if (m_eNeededRank == SR_STALKER_RANK.ROOKIE)
			return true;

		if (!user)
			return false;

		ARMST_PLAYER_STATS_COMPONENT stats = ARMST_PLAYER_STATS_COMPONENT.Cast(user.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
		if (!stats)
			return false;

		return stats.SR_GetRank() >= m_eNeededRank;
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

};