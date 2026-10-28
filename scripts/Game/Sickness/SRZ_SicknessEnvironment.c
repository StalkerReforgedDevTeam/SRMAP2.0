// The environment pass, DayZ's Environment class equivalent. Server side, once per tick per player.
// Reads time of day, weather, roof, vehicle, nearby lit fires and the clothes worn, then moves
// temperature and wetness toward their targets.
class SRZ_SicknessEnvironment
{
	static const float TICK = 10;	// same as SRZ_Sickness.TICK

	// Temperature targets
	static const float CLEAR_DAY = 10;
	static const float DUSK_DAWN = -10;
	static const float NIGHT = -25;
	static const float RAIN_PENALTY = 15;
	static const float FIRE_TARGET = 20;
	static const float INSULATION_MAX_BONUS = 60;
	static const float TEMP_RATE = 0.5;			// units per second, about 10 minutes across a 30 wide band
	static const float FIRE_RANGE = 4;
	static const float FIRE_BUFF = 300;			// seconds of heat buff after leaving a fire

	// Wetness
	static const float WET_RAIN_LIGHT = 1;		// per tick in light rain
	static const float WET_RAIN_HEAVY = 2;
	static const float DRY_OPEN = 1;
	static const float DRY_ROOF = 3;
	static const float DRY_FIRE = 10;

	// Slot weights (DayZ)
	static const float W_JACKET = 0.23;
	static const float W_PANTS = 0.23;
	static const float W_BOOTS = 0.12;
	static const float W_HEAD = 0.10;
	static const float W_FACE = 0.10;
	static const float W_VEST = 0.08;
	static const float W_GLOVES = 0.08;
	static const float W_BACKPACK = 0.06;

	// Slot defaults used when the database row has no insulation value yet (0)
	static const float D_JACKET = 25;
	static const float D_PANTS = 25;
	static const float D_BOOTS = 30;
	static const float D_HEAD = 30;
	static const float D_FACE = 0;
	static const float D_VEST = 20;
	static const float D_GLOVES = 30;
	static const float D_BACKPACK = 10;

	protected static bool s_bFireFound;
	protected static vector s_vFireCenter;
	protected static ref set<string> s_LoggedFires = new set<string>();

	//------------------------------------------------------------------------------------------------
	static void Update(SRZ_SicknessStats st)
	{
		IEntity ent = st.m_Char;
		if (!ent)
			return;

		bool indoors = Helpers.IsPlayerIndoors(ent);
		bool roof = indoors;
		if (!roof)
			roof = Helpers.CheckRoof(ent.GetWorld(), EyePos(ent));

		bool vehicle = false;
		ChimeraCharacter chimera = ChimeraCharacter.Cast(ent);
		if (chimera)
			vehicle = chimera.IsInVehicle();

		bool fire = NearLitFire(ent);

		float rain = 0;
		float overcast = 0;
		bool night = false;
		bool duskDawn = false;
		ReadWeather(ent, rain, overcast, night, duskDawn);

		// Wetness first, it feeds insulation
		if (rain > 0.05 && !roof && !vehicle)
		{
			float gain = WET_RAIN_LIGHT;
			if (rain > 0.5)
				gain = WET_RAIN_HEAVY;

			if (HasWaterproofHead(ent))
				gain *= 0.5;

			st.m_fWetness = Math.Min(100, st.m_fWetness + gain);
		}
		else
		{
			float dry = DRY_OPEN;
			if (fire)
				dry = DRY_FIRE;
			else if (roof || vehicle)
				dry = DRY_ROOF;

			st.m_fWetness = Math.Max(0, st.m_fWetness - dry);
		}

		float insulation = Insulation(ent, st.m_fWetness);
		st.m_fInsulation = insulation;

		// Environment target
		float target;
		if (indoors || vehicle)
		{
			target = 0;
		}
		else
		{
			target = CLEAR_DAY - CLEAR_DAY * overcast;
			if (night)
				target = NIGHT;
			else if (duskDawn)
				target = DUSK_DAWN;

			if (rain > 0.05 && !roof)
				target -= RAIN_PENALTY * Math.Clamp(rain * 2, 0.5, 1);
		}

		if (fire)
		{
			target = Math.Max(target, FIRE_TARGET);
			st.m_fHeatBuff = Math.Max(st.m_fHeatBuff, FIRE_BUFF);
		}

		target += insulation / 100 * INSULATION_MAX_BONUS;
		target = Math.Clamp(target, -100, 100);
		st.m_fTarget = target;

		// Move toward the target
		float step = TEMP_RATE * TICK;
		if (st.m_fHeatBuff > 0)
		{
			st.m_fHeatBuff -= TICK;
			if (target < st.m_fTemperature)
				target = st.m_fTemperature;
		}

		if (target > st.m_fTemperature)
			st.m_fTemperature = Math.Min(target, st.m_fTemperature + step);
		else
			st.m_fTemperature = Math.Max(target, st.m_fTemperature - step);
	}

	//------------------------------------------------------------------------------------------------
	protected static vector EyePos(IEntity ent)
	{
		ChimeraCharacter chimera = ChimeraCharacter.Cast(ent);
		if (chimera)
			return chimera.EyePosition();

		return ent.GetOrigin() + vector.Up * 1.6;
	}

	//------------------------------------------------------------------------------------------------
	// Time of day and weather from the vanilla manager. Rain and overcast are 0 to 1.
	protected static void ReadWeather(IEntity ent, out float rain, out float overcast, out bool night, out bool duskDawn)
	{
		rain = 0;
		overcast = 0;
		night = false;
		duskDawn = false;

		ChimeraWorld world = GetGame().GetWorld();
		if (!world)
			return;

		TimeAndWeatherManagerEntity tw = world.GetTimeAndWeatherManager();
		if (!tw)
			return;

		float hour = tw.GetTimeOfTheDay();
		float sunrise = 6;
		float sunset = 19;
		tw.GetSunriseHour(sunrise);
		tw.GetSunsetHour(sunset);

		if (hour < sunrise - 1 || hour > sunset + 1)
			night = true;
		else if (hour < sunrise + 1 || hour > sunset - 1)
			duskDawn = true;

		rain = Math.Clamp(tw.GetRainIntensity(), 0, 1);

		LocalWeatherSituation lws = new LocalWeatherSituation();
		if (tw.TryGetCompleteLocalWeather(lws, 0, ent.GetOrigin()))
		{
			overcast = Math.Clamp(lws.GetOvercast(), 0, 1);
			rain = Math.Max(rain, Math.Clamp(lws.GetRainIntensity(), 0, 1));
		}
	}

	//------------------------------------------------------------------------------------------------
	// A fire counts when a SCR_FireplaceComponent within 4 m says it is on. ARMST's decorative
	// campfires that have no such component count by name and get logged once so we can see what they are.
	static bool NearLitFire(IEntity ent)
	{
		BaseWorld world = ent.GetWorld();
		if (!world)
			return false;

		s_bFireFound = false;
		s_vFireCenter = ent.GetOrigin();
		world.QueryEntitiesBySphere(s_vFireCenter, FIRE_RANGE, FireQuery, null, EQueryEntitiesFlags.ALL);
		return s_bFireFound;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool FireQuery(IEntity e)
	{
		if (s_bFireFound || !e)
			return true;

		if (IsLitFire(e))
		{
			s_bFireFound = true;
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool IsLitFire(IEntity e)
	{
		SCR_FireplaceComponent fireplace = SCR_FireplaceComponent.Cast(e.FindComponent(SCR_FireplaceComponent));
		if (fireplace)
			return fireplace.IsOn();

		if (!e.GetPrefabData())
			return false;

		string prefab = e.GetPrefabData().GetPrefabName();
		string lower = prefab;
		lower.ToLower();
		if (!lower.Contains("fireplace") && !lower.Contains("campfire") && !lower.Contains("fire_sandbox") && !lower.Contains("bonfire"))
			return false;

		if (!s_LoggedFires.Contains(prefab))
		{
			s_LoggedFires.Insert(prefab);
			Print("[SRZ Sickness] Fire by name (no SCR_FireplaceComponent): " + prefab);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	// Overall insulation 0 to 100, slot weighted (DayZ), each item scaled by condition and wetness.
	static float Insulation(IEntity ent, float wetness)
	{
		EquipedLoadoutStorageComponent loadout = EquipedLoadoutStorageComponent.Cast(ent.FindComponent(EquipedLoadoutStorageComponent));
		if (!loadout)
			return 0;

		float jacket = SlotValue(loadout.GetClothFromArea(LoadoutJacketArea), D_JACKET);
		float pants = SlotValue(loadout.GetClothFromArea(LoadoutPantsArea), D_PANTS);
		// Suits cover both; when the pants slot is empty but a jacket is worn, the jacket counts for the legs too
		if (pants <= 0 && jacket > 0)
			pants = jacket;

		float total = jacket * W_JACKET
			+ pants * W_PANTS
			+ SlotValue(loadout.GetClothFromArea(LoadoutBootsArea), D_BOOTS) * W_BOOTS
			+ SlotValue(loadout.GetClothFromArea(LoadoutHeadCoverArea), D_HEAD) * W_HEAD
			+ SlotValue(loadout.GetClothFromArea(ARMST_FaceArea), D_FACE) * W_FACE
			+ SlotValue(loadout.GetClothFromArea(LoadoutVestArea), D_VEST) * W_VEST
			+ SlotValue(loadout.GetClothFromArea(LoadoutHandwearSlotArea), D_GLOVES) * W_GLOVES
			+ SlotValue(loadout.GetClothFromArea(LoadoutBackpackArea), D_BACKPACK) * W_BACKPACK;

		return Math.Clamp(total * WetFactor(wetness), 0, 100);
	}

	//------------------------------------------------------------------------------------------------
	// One item's insulation after condition. Database value if set, otherwise the slot default plus half the
	// item's ARMST fire protection so sealed and armoured suits come out warm.
	protected static float SlotValue(IEntity item, float slotDefault)
	{
		if (!item)
			return 0;

		ARMST_ITEMS_STATS_COMPONENTS stats = ARMST_ITEMS_STATS_COMPONENTS.Cast(item.FindComponent(ARMST_ITEMS_STATS_COMPONENTS));
		if (!stats)
			return slotDefault;

		float value = stats.m_fSRZ_Insulation;
		if (value <= 0)
			value = Math.Min(100, slotDefault + stats.m_fProtectionThermoLevel * 0.5);

		return value * ConditionFactor(stats.GetConditionLevel());
	}

	//------------------------------------------------------------------------------------------------
	protected static float ConditionFactor(float condition)
	{
		if (condition >= 80) return 1;
		if (condition >= 60) return 0.9;
		if (condition >= 40) return 0.8;
		if (condition >= 20) return 0.5;
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	static float WetFactor(float wetness)
	{
		if (wetness < 20) return 1;
		if (wetness < 40) return 0.9;
		if (wetness < 60) return 0.5;
		if (wetness < 80) return 0.1;
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool HasWaterproofHead(IEntity ent)
	{
		EquipedLoadoutStorageComponent loadout = EquipedLoadoutStorageComponent.Cast(ent.FindComponent(EquipedLoadoutStorageComponent));
		if (!loadout)
			return false;

		array<IEntity> items = {loadout.GetClothFromArea(LoadoutHeadCoverArea), loadout.GetClothFromArea(LoadoutJacketArea)};
		foreach (IEntity item : items)
		{
			if (!item)
				continue;

			ARMST_ITEMS_STATS_COMPONENTS stats = ARMST_ITEMS_STATS_COMPONENTS.Cast(item.FindComponent(ARMST_ITEMS_STATS_COMPONENTS));
			if (stats && stats.m_bSRZ_Waterproof)
				return true;
		}

		// Helmets and hoods shed rain even without the flag
		IEntity head = loadout.GetClothFromArea(LoadoutHeadCoverArea);
		if (head && head.GetPrefabData())
		{
			string name = head.GetPrefabData().GetPrefabName();
			name.ToLower();
			if (name.Contains("helmet") || name.Contains("hood") || name.Contains("rain"))
				return true;
		}

		return false;
	}
}
