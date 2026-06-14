modded class ARMST_EDITOR_GLOBAL_SETTINGS : SCR_BaseGameModeComponent
{
    // Respawn timer list
    protected ref array<ref ARMST_RespawnTimer> m_aRespawnTimers = new array<ref ARMST_RespawnTimer>();

    //------------------------------------------------------------------------------------------------
    // Called by SCR_AIGroup destructor to register a respawn timer
    void RegisterRespawnTimer(ARMST_RespawnTimer timer)
    {
        if (!timer)
            return;

        m_aRespawnTimers.Insert(timer);
        Print(string.Format("[ARMST_RespawnTimer] Registered timer for location: %1. Active timers: %2",
            typename.EnumToString(ARMST_LOCATION_LABEL, timer.GetLocationKey()),
            m_aRespawnTimers.Count()), LogLevel.NORMAL);

        // Hook the tick into the callqueue if this is the first timer
        if (m_aRespawnTimers.Count() == 1)
            GetGame().GetCallqueue().CallLater(TickRespawnTimers, 1000, true);
    }

    //------------------------------------------------------------------------------------------------
    // Ticks all active respawn timers every second
    protected void TickRespawnTimers()
    {
        for (int i = m_aRespawnTimers.Count() - 1; i >= 0; i--)
        {
            ARMST_RespawnTimer timer = m_aRespawnTimers[i];
            if (!timer || timer.Update(1.0))
            {
                m_aRespawnTimers.Remove(i);
                Print(string.Format("[ARMST_RespawnTimer] Timer completed and removed. Remaining: %1",
                    m_aRespawnTimers.Count()), LogLevel.NORMAL);
            }
        }

        // Stop ticking if no timers are left
        if (m_aRespawnTimers.IsEmpty())
            GetGame().GetCallqueue().Remove(TickRespawnTimers);
    }

    //------------------------------------------------------------------------------------------------
    // Called by ARMST_RespawnTimer when the delay expires
    void RequestGroupSpawn(ARMST_LOCATION_LABEL locationKey)
    {
        foreach (ARMST_ALIFE_LOCATION_DATA data : m_LocationData)
        {
            if (data.m_LocationKey != locationKey)
                continue;

            Print(string.Format("[ARMST_RespawnTimer] RequestGroupSpawn for location: %1 (current: %2, min: %3)",
                data.m_LocationName, data.m_CurrentGroupCount, data.m_MinGroupsLocation), LogLevel.NORMAL);

            // Only spawn if players are nearby and we are below the minimum
            if (!IsPlayerInRadius(data.m_LocationEpicenter, data.m_LocationRadius))
            {
                Print(string.Format("[ARMST_RespawnTimer] No players near %1, skipping respawn.", data.m_LocationName), LogLevel.NORMAL);
                return;
            }

            if (data.m_CurrentGroupCount < data.m_MinGroupsLocation)
                SpawnGroupInLocation(data);

            return;
        }

        Print(string.Format("[ARMST_RespawnTimer] WARNING: Location key %1 not found in location data.",
            typename.EnumToString(ARMST_LOCATION_LABEL, locationKey)), LogLevel.WARNING);
    }

    //------------------------------------------------------------------------------------------------
    // Returns the respawn delay for a given location from config
    int GetRespawnDelayForLocation(ARMST_LOCATION_LABEL locationKey)
    {
        foreach (ARMST_ALIFE_LOCATION_DATA data : m_LocationData)
        {
            if (data.m_LocationKey == locationKey)
                return data.m_CheckTimerLocation; // reuse existing check interval as respawn delay
        }
        return 120; // fallback default
    }
}