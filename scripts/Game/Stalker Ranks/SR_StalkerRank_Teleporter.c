modded class ARMST_TELEPORT_ACTIONS : ScriptedUserAction
{    
	
	[Attribute("0", UIWidgets.ComboBox, "Stalker rank", "", ParamEnumArray.FromEnum(SR_STALKER_RANK), category: "Requirements")]
	protected SR_STALKER_RANK m_eNeededRank;
	
    
    //------------------------------------------------------------------------------------------------
    override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
    {
		
		if(!CheckStalkerRankRequirements(pUserEntity))
			return;
		
		super.PerformAction(pOwnerEntity, pUserEntity);

    }
    //------------------------------------------------------------------------------------------------
	
	bool CheckStalkerRankRequirements(IEntity ent) 
	{
		if(!ent)
		{
			Print("[SR Ranks] Ent not found");
			return false;
		}
		ARMST_PLAYER_STATS_COMPONENT SR_APSC = ARMST_PLAYER_STATS_COMPONENT.Cast(ent.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
		if(!SR_APSC)
		{
			Print("[SR Ranks] ARMST_PLAYER_STATS_COMPONENT NOT FOUND!", LogLevel.ERROR);
			return false;
		}
		SR_STALKER_RANK playerRank = SR_APSC.SR_GetRank();
		
		if(!playerRank)
		{
			Print("[SR Ranks] playerRank not found");
			playerRank = SR_STALKER_RANK.ROOKIE;
		}
		
		if(playerRank >= m_eNeededRank) 
		{
			SCR_PlayerController.ShowNotificationPDA(ent, "Stalker Rank System", "Teleporting in 3...", true);
			return true;
		}
		else 
		{ 	
			SCR_PlayerController.ShowNotificationPDA(ent, "Stalker Rank System", "Not correct rank", true);
			PrintFormat("[SR Ranks] playerRank is %1 | m_eNeededRank is %2", playerRank, m_eNeededRank);
			return false;
		}
		
	}
}