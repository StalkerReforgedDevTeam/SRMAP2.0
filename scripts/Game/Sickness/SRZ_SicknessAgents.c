// The disease counters, DayZ's agent pool equivalent. Server side, once per tick per player.
// Each counter rises while its cause is there, falls when it isn't or when immunity beats its potency,
// and medicine pushes it down over time.
class SRZ_SicknessAgents
{
	static const float TICK = 10;	// same as SRZ_Sickness.TICK

	// Cold chain, 0 to 300
	static const float COLD_STAGE = 100;
	static const float FLU_STAGE = 200;
	static const float PNEUMONIA_STAGE = 300;
	static const float COLD_POTENCY = 65;
	static const float COLD_RISE = 1.1;			// per tick in the cold band, 100 in about 15 minutes
	static const float COLD_RISE_FREEZING = 1.7;
	static const float COLD_DECAY_WARM = 1.7;		// per tick when comfortable, 100 in about 10 minutes
	static const float COLD_DECAY_IMMUNE = 0.6;		// extra per tick when immunity beats the potency
	static const float RESIST_STAGE = 600;			// seconds it can't rise after a stage drop
	static const float RESIST_CURED = 900;
	static const float TETRA_TIME = 300;
	static const float TETRA_TOTAL = 100;			// one box drops one full stage over its 5 minutes

	// Salmonella, 0 to 100
	static const float SALM_POTENCY = 85;
	static const float SALM_INCUBATION = 150;		// seconds from eating to onset
	static const float SALM_ONSET = 60;				// counter value at onset
	static const float SALM_RISE = 0.6;				// per tick to the peak, about 11 minutes after onset
	static const float SALM_DECAY = 0.3;			// per tick, about an hour from peak
	static const float SALM_DECAY_IMMUNE = 0.6;
	static const float SALM_VOMIT_RELIEF = 3;
	static const float CHARCOAL_TIME = 300;
	static const float CHARCOAL_TOTAL = 60;
	static const float VITAMIN_TIME = 300;
	static const float VITAMIN_TOTAL = 20;			// off both counters over its 5 minutes

	//------------------------------------------------------------------------------------------------
	// DayZ immunity: the average of food, water and health, 0 to 100
	static float Immunity(SRZ_SicknessStats st)
	{
		if (st.m_fImmunityBoost > 0)
			return 100;

		ARMST_PLAYER_STATS_COMPONENT stats = ARMST_PLAYER_STATS_COMPONENT.Cast(st.m_Char.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
		if (!stats)
			return 50;

		float health = 100;
		DamageManagerComponent dmg = DamageManagerComponent.Cast(st.m_Char.FindComponent(DamageManagerComponent));
		if (dmg && dmg.GetMaxHealth() > 0)
			health = dmg.GetHealth() / dmg.GetMaxHealth() * 100;

		return (stats.ArmstPlayerStatGetEat() + stats.ArmstPlayerStatGetWater() + health) / 3;
	}

	//------------------------------------------------------------------------------------------------
	static int ColdStage(SRZ_SicknessStats st)
	{
		if (st.m_fColdChain >= PNEUMONIA_STAGE) return 3;
		if (st.m_fColdChain >= FLU_STAGE) return 2;
		if (st.m_fColdChain >= COLD_STAGE) return 1;
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	static int SalmonellaStage(SRZ_SicknessStats st)
	{
		if (st.m_fSalmonella >= 90) return 2;
		if (st.m_fSalmonella >= 30) return 1;
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	static void Update(SRZ_SicknessStats st, int tempBand)
	{
		float immunity = Immunity(st);

		// Timers
		st.m_fResistance = Math.Max(0, st.m_fResistance - TICK);
		st.m_fImmunityBoost = Math.Max(0, st.m_fImmunityBoost - TICK);
		st.m_fCodeine = Math.Max(0, st.m_fCodeine - TICK);

		UpdateColdChain(st, tempBand, immunity);
		UpdateSalmonella(st, immunity);
	}

	//------------------------------------------------------------------------------------------------
	protected static void UpdateColdChain(SRZ_SicknessStats st, int tempBand, float immunity)
	{
		int stageBefore = ColdStage(st);
		bool cold = tempBand <= SRZ_Sickness.BAND_COLD;
		bool freezing = tempBand <= SRZ_Sickness.BAND_FREEZING;

		if (cold && st.m_fResistance <= 0 && st.m_fColdChain < PNEUMONIA_STAGE)
		{
			float rise = COLD_RISE;
			if (freezing)
				rise = COLD_RISE_FREEZING;

			st.m_fColdChain = Math.Min(PNEUMONIA_STAGE, st.m_fColdChain + rise);
		}
		else if (!cold)
		{
			// Warmth clears a cold and the flu by itself. Pneumonia only drops with tetracycline.
			if (stageBefore < 3)
				st.m_fColdChain = Math.Max(0, st.m_fColdChain - COLD_DECAY_WARM);
		}

		if (immunity > COLD_POTENCY && stageBefore < 3)
			st.m_fColdChain = Math.Max(0, st.m_fColdChain - COLD_DECAY_IMMUNE);

		if (st.m_fTetraLeft > 0)
		{
			st.m_fTetraLeft = Math.Max(0, st.m_fTetraLeft - TICK);
			st.m_fColdChain = Math.Max(0, st.m_fColdChain - TETRA_TOTAL / TETRA_TIME * TICK);
		}

		// Dropping a stage earns a resistance window where it can't climb back
		int stageAfter = ColdStage(st);
		if (stageAfter < stageBefore)
		{
			// Sit just under the stage line so the next tick doesn't flip straight back
			float line = COLD_STAGE * stageBefore;
			st.m_fColdChain = Math.Min(st.m_fColdChain, line - 20);
			if (stageAfter == 0)
				st.m_fResistance = RESIST_CURED;
			else
				st.m_fResistance = RESIST_STAGE;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static void UpdateSalmonella(SRZ_SicknessStats st, float immunity)
	{
		if (st.m_fIncubation > 0)
		{
			st.m_fIncubation -= TICK;
			if (st.m_fIncubation <= 0)
			{
				st.m_fIncubation = 0;
				st.m_fSalmonella = Math.Max(st.m_fSalmonella, SALM_ONSET);
			}
		}

		if (st.m_fSalmonella > 0)
		{
			if (immunity > SALM_POTENCY)
				st.m_fSalmonella -= SALM_DECAY_IMMUNE;
			else if (st.m_fSalmonella < 100 && st.m_fSalmonella >= SALM_ONSET - 1 && st.m_fCharcoalLeft <= 0)
				st.m_fSalmonella += SALM_RISE;
			else
				st.m_fSalmonella -= SALM_DECAY;
		}

		if (st.m_fCharcoalLeft > 0)
		{
			st.m_fCharcoalLeft = Math.Max(0, st.m_fCharcoalLeft - TICK);
			st.m_fSalmonella -= CHARCOAL_TOTAL / CHARCOAL_TIME * TICK;
		}

		st.m_fSalmonella = Math.Clamp(st.m_fSalmonella, 0, 100);
	}

	//------------------------------------------------------------------------------------------------
	// Eating something bad. Immunity above the potency shrugs it off (DayZ).
	static void Ingest(SRZ_SicknessStats st)
	{
		if (Immunity(st) >= SALM_POTENCY)
			return;

		if (st.m_fSalmonella >= SALM_ONSET)
		{
			st.m_fSalmonella = Math.Min(100, st.m_fSalmonella + 20);
			return;
		}

		if (st.m_fIncubation <= 0)
			st.m_fIncubation = SALM_INCUBATION;
	}

	//------------------------------------------------------------------------------------------------
	static void Vomited(SRZ_SicknessStats st)
	{
		st.m_fSalmonella = Math.Max(0, st.m_fSalmonella - SALM_VOMIT_RELIEF);
	}

	//------------------------------------------------------------------------------------------------
	static void Tetracycline(SRZ_SicknessStats st)
	{
		st.m_fTetraLeft = TETRA_TIME;
	}

	//------------------------------------------------------------------------------------------------
	static void Charcoal(SRZ_SicknessStats st)
	{
		st.m_fCharcoalLeft = CHARCOAL_TIME;
	}

	//------------------------------------------------------------------------------------------------
	static void Multivitamin(SRZ_SicknessStats st)
	{
		st.m_fImmunityBoost = VITAMIN_TIME;
		st.m_fColdChain = Math.Max(0, st.m_fColdChain - VITAMIN_TOTAL);
		st.m_fSalmonella = Math.Max(0, st.m_fSalmonella - VITAMIN_TOTAL);
	}
}
