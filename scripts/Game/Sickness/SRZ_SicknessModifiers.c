// The effects, DayZ's modifiers equivalent. Server side, once per tick per player.
// Each block checks its own condition and applies its drain. Speed goes through the slowest wins
// rule in RadSickness.c via SRZ_Sickness.GetSpeed.
class SRZ_SicknessModifiers
{
	static const float TICK = 10;	// same as SRZ_Sickness.TICK

	// Temperature
	static const float HYPO_HEALTH = 0.333;		// per tick, 1 health every 30 seconds
	static const float HEAT_HEALTH = 0.333;
	static const float COLD_STAMINA = 0.02;
	static const float FREEZING_STAMINA = 0.05;
	static const float COLD_FOOD = 0.3;						// extra food per tick while cold
	static const float HOT_WATER = 0.3;
	static const float HYPO_SPEED = 0.8;

	// Cold chain
	static const float FLU_WATER = 0.2;
	static const float PNEUMONIA_HEALTH = 0.25;		// per tick, 1.5 health a minute
	static const float PNEUMONIA_HEALTH_WARM = 0.05;
	static const float PNEUMONIA_STAMINA = 0.05;

	// Radiation stage 1 (stage 2 stays what RadSickness.c does today)
	static const float RAD_STAGE1 = 60;
	static const float RAD_STAMINA = 0.03;

	//------------------------------------------------------------------------------------------------
	static void Update(SRZ_SicknessStats st, int tempBand)
	{
		IEntity ent = st.m_Char;
		ARMST_PLAYER_STATS_COMPONENT stats = ARMST_PLAYER_STATS_COMPONENT.Cast(ent.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
		if (!stats)
			return;

		int coldStage = SRZ_SicknessAgents.ColdStage(st);
		bool warm = tempBand >= SRZ_Sickness.BAND_COMFORT;

		// Temperature bands
		if (tempBand == SRZ_Sickness.BAND_COLD)
		{
			Stamina(ent, -COLD_STAMINA);
			stats.ArmstPlayerStatSetEat(-COLD_FOOD);
		}
		else if (tempBand <= SRZ_Sickness.BAND_FREEZING)
		{
			Stamina(ent, -FREEZING_STAMINA);
			stats.ArmstPlayerStatSetEat(-COLD_FOOD);
			if (tempBand == SRZ_Sickness.BAND_HYPOTHERMIA)
				Health(ent, -HYPO_HEALTH);
		}
		else if (tempBand == SRZ_Sickness.BAND_HOT)
		{
			stats.ArmstPlayerStatSetWater(-HOT_WATER);
		}
		else if (tempBand == SRZ_Sickness.BAND_HEATSTROKE)
		{
			stats.ArmstPlayerStatSetWater(-HOT_WATER);
			Health(ent, -HEAT_HEALTH);
		}

		// Cold chain
		if (coldStage == 2)
		{
			stats.ArmstPlayerStatSetWater(-FLU_WATER);
		}
		else if (coldStage == 3)
		{
			stats.ArmstPlayerStatSetWater(-FLU_WATER);
			Stamina(ent, -PNEUMONIA_STAMINA);
			if (warm)
				Health(ent, -PNEUMONIA_HEALTH_WARM);
			else
				Health(ent, -PNEUMONIA_HEALTH);
		}

		// Radiation stage 1
		if (IsRadStage1(stats))
			Stamina(ent, -RAD_STAMINA);
	}

	//------------------------------------------------------------------------------------------------
	static bool IsRadStage1(ARMST_PLAYER_STATS_COMPONENT stats)
	{
		return stats && stats.ArmstPlayerStatGetRadio() >= RAD_STAGE1 && !stats.IsRadiationSick();
	}

	//------------------------------------------------------------------------------------------------
	// Whether health is draining right now, for the flashing HUD icon
	static bool IsDraining(SRZ_SicknessStats st, int tempBand)
	{
		if (tempBand == SRZ_Sickness.BAND_HYPOTHERMIA || tempBand == SRZ_Sickness.BAND_HEATSTROKE)
			return true;

		return SRZ_SicknessAgents.ColdStage(st) == 3;
	}

	//------------------------------------------------------------------------------------------------
	// Speed cap for the slowest wins rule. -1 means no cap.
	static float Speed(SRZ_SicknessStats st, int tempBand)
	{
		if (tempBand == SRZ_Sickness.BAND_HYPOTHERMIA)
			return HYPO_SPEED;

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	protected static void Health(IEntity ent, float delta)
	{
		DamageManagerComponent dmg = DamageManagerComponent.Cast(ent.FindComponent(DamageManagerComponent));
		if (!dmg)
			return;

		float max = dmg.GetMaxHealth();
		if (max <= 0)
			return;

		// Same path radiation sickness uses
		float health = Math.Clamp(dmg.GetHealth() + delta, 0, max);
		dmg.SetHealthScaled(health / max);
	}

	//------------------------------------------------------------------------------------------------
	protected static void Stamina(IEntity ent, float delta)
	{
		CharacterStaminaComponent stamina = CharacterStaminaComponent.Cast(ent.FindComponent(CharacterStaminaComponent));
		if (stamina)
			stamina.AddStamina(delta);
	}
}
