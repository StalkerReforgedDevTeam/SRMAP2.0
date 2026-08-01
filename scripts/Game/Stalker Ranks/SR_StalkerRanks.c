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
		return m_eSRCurrentRank;
	}

	//------------------------------------------------------------------------------------------------
	void SR_SetRank(SR_STALKER_RANK newRank)
	{
		m_eSRCurrentRank = newRank;
		Replication.BumpMe();
		SR_OnRankChanged();
	}

	//------------------------------------------------------------------------------------------------
	protected void SR_OnRankChanged()
	{
		// Intentionally silent - was spamming logs on every replication update
	}

	// Replicate the rank across the network
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void Rpc_SR_SetRank(SR_STALKER_RANK value)
	{
		SR_SetRank(value);
	}
}