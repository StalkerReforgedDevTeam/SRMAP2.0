modded class EPF_ArmstPlayerStatsComponentSaveData : EPF_ComponentSaveData
{
	[Attribute("0", UIWidgets.None, "Persistently saved STALKER rank index")]
	protected SR_STALKER_RANK m_eSRSavedRank; 
    
    //------------------------------------------------------------------------------------------------
    override EPF_EReadResult ReadFrom(IEntity owner, GenericComponent component, EPF_ComponentSaveDataClass attributes)
    {
		// 1. Run the base mod's original save code first (saving hunger, water, money)
        EPF_EReadResult result = super.ReadFrom(owner, component, attributes);
		if (result == EPF_EReadResult.ERROR)
			return result;

        ARMST_PLAYER_STATS_COMPONENT statsComponent = ARMST_PLAYER_STATS_COMPONENT.Cast(component);
        if (statsComponent)
        {
			// 2. Safely grab your injected rank variable
			m_eSRSavedRank = statsComponent.SR_GetRank();
		}
        PrintFormat("[SR Ranks] Player rank ReadFrom is: %1", m_eSRSavedRank, LogLevel.NORMAL);
        return EPF_EReadResult.OK;
    }

    //------------------------------------------------------------------------------------------------
	override EPF_EApplyResult ApplyTo(IEntity owner, GenericComponent component, EPF_ComponentSaveDataClass attributes)
	{
		// 1. Run the base mod's original load code first (loading hunger, water, money)
	    EPF_EApplyResult result = super.ApplyTo(owner, component, attributes);
		if (result == EPF_EApplyResult.ERROR)
			return result;

	    ARMST_PLAYER_STATS_COMPONENT statsComponent = ARMST_PLAYER_STATS_COMPONENT.Cast(component);
	    if (statsComponent)
	    {
			// 2. Safely push your saved rank back into the live session
			statsComponent.Rpc_SR_SetRank(m_eSRSavedRank);
		}
	    PrintFormat("[SR Ranks] Player rank ApplyTo is: %1", m_eSRSavedRank, LogLevel.NORMAL);
	    return EPF_EApplyResult.OK;
	}

    //------------------------------------------------------------------------------------------------
    override bool Equals(notnull EPF_ComponentSaveData other)
    {
		// 1. Check if the base mod values match first
        if (!super.Equals(other))
			return false;

        EPF_ArmstPlayerStatsComponentSaveData otherData = EPF_ArmstPlayerStatsComponentSaveData.Cast(other);
        if (!otherData)
            return false;
        
		// 2. Add your custom rank comparison to the check loop
		PrintFormat("[SR Ranks] Player rank Equals is: %1 | Player rank otherData is %2", m_eSRSavedRank, otherData.m_eSRSavedRank, LogLevel.NORMAL);
        return m_eSRSavedRank == otherData.m_eSRSavedRank; 
    }
}