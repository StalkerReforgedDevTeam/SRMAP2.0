// Sickness, DayZ style: temperature and wetness from the weather and your clothes, the cold chain
// (cold, influenza, pneumonia), salmonella from bad food, a first stage of radiation sickness,
// and the medicine that treats them. Server side, ticks every 10 seconds per player.
// Values are saved through relogs and restarts and cleared on death.
// Layout follows DayZ: Stats, Environment, Agents, Modifiers, Symptoms, in the files next to this one.
// Insulation comes from slot defaults plus the item's ARMST fire protection, see SRZ_SicknessEnvironment.

// Medicine and bad food go through the same hook as the stims
modded class ARMST_Consumable
{
	override void AddConsumableDamageEffects(notnull ChimeraCharacter char, IEntity instigator)
	{
		super.AddConsumableDamageEffects(char, instigator);
		SRZ_Sickness.OnUsed(char, Temp);
	}
}

modded class ARMST_PLAYER_STATS_COMPONENT
{
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		if (GetGame().InPlayMode() && !SCR_Global.IsEditMode() && Replication.IsServer())
			SRZ_Sickness.StartTicking();
	}
}

class SRZ_Sickness
{
	static const float TICK = 10;
	protected static const float SAVE_EVERY = 30;
	protected static const string SAVE_DIR = "$profile:SRZ_Sickness";

	// Temperature bands
	static const int BAND_HYPOTHERMIA = -3;
	static const int BAND_FREEZING = -2;
	static const int BAND_COLD = -1;
	static const int BAND_COMFORT = 0;
	static const int BAND_HOT = 1;
	static const int BAND_HEATSTROKE = 2;

	// Sick icon states sent to the HUD
	static const int SICK_NONE = 0;
	static const int SICK_MILD = 1;
	static const int SICK_BAD = 2;

	static const float HEATPACK_BUFF = 600;
	static const float CODEINE_TIME = 600;
	static const float VODKA_WARMTH = 15;
	static const float BAD_FOOD_CONDITION = 50;

	protected static ref array<ref SRZ_SicknessStats> s_aStates = {};
	protected static bool s_bTicking;

	//------------------------------------------------------------------------------------------------
	static void StartTicking()
	{
		if (s_bTicking)
			return;

		s_bTicking = true;
		GetGame().GetCallqueue().CallLater(Tick, TICK * 1000, true);
		Print("[SRZ Sickness] Ticking every " + TICK.ToString() + " s");
	}

	//------------------------------------------------------------------------------------------------
	static int Band(float temperature)
	{
		if (temperature < -75) return BAND_HYPOTHERMIA;
		if (temperature < -45) return BAND_FREEZING;
		if (temperature < -15) return BAND_COLD;
		if (temperature > 75) return BAND_HEATSTROKE;
		if (temperature > 15) return BAND_HOT;
		return BAND_COMFORT;
	}

	//------------------------------------------------------------------------------------------------
	// Speed cap for the slowest wins rule in RadSickness.c. -1 means no cap.
	static float GetSpeed(IEntity character)
	{
		foreach (SRZ_SicknessStats st : s_aStates)
		{
			if (st.m_Char == character)
				return SRZ_SicknessModifiers.Speed(st, Band(st.m_fTemperature));
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	static SRZ_SicknessStats GetState(IEntity character)
	{
		if (!character)
			return null;

		foreach (SRZ_SicknessStats st : s_aStates)
		{
			if (st.m_Char == character)
				return st;
		}

		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(character);
		if (playerId <= 0)
			return null;

		SRZ_SicknessStats created = new SRZ_SicknessStats();
		created.m_Char = character;
		created.m_sUid = Uid(playerId);
		s_aStates.Insert(created);
		Load(created);
		return created;
	}

	//------------------------------------------------------------------------------------------------
	protected static string Uid(int playerId)
	{
		string uid = SCR_PlayerIdentityUtils.GetPlayerIdentityId(playerId);
		if (uid.IsEmpty())
			uid = "local_" + playerId;

		return uid;
	}

	//------------------------------------------------------------------------------------------------
	protected static void Tick()
	{
		array<int> ids = {};
		GetGame().GetPlayerManager().GetPlayers(ids);
		foreach (int id : ids)
		{
			IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(id);
			if (ent && !IsDead(ent))
				GetState(ent);
		}

		float now = GetGame().GetWorld().GetWorldTime() / 1000;

		for (int i = s_aStates.Count() - 1; i >= 0; i--)
		{
			SRZ_SicknessStats st = s_aStates[i];
			if (!st.m_Char)
			{
				s_aStates.Remove(i);
				continue;
			}

			if (IsDead(st.m_Char))
			{
				st.Reset();
				Save(st);
				s_aStates.Remove(i);
				continue;
			}

			SRZ_SicknessEnvironment.Update(st);
			int band = Band(st.m_fTemperature);
			SRZ_SicknessAgents.Update(st, band);
			SRZ_SicknessModifiers.Update(st, band);
			SRZ_SicknessSymptoms.Update(st, band, now);
			SendHud(st, band);

			if (now >= st.m_fNextSave)
			{
				st.m_fNextSave = now + SAVE_EVERY;
				Save(st);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static void SendHud(SRZ_SicknessStats st, int band)
	{
		SCR_ChimeraCharacter chimera = SCR_ChimeraCharacter.Cast(st.m_Char);
		if (!chimera)
			return;

		int wet = 0;
		if (st.m_fWetness >= 60)
			wet = 2;
		else if (st.m_fWetness >= 30)
			wet = 1;

		int sick = SICK_NONE;
		int coldStage = SRZ_SicknessAgents.ColdStage(st);
		int salmStage = SRZ_SicknessAgents.SalmonellaStage(st);
		if (coldStage >= 2 || salmStage >= 2)
			sick = SICK_BAD;
		else if (coldStage >= 1 || salmStage >= 1)
			sick = SICK_MILD;

		chimera.SRZ_SendSickHud(band, wet, sick, SRZ_SicknessModifiers.IsDraining(st, band));
	}

	//------------------------------------------------------------------------------------------------
	// Medicine by prefab name, bad food by condition or name
	static void OnUsed(IEntity character, IEntity item)
	{
		if (!Replication.IsServer() || !character || !item || !item.GetPrefabData())
			return;

		SRZ_SicknessStats st = GetState(character);
		if (!st)
			return;

		string prefab = item.GetPrefabData().GetPrefabName();
		prefab.ToLower();

		if (prefab.Contains("srz_tetracycline"))
		{
			SRZ_SicknessAgents.Tetracycline(st);
			return;
		}

		if (prefab.Contains("srz_charcoal"))
		{
			SRZ_SicknessAgents.Charcoal(st);
			return;
		}

		if (prefab.Contains("srz_multivitamin"))
		{
			SRZ_SicknessAgents.Multivitamin(st);
			return;
		}

		if (prefab.Contains("srz_codeine"))
		{
			st.m_fCodeine = CODEINE_TIME;
			return;
		}

		if (prefab.Contains("srz_heatpack"))
		{
			st.m_fHeatBuff = Math.Max(st.m_fHeatBuff, HEATPACK_BUFF);
			st.m_fTemperature = Math.Max(st.m_fTemperature, Math.Min(st.m_fTemperature + 20, 10));
			return;
		}

		if (prefab.Contains("vodka"))
		{
			st.m_fTemperature = Math.Min(100, st.m_fTemperature + VODKA_WARMTH);
			return;
		}

		// Food: raw meat and anything under 50 condition
		ARMST_ItemUseComponent use = ARMST_ItemUseComponent.Cast(item.FindComponent(ARMST_ItemUseComponent));
		if (!use || use.m_ArmstChangeEat <= 0)
			return;

		bool bad = prefab.Contains("raw") || prefab.Contains("meat") || prefab.Contains("flesh") || prefab.Contains("mutant");
		ARMST_ITEMS_STATS_COMPONENTS stats = ARMST_ITEMS_STATS_COMPONENTS.Cast(item.FindComponent(ARMST_ITEMS_STATS_COMPONENTS));
		if (stats && stats.GetConditionLevel() < BAD_FOOD_CONDITION)
			bad = true;

		if (bad)
			SRZ_SicknessAgents.Ingest(st);
	}

	//------------------------------------------------------------------------------------------------
	protected static bool IsDead(IEntity character)
	{
		ChimeraCharacter chimera = ChimeraCharacter.Cast(character);
		if (!chimera || !chimera.GetCharacterController())
			return true;

		return chimera.GetCharacterController().IsDead();
	}

	//------------------------------------------------------------------------------------------------
	protected static string SaveFile(SRZ_SicknessStats st)
	{
		return SAVE_DIR + "/" + st.m_sUid + ".txt";
	}

	//------------------------------------------------------------------------------------------------
	protected static void Save(SRZ_SicknessStats st)
	{
		if (st.IsClean())
		{
			if (FileIO.FileExists(SaveFile(st)))
				FileIO.DeleteFile(SaveFile(st));

			return;
		}

		FileIO.MakeDirectory(SAVE_DIR);
		FileHandle file = FileIO.OpenFile(SaveFile(st), FileMode.WRITE);
		if (!file)
			return;

		file.WriteLine(st.Serialize());
		file.Close();
	}

	//------------------------------------------------------------------------------------------------
	protected static void Load(SRZ_SicknessStats st)
	{
		if (!FileIO.FileExists(SaveFile(st)))
			return;

		FileHandle file = FileIO.OpenFile(SaveFile(st), FileMode.READ);
		if (!file)
			return;

		string line;
		file.ReadLine(line);
		file.Close();

		if (st.Deserialize(line))
		{
			string info = "[SRZ Sickness] Restored " + st.m_sUid;
			info += " temp " + st.m_fTemperature.ToString();
			info += " cold " + st.m_fColdChain.ToString();
			info += " salm " + st.m_fSalmonella.ToString();
			Print(info);
		}
	}
}
