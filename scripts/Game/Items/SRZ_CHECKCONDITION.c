modded class ARMST_USER_CHECK_COND : ScriptedUserAction
{
    override bool GetActionNameScript(out string outName)
    {
        ARMST_ITEMS_STATS_COMPONENTS itemStats = ARMST_ITEMS_STATS_COMPONENTS.Cast(GetOwner().FindComponent(ARMST_ITEMS_STATS_COMPONENTS));
        if (!itemStats)
            return false;

        float conditionLevel = itemStats.GetConditionLevel();
        outName = string.Format("Condition: %1%", Math.Round(conditionLevel));
        return true;
    }

    override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
    {
    }
}