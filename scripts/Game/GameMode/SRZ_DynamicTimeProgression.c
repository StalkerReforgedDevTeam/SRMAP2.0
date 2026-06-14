// Scripts/Game/GameMode/SRZ_DynamicTimeProgression.c
// Server-side dynamic time progression - 4x day, 10x night

modded class SCR_BaseGameMode : BaseGameMode
{
    protected const float DAY_MULTIPLIER = 4.0;
    protected const float NIGHT_MULTIPLIER = 10.0;
    protected const float SUNRISE_HOUR = 8.0;
    protected const float SUNSET_HOUR = 21.0;
    protected const float CHECK_INTERVAL_MS = 30000;
    protected const int DAY_DURATION = 86400;
    
    protected bool m_bDynamicTimeInitialized = false;
    protected bool m_bCurrentlyDay = true;
    protected TimeAndWeatherManagerEntity m_TimeManager;
    
    override void OnGameStart()
    {
        super.OnGameStart();
        
        Print("[DynamicTime] OnGameStart called");
        
        GetGame().GetCallqueue().CallLater(InitDynamicTimeProgression, 2000, false);
    }
    
    protected TimeAndWeatherManagerEntity GetTimeManager()
    {
        ChimeraWorld world = ChimeraWorld.CastFrom(GetWorld());
        if (!world)
            return null;
        
        return world.GetTimeAndWeatherManager();
    }
    
    protected void InitDynamicTimeProgression()
    {
        if (m_bDynamicTimeInitialized)
            return;
        
        Print("[DynamicTime] Attempting to initialize...");
        
        m_TimeManager = GetTimeManager();
        if (!m_TimeManager)
        {
            Print("[DynamicTime] TimeManager not found yet");
            GetGame().GetCallqueue().CallLater(InitDynamicTimeProgression, 2000, false);
            return;
        }
        
        float hour = m_TimeManager.GetTimeOfTheDay();
        
        m_bCurrentlyDay = IsDayTime(hour);
        
        float mult;
        string mode;
        
        if (m_bCurrentlyDay)
        {
            mult = DAY_MULTIPLIER;
            mode = "DAY";
        }
        else
        {
            mult = NIGHT_MULTIPLIER;
            mode = "NIGHT";
        }
        
        ApplyTimeMultiplier(mult);
        
        Print("[DynamicTime] Initialized - " + mode + " mode (" + mult.ToString() + "x)");
        
        GetGame().GetCallqueue().CallLater(CheckTimeProgression, CHECK_INTERVAL_MS, true);
        
        m_bDynamicTimeInitialized = true;
    }
    
    protected void CheckTimeProgression()
    {
        if (!m_TimeManager)
            return;
        
        float hour = m_TimeManager.GetTimeOfTheDay();
        
        bool isDay = IsDayTime(hour);
        
        if (isDay == m_bCurrentlyDay)
            return;
        
        m_bCurrentlyDay = isDay;
        
        float newMultiplier;
        string mode;
        
        if (isDay)
        {
            newMultiplier = DAY_MULTIPLIER;
            mode = "DAY";
        }
        else
        {
            newMultiplier = NIGHT_MULTIPLIER;
            mode = "NIGHT";
        }
        
        ApplyTimeMultiplier(newMultiplier);
        
        Print("[DynamicTime] Transition - Now " + mode + " (" + newMultiplier.ToString() + "x)");
    }
    
    protected bool IsDayTime(float timeOfDay)
    {
        if (timeOfDay >= SUNRISE_HOUR && timeOfDay < SUNSET_HOUR)
            return true;
        
        return false;
    }
    
    protected void ApplyTimeMultiplier(float multiplier)
    {
        if (!m_TimeManager)
            return;
        
        float duration = DAY_DURATION / multiplier;
        m_TimeManager.SetDayDuration(duration);
    }
}