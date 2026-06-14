class ARMST_RespawnTimer
{
    private float m_fElapsed = 0.0;
    private bool m_bActive = false;
    private ARMST_LOCATION_LABEL m_LocationKey;
    private ARMST_EDITOR_GLOBAL_SETTINGS m_Alife;
    private int m_RespawnDelay;

    void ARMST_RespawnTimer(ARMST_LOCATION_LABEL locationKey, ARMST_EDITOR_GLOBAL_SETTINGS alife, int respawnDelay)
    {
        m_LocationKey = locationKey;
        m_Alife = alife;
        m_RespawnDelay = respawnDelay;
    }

    void Start()
    {
        m_fElapsed = 0.0;
        m_bActive = true;
        Print(string.Format("[ARMST_RespawnTimer] Timer started for location: %1 (delay: %2s)",
            typename.EnumToString(ARMST_LOCATION_LABEL, m_LocationKey), m_RespawnDelay), LogLevel.NORMAL);
    }

    bool Update(float timeSlice)
    {
        if (!m_bActive)
            return false;

        m_fElapsed += timeSlice;

        if (m_fElapsed >= m_RespawnDelay)
        {
            m_bActive = false;
            FireRespawn();
            return true;
        }

        return false;
    }

    private void FireRespawn()
    {
        if (!m_Alife)
        {
            Print("[ARMST_RespawnTimer] ERROR - Alife reference is null!", LogLevel.ERROR);
            return;
        }

        Print(string.Format("[ARMST_RespawnTimer] Timer expired, requesting respawn for location: %1",
            typename.EnumToString(ARMST_LOCATION_LABEL, m_LocationKey)), LogLevel.NORMAL);

        m_Alife.RequestGroupSpawn(m_LocationKey);
    }

    bool IsActive() { return m_bActive; }
    ARMST_LOCATION_LABEL GetLocationKey() { return m_LocationKey; }
}