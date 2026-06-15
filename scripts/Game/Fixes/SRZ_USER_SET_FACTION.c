class SRZ_USER_SET_FACTION : ScriptedUserAction
{
    [Attribute("1", UIWidgets.ComboBox, "Faction to set", "", ParamEnumArray.FromEnum(ARMST_FACTION_LABEL), category: "Faction")]
    protected ARMST_FACTION_LABEL m_FactionKey;

    [Attribute("false", UIWidgets.CheckBox, "Delete entity after use", "If enabled, the entity this action is on will be deleted after use.", category: "Behavior")]
    protected bool m_DeleteOwnerAfterUse; // removed = false

    //------------------------------------------------------------------------------------------------
    override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
    {
        if (!pUserEntity)
            return;

        ARMST_PLAYER_STATS_COMPONENT playerStats = ARMST_PLAYER_STATS_COMPONENT.Cast(pUserEntity.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
        if (!playerStats)
            return;

        playerStats.SetFactionKey(m_FactionKey);

        if (m_DeleteOwnerAfterUse && pOwnerEntity)
            SCR_EntityHelper.DeleteEntityAndChildren(pOwnerEntity);
    }

    //------------------------------------------------------------------------------------------------
    override bool CanBePerformedScript(IEntity user)
    {
        return true;
    }

    //------------------------------------------------------------------------------------------------
    override bool CanBeShownScript(IEntity user)
    {
        return true;
    }

    //------------------------------------------------------------------------------------------------
    override bool GetActionNameScript(out string outName)
    {
        outName = "#set_faction_action";
        return true;
    }
}