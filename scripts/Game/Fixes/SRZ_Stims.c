// Tarkov style stims: effects run over time, then side effects kick in after the main effect ends.
modded class SCR_ChimeraCharacter
{
	void SRZ_SendStimHud(string name, float main, float side)
	{
		Rpc(RpcDo_SRZStimHud, name, main, side);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RpcDo_SRZStimHud(string name, float main, float side)
	{
		SRZ_StimHud.Add(name, main, side);
	}
}

class SRZ_StimHudEntry
{
	string m_sName;
	float m_fMainEnd;
	float m_fSideEnd;
}

// Client side list of active stims, only shown while the inventory is open
class SRZ_StimHud
{
	protected static const ResourceName LAYOUT = "{6C1F2A9B3E7D4058}UI/layouts/HUD/SRZ_StimHud.layout";
	protected static const int LINES = 6;

	protected static ref array<ref SRZ_StimHudEntry> s_aEntries = {};
	protected static Widget s_wRoot;
	protected static ref array<RichTextWidget> s_aLines = {};
	protected static bool s_bTicking;

	//------------------------------------------------------------------------------------------------
	static void Add(string name, float main, float side)
	{
		float now = GetGame().GetWorld().GetWorldTime() / 1000;
		SRZ_StimHudEntry entry;
		foreach (SRZ_StimHudEntry e : s_aEntries)
		{
			if (e.m_sName == name)
			{
				entry = e;
				break;
			}
		}

		if (!entry)
		{
			entry = new SRZ_StimHudEntry();
			entry.m_sName = name;
			s_aEntries.Insert(entry);
		}

		entry.m_fMainEnd = now + main;
		entry.m_fSideEnd = now + main + side;

		Create();
		Update();
		if (!s_bTicking)
		{
			s_bTicking = true;
			GetGame().GetCallqueue().CallLater(Update, 250, true);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static void Create()
	{
		if (s_wRoot)
			return;

		s_wRoot = GetGame().GetWorkspace().CreateWidgets(LAYOUT);
		if (!s_wRoot)
		{
			Print("[SRZ Stims] Could not create HUD layout", LogLevel.WARNING);
			return;
		}

		s_wRoot.SetZOrder(10000);
		s_wRoot.SetVisible(false);
		s_aLines.Clear();
		for (int i = 0; i < LINES; i++)
		{
			s_aLines.Insert(RichTextWidget.Cast(s_wRoot.FindAnyWidget("SRZ_Stim" + i)));
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static void Update()
	{
		IEntity local = SCR_PlayerController.GetLocalControlledEntity();
		ChimeraCharacter chimera = ChimeraCharacter.Cast(local);
		bool alive = chimera && chimera.GetCharacterController() && !chimera.GetCharacterController().IsDead();
		if (!alive)
			s_aEntries.Clear();

		float now = GetGame().GetWorld().GetWorldTime() / 1000;
		for (int i = s_aEntries.Count() - 1; i >= 0; i--)
		{
			if (now >= s_aEntries[i].m_fSideEnd)
				s_aEntries.RemoveOrdered(i);
		}

		if (s_aEntries.IsEmpty())
		{
			if (s_wRoot)
				s_wRoot.RemoveFromHierarchy();

			s_wRoot = null;
			s_aLines.Clear();
			GetGame().GetCallqueue().Remove(Update);
			s_bTicking = false;
			return;
		}

		bool inventoryOpen = GetGame().GetMenuManager().FindMenuByPreset(ChimeraMenuPreset.Inventory20Menu) != null;
		if (s_wRoot)
			s_wRoot.SetVisible(inventoryOpen);

		if (!inventoryOpen)
			return;

		for (int line = 0; line < s_aLines.Count(); line++)
		{
			RichTextWidget w = s_aLines[line];
			if (!w)
				continue;

			if (line >= s_aEntries.Count())
			{
				w.SetVisible(false);
				continue;
			}

			SRZ_StimHudEntry entry = s_aEntries[line];
			string text;
			if (now < entry.m_fMainEnd)
				text = "<color rgba='120,200,90,255'>+</color>  " + entry.m_sName + "  <color rgba='170,170,170,255'>" + FormatTime(entry.m_fMainEnd - now) + "</color>";
			else
				text = "<color rgba='220,140,50,255'>!</color>  " + entry.m_sName + " wearing off  <color rgba='170,170,170,255'>" + FormatTime(entry.m_fSideEnd - now) + "</color>";

			w.SetText(text);
			w.SetVisible(true);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static string FormatTime(float seconds)
	{
		int s = (int)Math.Max(0, Math.Ceil(seconds));
		int m = s / 60;
		s = s % 60;
		string sec = s.ToString();
		if (s < 10)
			sec = "0" + sec;

		return m.ToString() + ":" + sec;
	}
}

class SRZ_StimPhase
{
	float m_fDuration;
	float m_fHeal;
	float m_fStamina;
	float m_fToxic;
	float m_fRad;
	float m_fPsy;
	float m_fWater;
	float m_fEat;
	bool m_bStopBleed;
	bool m_bTremor;
}

class SRZ_StimProfile
{
	string m_sKey;
	string m_sName;
	ref SRZ_StimPhase m_Main;
	ref SRZ_StimPhase m_Side;
}

class SRZ_StimEffect
{
	IEntity m_Char;
	ref SRZ_StimProfile m_Profile;
	float m_fTime;
}

class SRZ_Stims
{
	protected static const float TICK = 2;

	protected static ref array<ref SRZ_StimProfile> s_aProfiles;
	protected static ref array<ref SRZ_StimEffect> s_aActive = {};
	protected static bool s_bTicking;

	//------------------------------------------------------------------------------------------------
	static SRZ_StimPhase Phase(float duration, float heal, float stamina, float toxic, float rad, float psy, float water, float eat, bool bleed = false, bool tremor = false)
	{
		SRZ_StimPhase p = new SRZ_StimPhase();
		p.m_fDuration = duration;
		p.m_fHeal = heal;
		p.m_fStamina = stamina;
		p.m_fToxic = toxic;
		p.m_fRad = rad;
		p.m_fPsy = psy;
		p.m_fWater = water;
		p.m_fEat = eat;
		p.m_bStopBleed = bleed;
		p.m_bTremor = tremor;
		return p;
	}

	//------------------------------------------------------------------------------------------------
	protected static void Add(string name, string key, SRZ_StimPhase main, SRZ_StimPhase side = null)
	{
		SRZ_StimProfile prof = new SRZ_StimProfile();
		prof.m_sName = name;
		prof.m_sKey = key;
		prof.m_Main = main;
		prof.m_Side = side;
		s_aProfiles.Insert(prof);
	}

	//------------------------------------------------------------------------------------------------
	// Values are per 2 second tick. Heal is HP, stamina is 0 to 1, psy/water/eat positive restores, toxic/rad negative cures
	protected static void Build()
	{
		s_aProfiles = {};

		// Propital: long steady regen, shakes and thirst after
		Add("Propital", "armst_itm_stimulator_137", Phase(300, 3, 0, 0, 0, 0, 0, 0), Phase(60, 0, 0, 0, 0, -0.5, -1, 0, false, true));
		// eTG-c: very fast regen, hard hunger and thirst crash
		Add("eTG-c", "armst_itm_stimulator_941", Phase(60, 12, 0, 0, 0, 0, 0, 0), Phase(120, 0, 0, 0, 0, 0, -1, -1));
		// SJ6 TGLabs: stamina stays topped up, nerves and shakes after
		Add("SJ6 TGLabs", "armst_itm_stimulator_572", Phase(240, 0, 0.25, 0, 0, 0, -0.2, 0), Phase(60, 0, 0, 0, 0, -1, -0.6, 0, false, true));
		// Adrenaline: short burst of stamina and healing
		Add("Adrenaline", "armst_itm_stimulator_213", Phase(60, 2, 0.4, 0, 0, 0, 0, 0), Phase(30, 0, 0, 0, 0, 0, -1.5, -0.8));
		// Zagustin: stops bleeding for three minutes
		Add("Zagustin", "armst_itm_stimulator_993", Phase(180, 0, 0, 0, 0, 0, -0.3, 0, true));
		// Perfotoran: flushes toxins and radiation
		Add("Perfotoran", "armst_itm_stimulator_463", Phase(120, 0, 0, -3, -1.5, 0, 0, 0), Phase(60, 0, 0, 0, 0, 0, -0.8, -0.8));
		// SJ9 TGLabs: psy shield, body pays for it after
		Add("SJ9 TGLabs", "armst_itm_stimulator_816", Phase(180, 0, 0, 0, 0, 3, 0, 0), Phase(120, -0.8, 0, 0.5, 0, 0, 0, 0, false, true));
		// Antirad and Antitoxin work over time too
		Add("Antirad", "armst_itm_stim_derad", Phase(90, 0, 0, 0, -3, 0, -0.3, 0));
		Add("Antitoxin", "armst_itm_stim_detox", Phase(90, 0, 0, -3, 0, 0, -0.2, 0));
		// Obdolbos 2: everything at once, then radiation and poison
		Add("Obdolbos 2", "armst_itm_stimulator_secret_2", Phase(300, 3, 0.25, 0, 0, 2, 0, 0, true), Phase(120, 0, 0, 0.8, 1, 0, -1, -1, false, true));
	}

	//------------------------------------------------------------------------------------------------
	// Obdolbos: every effect is a coin flip
	protected static SRZ_StimProfile BuildObdolbos()
	{
		SRZ_StimProfile prof = new SRZ_StimProfile();
		prof.m_sKey = "armst_itm_stimulator_secret_1";
		prof.m_sName = "Obdolbos";
		prof.m_Main = Phase(300, 0, 0, 0, 0, 0, 0, 0);
		prof.m_Side = Phase(120, 0, 0, 0, 0, 0, 0, 0);

		if (Math.RandomFloat01() < 0.5) prof.m_Main.m_fHeal = 3;
		if (Math.RandomFloat01() < 0.5) prof.m_Main.m_fStamina = 0.25;
		if (Math.RandomFloat01() < 0.5) prof.m_Main.m_fPsy = 2;
		if (Math.RandomFloat01() < 0.4) prof.m_Main.m_bStopBleed = true;
		if (Math.RandomFloat01() < 0.4) prof.m_Main.m_fRad = -1.5;

		if (Math.RandomFloat01() < 0.25) prof.m_Side.m_fPsy = -1.5;
		if (Math.RandomFloat01() < 0.25) prof.m_Side.m_fToxic = 1;
		if (Math.RandomFloat01() < 0.25) prof.m_Side.m_fRad = 1;
		if (Math.RandomFloat01() < 0.25) prof.m_Side.m_fHeal = -1;
		if (Math.RandomFloat01() < 0.4) prof.m_Side.m_bTremor = true;
		prof.m_Side.m_fWater = -0.8;
		return prof;
	}

	//------------------------------------------------------------------------------------------------
	static void OnUsed(IEntity character, IEntity item)
	{
		if (!Replication.IsServer() || !character || !item || !item.GetPrefabData())
			return;

		if (!s_aProfiles)
			Build();

		string prefab = item.GetPrefabData().GetPrefabName();
		prefab.ToLower();

		SRZ_StimProfile prof;
		if (prefab.Contains("armst_itm_stimulator_secret_1"))
		{
			prof = BuildObdolbos();
		}
		else
		{
			foreach (SRZ_StimProfile p : s_aProfiles)
			{
				if (prefab.Contains(p.m_sKey))
				{
					prof = p;
					break;
				}
			}
		}

		if (!prof)
			return;

		SRZ_SendHud(character, prof);

		foreach (SRZ_StimEffect active : s_aActive)
		{
			if (active.m_Char == character && active.m_Profile.m_sKey == prof.m_sKey)
			{
				active.m_Profile = prof;
				active.m_fTime = 0;
				return;
			}
		}

		SRZ_StimEffect effect = new SRZ_StimEffect();
		effect.m_Char = character;
		effect.m_Profile = prof;
		s_aActive.Insert(effect);

		if (!s_bTicking)
		{
			s_bTicking = true;
			GetGame().GetCallqueue().CallLater(Tick, TICK * 1000, true);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static void SRZ_SendHud(IEntity character, SRZ_StimProfile prof)
	{
		float side;
		if (prof.m_Side)
			side = prof.m_Side.m_fDuration;

		if (character == SCR_PlayerController.GetLocalControlledEntity())
			SRZ_StimHud.Add(prof.m_sName, prof.m_Main.m_fDuration, side);

		SCR_ChimeraCharacter chimera = SCR_ChimeraCharacter.Cast(character);
		if (chimera)
			chimera.SRZ_SendStimHud(prof.m_sName, prof.m_Main.m_fDuration, side);
	}

	//------------------------------------------------------------------------------------------------
	protected static void Tick()
	{
		for (int i = s_aActive.Count() - 1; i >= 0; i--)
		{
			SRZ_StimEffect effect = s_aActive[i];
			if (!effect.m_Char || IsDead(effect.m_Char))
			{
				s_aActive.Remove(i);
				continue;
			}

			effect.m_fTime += TICK;
			SRZ_StimProfile prof = effect.m_Profile;
			float mainEnd = prof.m_Main.m_fDuration;
			float sideEnd = mainEnd;
			if (prof.m_Side)
				sideEnd += prof.m_Side.m_fDuration;

			if (effect.m_fTime <= mainEnd)
				ApplyPhase(effect.m_Char, prof.m_Main);
			else if (effect.m_fTime <= sideEnd)
				ApplyPhase(effect.m_Char, prof.m_Side);
			else
				s_aActive.Remove(i);
		}

		if (s_aActive.IsEmpty())
		{
			GetGame().GetCallqueue().Remove(Tick);
			s_bTicking = false;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static bool IsDead(IEntity character)
	{
		ChimeraCharacter chimera = ChimeraCharacter.Cast(character);
		if (!chimera)
			return true;

		CharacterControllerComponent controller = chimera.GetCharacterController();
		return !controller || controller.IsDead();
	}

	//------------------------------------------------------------------------------------------------
	protected static void ApplyPhase(IEntity character, SRZ_StimPhase p)
	{
		ARMST_PLAYER_STATS_COMPONENT stats = ARMST_PLAYER_STATS_COMPONENT.Cast(character.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
		if (stats)
		{
			if (p.m_fToxic != 0)
				stats.ArmstPlayerStatSetToxic(p.m_fToxic);

			if (p.m_fRad != 0)
				stats.ArmstPlayerStatSetRadio(p.m_fRad);

			if (p.m_fPsy != 0)
				stats.ArmstPlayerStatSetPsy(p.m_fPsy);

			if (p.m_fWater != 0)
				stats.ArmstPlayerStatSetWater(p.m_fWater);

			if (p.m_fEat != 0)
				stats.ArmstPlayerStatSetEat(p.m_fEat);
		}

		SCR_CharacterDamageManagerComponent dmg = SCR_CharacterDamageManagerComponent.Cast(character.FindComponent(SCR_CharacterDamageManagerComponent));
		if (dmg)
		{
			if (p.m_fHeal > 0)
			{
				dmg.HealHitZones(p.m_fHeal);
			}
			else if (p.m_fHeal < 0)
			{
				BaseDamageContext ctx = new BaseDamageContext();
				ctx.damageValue = -p.m_fHeal;
				ctx.damageType = EDamageType.INCENDIARY;
				dmg.HandleDamage(ctx);
			}

			if (p.m_bStopBleed && dmg.IsBleeding())
				dmg.RemoveAllBleedings();
		}

		if (p.m_fStamina != 0)
		{
			CharacterStaminaComponent stamina = CharacterStaminaComponent.Cast(character.FindComponent(CharacterStaminaComponent));
			if (stamina)
				stamina.AddStamina(p.m_fStamina);
		}

		if (p.m_bTremor)
		{
			SCR_ChimeraCharacter chimera = SCR_ChimeraCharacter.Cast(character);
			if (chimera)
				chimera.ArmstCameraShake(0.6, 1, 0.2, 1.4, 0.4);
		}
	}
}

modded class ARMST_Consumable
{
	override void AddConsumableDamageEffects(notnull ChimeraCharacter char, IEntity instigator)
	{
		super.AddConsumableDamageEffects(char, instigator);
		SRZ_Stims.OnUsed(char, Temp);
	}
}

