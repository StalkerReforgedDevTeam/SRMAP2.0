// Scripts/Game/GameMode/SRZ_DynamicTimeProgression.c
// Server-side dynamic time progression + rain frequency control

modded class SCR_BaseGameMode : BaseGameMode
{
	protected const float DAY_MULTIPLIER = 3.0;
	protected const float NIGHT_MULTIPLIER = 6.0;
	protected const float SUNRISE_HOUR = 8.0;
	protected const float SUNSET_HOUR = 21.0;
	protected const float CHECK_INTERVAL_MS = 30000;
	protected const int DAY_DURATION = 86400;

	// Weather control
	protected const float WEATHER_CHECK_INTERVAL_MS = 600000; // check every 10 min
	protected const float RAIN_CHANCE = 0.10; // 10% chance to roll rain each check
	protected const float RAIN_STATE_DURATION = 900; // rain lasts ~15 min in-game seconds if it happens
	protected const float CLEAR_TRANSITION_DURATION = 60;

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

		Print("[DynamicTime] Initialized - " + mode + " mode (" + mult.ToString() + "x) at hour " + hour.ToString());

		GetGame().GetCallqueue().CallLater(CheckTimeProgression, CHECK_INTERVAL_MS, true);

		// Start rain control loop
		GetGame().GetCallqueue().CallLater(CheckWeatherProgression, WEATHER_CHECK_INTERVAL_MS, true);

		// Force clear skies immediately on init so we don't inherit a rainy start
		m_TimeManager.ForceWeatherTo(false, "Clear", CLEAR_TRANSITION_DURATION, 0.001, 0);

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

		Print("[DynamicTime] Transition - Now " + mode + " (" + newMultiplier.ToString() + "x) at hour " + hour.ToString());
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

	//------------------------------------------------------------------------------------------------
	// Weather control - keeps things mostly clear, rare short rain
	//------------------------------------------------------------------------------------------------
	protected void CheckWeatherProgression()
	{
		if (!m_TimeManager)
			return;

		float roll = Math.RandomFloat01();

		if (roll <= RAIN_CHANCE)
		{
			m_TimeManager.ForceWeatherTo(false, "Rain", 120, RAIN_STATE_DURATION, 0);
			Print("[DynamicWeather] Rolled rain - short shower incoming");
		}
		else
		{
			m_TimeManager.ForceWeatherTo(false, "Clear", CLEAR_TRANSITION_DURATION, 0.001, 0);
		}
	}
}