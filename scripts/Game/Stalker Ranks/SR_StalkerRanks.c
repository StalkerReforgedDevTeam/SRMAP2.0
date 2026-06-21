enum SR_STALKER_RANK 
{
	ROOKIE,
	EXPERIENCED,
	VETERAN,
	EXPERT,
	MASTER,
}


// Extends their component without touching their actual file source
modded class ARMST_PLAYER_STATS_COMPONENT : ScriptComponent
{
	// Prefixing variables prevents clashes with future base-mod updates
	[RplProp(onRplName: "SR_OnRankChanged")]
	protected SR_STALKER_RANK m_eSRCurrentRank = SR_STALKER_RANK.ROOKIE;

	//------------------------------------------------------------------------------------------------
	SR_STALKER_RANK SR_GetRank()
	{
		PrintFormat("[SR Ranks] Player rank SR_GetRank is: %1", m_eSRCurrentRank, LogLevel.NORMAL);
		return m_eSRCurrentRank;
	}

	//------------------------------------------------------------------------------------------------
	void SR_SetRank(SR_STALKER_RANK newRank)
	{
		RplComponent rpl = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		if (rpl && !rpl.IsMaster())
			return;
		
		m_eSRCurrentRank = newRank;
		
		// Tells Enfusion to replicate this component immediately
		Replication.BumpMe();
		PrintFormat("[SR Ranks] Player rank SR_SetRank is: %1 | newRank is %2", m_eSRCurrentRank, newRank, LogLevel.NORMAL);	
		SR_OnRankChanged();
	}

	//------------------------------------------------------------------------------------------------
	protected void SR_OnRankChanged()
	{
		Print(string.Format("[SR Ranks] Player rank synchronized: %1", m_eSRCurrentRank.ToString()));
	}
	
	// Replicate the rank across the network
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void Rpc_SR_SetRank(SR_STALKER_RANK value)
	{
		SR_SetRank(value);
	}
} 	
