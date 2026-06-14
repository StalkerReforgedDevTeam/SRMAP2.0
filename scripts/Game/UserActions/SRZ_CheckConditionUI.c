modded class ARMST_USER_CHECK_COND : ScriptedUserAction
{
	
	override bool GetActionNameScript(out string outName)
	{
		ARMST_ITEMS_STATS_COMPONENTS cmdHandler = ARMST_ITEMS_STATS_COMPONENTS.Cast(GetOwner().FindComponent(ARMST_ITEMS_STATS_COMPONENTS));
			if (!cmdHandler)
				return false;
	    int repairKitName = cmdHandler.GetConditionLevel();
		string percentage = "%";
	    outName = string.Format("Condition: %1%2", repairKitName, percentage);
	    return true;
	}
    override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
    {
		
    }
    //------------------------------------------------------------------------------------------------
    void ~ARMST_USER_CHECK_COND()
    {
    }
};