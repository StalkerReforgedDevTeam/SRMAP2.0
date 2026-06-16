modded class SCR_LoiterUserAction : SCR_UserActionWithOccupancy
{
    override bool CanBeShownScript(IEntity user)
    {
        return false;
    }

    override bool CanBePerformedScript(IEntity user)
    {
        return false;
    }
}