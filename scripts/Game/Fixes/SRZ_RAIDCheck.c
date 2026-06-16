modded class ARMST_BasicSpawnLogic : EPF_BasicSpawnLogic
{
    //------------------------------------------------------------------------------------------------
    override void CheckRAID(IEntity player)
    {
        if (!player)
            return;

        ARMST_RAID_TIMER_COMPONENT raidTimer = ARMST_RAID_TIMER_COMPONENT.Cast(player.FindComponent(ARMST_RAID_TIMER_COMPONENT));
        if (raidTimer && !raidTimer.IsPlayerInSafeZone())
        {
            raidTimer.KillPlayer();
        }
    }
}