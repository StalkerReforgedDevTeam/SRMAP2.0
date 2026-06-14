class SRZ_USER_SET_FACTION : ScriptedUserAction
{
	[Attribute("1", UIWidgets.ComboBox, "Faction to set", "", ParamEnumArray.FromEnum(ARMST_FACTION_LABEL), category: "Faction")]
	protected ARMST_FACTION_LABEL m_FactionKey;

	[Attribute("0", UIWidgets.CheckBox, "Delete entity after use", "", category: "Behavior")]
	protected bool m_DeleteOwnerAfterUse;

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!Replication.IsServer())
			return;

		if (!pUserEntity)
			return;

		ARMST_PLAYER_STATS_COMPONENT playerStats =
			ARMST_PLAYER_STATS_COMPONENT.Cast(pUserEntity.FindComponent(ARMST_PLAYER_STATS_COMPONENT));

		if (!playerStats)
			return;

		playerStats.SetFactionKey(m_FactionKey);

		Print(string.Format("[SRZ_FACTION] Set faction to %1 for user %2", m_FactionKey, pUserEntity), LogLevel.NORMAL);

		if (m_DeleteOwnerAfterUse && pOwnerEntity)
			SCR_EntityHelper.DeleteEntityAndChildren(pOwnerEntity);
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		outName = "Set Faction";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		return true;
	}
}