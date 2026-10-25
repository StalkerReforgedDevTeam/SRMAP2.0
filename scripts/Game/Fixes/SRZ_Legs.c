// Broken legs, DayZ style: falls of about 13 feet or more and some mutant hits can break a leg. A broken leg slows you to a limp
// and heals on its own after 10 minutes. Walking or running upright on it knocks you out; crouch or crawl instead.
// A splint speeds you up and heals it in 2. A bone icon shows on the HUD.
// Saved through relogs and restarts.
modded class SCR_ChimeraCharacter
{
	//------------------------------------------------------------------------------------------------
	void SRZ_SendLegState(int state)
	{
		Rpc(RpcDo_SRZLegState, state);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RpcDo_SRZLegState(int state)
	{
		SRZ_Legs.OnLocalState(this, state);
	}

	//------------------------------------------------------------------------------------------------
	void SRZ_SendLegSfx(bool crack)
	{
		Rpc(RpcDo_SRZLegSfx, crack);
		RpcDo_SRZLegSfx(crack);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_SRZLegSfx(bool crack)
	{
		SRZ_Legs.PlaySfx(this, crack);
	}
}

// The bone icon now lives in the status HUD
class SRZ_LegHud
{
	//------------------------------------------------------------------------------------------------
	static void Show(int state)
	{
		SRZ_StatusHud.SetLegState(state);
	}
}

class SRZ_LegState
{
	IEntity m_Char;
	string m_sUid;
	int m_iState;
	float m_fHealLeft;
	float m_fNextPain;
	float m_fNextSave;
	float m_fStrain;
	float m_fNoKnockoutUntil;
	vector m_vLastPos;
}

class SRZ_Legs
{
	static const int HEALTHY = 0;
	static const int BROKEN = 1;
	static const int SPLINTED = 2;
	protected static const ResourceName CRACK_SOUND = "{D86737DA5990F274}Sounds/SRZ/Legs/SRZ_BoneCrack.acp";

	protected static const float BROKEN_SPEED = 0.4;
	protected static const float SPLINTED_SPEED = 0.75;
	protected static const float SELF_HEAL_TIME = 600;
	protected static const float SPLINT_HEAL_TIME = 120;
	protected static const float TICK = 1;
	protected static const float WALK_SPEED = 0.5;
	protected static const float RUN_SPEED = 1.8;
	protected static const float STRAIN_KNOCKOUT = 6;
	protected static const float KNOCKOUT_COOLDOWN = 20;
	protected static const float FALL_MIN = 4;
	protected static const string SPLINT_PREFAB = "srz_splint";
	protected static const string SAVE_DIR = "$profile:SRZ_Legs";

	protected static ref array<ref SRZ_LegState> s_aStates = {};
	protected static bool s_bTicking;

	//------------------------------------------------------------------------------------------------
	// Called on landing with how far the player dropped
	static void OnLanded(IEntity character, float drop)
	{
		if (drop < FALL_MIN)
			return;

		float chance = Math.Clamp(0.25 + (drop - FALL_MIN) * 0.15, 0, 1);
		Print("[SRZ Legs] Fell " + drop + "m, break chance " + chance);
		if (Math.RandomFloat01() < chance)
			Break(character);
	}

	//------------------------------------------------------------------------------------------------
	static void OnMutantHit(IEntity mutant, IEntity target)
	{
		if (!Replication.IsServer() || !mutant || !target || !EntityUtils.IsPlayer(target))
			return;

		float chance = 0.02;
		if (SRZ_MutantTuning.IsPrefab(mutant, "/boar/"))
			chance = 0.1;
		else if (SRZ_MutantTuning.IsPrefab(mutant, "nighstalker"))
			chance = 0.06;
		else if (SRZ_MutantTuning.IsPrefab(mutant, "/wolf/"))
			chance = 0.04;

		if (Math.RandomFloat01() < chance)
			Break(target);
	}

	//------------------------------------------------------------------------------------------------
	static void OnItemUsed(IEntity character, IEntity item)
	{
		if (!Replication.IsServer() || !character || !item || !item.GetPrefabData())
			return;

		string prefab = item.GetPrefabData().GetPrefabName();
		prefab.ToLower();
		if (!prefab.Contains(SPLINT_PREFAB))
			return;

		SRZ_LegState st = GetState(character);
		if (!st)
			return;

		if (st.m_iState != BROKEN)
			return;

		st.m_iState = SPLINTED;
		st.m_fHealLeft = Math.Min(st.m_fHealLeft, SPLINT_HEAL_TIME);
		Apply(st);
	}

	//------------------------------------------------------------------------------------------------
	static void Break(IEntity character)
	{
		SRZ_LegState st = GetState(character);
		if (!st || st.m_iState == BROKEN)
			return;

		st.m_iState = BROKEN;
		st.m_fHealLeft = SELF_HEAL_TIME;
		Print("[SRZ Legs] Leg broken");
		Apply(st);

		SCR_ChimeraCharacter chimera = SCR_ChimeraCharacter.Cast(character);
		if (chimera)
			chimera.SRZ_SendLegSfx(true);
	}

	//------------------------------------------------------------------------------------------------
	// Runs on every client near the player: bone crack plus the character's own pain grunt
	static void PlaySfx(IEntity character, bool crack)
	{
		if (System.IsConsoleApp() || RplSession.Mode() == RplMode.Dedicated || !character)
			return;

		if (crack)
		{
			vector mat[4];
			character.GetWorldTransform(mat);
			AudioSystem.PlayEvent(CRACK_SOUND, "SOUND_SRZ_BONECRACK", mat, new array<string>(), new array<float>());
		}

		SCR_CommunicationSoundComponent voice = SCR_CommunicationSoundComponent.Cast(character.FindComponent(SCR_CommunicationSoundComponent));
		if (voice)
			voice.SoundEventHit(false, EDamageType.MELEE);
	}

	//------------------------------------------------------------------------------------------------
	// Speed cap for other scripts that also slow players. -1 means no cap.
	static float GetSpeed(IEntity character)
	{
		foreach (SRZ_LegState st : s_aStates)
		{
			if (st.m_Char != character)
				continue;

			if (st.m_iState == BROKEN)
				return BROKEN_SPEED;

			if (st.m_iState == SPLINTED)
				return SPLINTED_SPEED;
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	protected static float StateSpeed(int state)
	{
		if (state == BROKEN)
			return BROKEN_SPEED;

		if (state == SPLINTED)
			return SPLINTED_SPEED;

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	protected static SRZ_LegState GetState(IEntity character)
	{
		if (!character)
			return null;

		foreach (SRZ_LegState st : s_aStates)
		{
			if (st.m_Char == character)
				return st;
		}

		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(character);
		if (playerId <= 0)
			return null;

		SRZ_LegState created = new SRZ_LegState();
		created.m_Char = character;
		created.m_sUid = Uid(playerId);
		s_aStates.Insert(created);
		Load(created);
		StartTicking();
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
	protected static void StartTicking()
	{
		if (s_bTicking)
			return;

		s_bTicking = true;
		GetGame().GetCallqueue().CallLater(Tick, TICK * 1000, true);
	}

	//------------------------------------------------------------------------------------------------
	protected static void Tick()
	{
		// Pick up players who logged in with a saved broken leg
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
			SRZ_LegState st = s_aStates[i];
			if (!st.m_Char)
			{
				s_aStates.Remove(i);
				continue;
			}

			if (IsDead(st.m_Char))
			{
				if (st.m_iState != HEALTHY)
				{
					st.m_iState = HEALTHY;
					Apply(st);
				}

				s_aStates.Remove(i);
				continue;
			}

			if (st.m_iState != HEALTHY)
			{
				st.m_fHealLeft -= TICK;
				if (st.m_fHealLeft <= 0)
				{
					st.m_iState = HEALTHY;
					Apply(st);
					continue;
				}

				if (now >= st.m_fNextSave)
				{
					st.m_fNextSave = now + 30;
					Save(st);
				}
			}

			if (st.m_iState == BROKEN)
				UpdateStrain(st, now);
		}
	}

	//------------------------------------------------------------------------------------------------
	// Walking or running upright on a broken leg builds up pain until you pass out
	protected static void UpdateStrain(SRZ_LegState st, float now)
	{
		ChimeraCharacter chimera = ChimeraCharacter.Cast(st.m_Char);
		if (!chimera)
			return;

		CharacterControllerComponent controller = chimera.GetCharacterController();
		if (!controller || controller.IsUnconscious())
		{
			st.m_fStrain = 0;
			st.m_fNoKnockoutUntil = now + KNOCKOUT_COOLDOWN;
			return;
		}

		// Ground speed from how far they moved since the last check
		vector pos = chimera.GetOrigin();
		float speed = vector.DistanceXZ(pos, st.m_vLastPos) / TICK;
		if (st.m_vLastPos == vector.Zero || speed > 15)
			speed = 0;

		st.m_vLastPos = pos;
		bool upright = controller.GetStance() == ECharacterStance.STAND;

		// About 6 seconds walking or running upright before you pass out
		if (upright && speed > WALK_SPEED)
			st.m_fStrain += 1;
		else
			st.m_fStrain = Math.Max(0, st.m_fStrain - 0.5);

		SCR_ChimeraCharacter scrChar = SCR_ChimeraCharacter.Cast(chimera);
		if (upright && speed > WALK_SPEED && scrChar)
		{
			scrChar.ArmstCameraShake(0.05 + st.m_fStrain * 0.03, 0.15 + st.m_fStrain * 0.08, 0.05, 0.15, 0.35);
			scrChar.SRZ_SendLegSfx(false);
		}

		if (st.m_fStrain < STRAIN_KNOCKOUT || now < st.m_fNoKnockoutUntil)
			return;

		st.m_fStrain = 0;
		st.m_fNoKnockoutUntil = now + KNOCKOUT_COOLDOWN;

		SCR_CharacterDamageManagerComponent damage = SCR_CharacterDamageManagerComponent.Cast(chimera.GetDamageManager());
		if (damage)
		{
			Print("[SRZ Legs] Passed out from walking on a broken leg");
			damage.ForceUnconsciousness();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static void Apply(SRZ_LegState st)
	{
		Save(st);
		SetSpeed(st.m_Char, StateSpeed(st.m_iState));

		SCR_ChimeraCharacter chimera = SCR_ChimeraCharacter.Cast(st.m_Char);
		if (!chimera)
			return;

		if (chimera == SCR_PlayerController.GetLocalControlledEntity())
			OnLocalState(chimera, st.m_iState);
		else
			chimera.SRZ_SendLegState(st.m_iState);
	}

	//------------------------------------------------------------------------------------------------
	protected static void SetSpeed(IEntity character, float speed)
	{
		ChimeraCharacter chimera = ChimeraCharacter.Cast(character);
		if (!chimera || !chimera.GetCharacterController())
			return;

		chimera.GetCharacterController().OverrideMaxSpeed(speed);
	}

	//------------------------------------------------------------------------------------------------
	// On the player's own machine: movement cap and the HUD icon
	static void OnLocalState(IEntity character, int state)
	{
		SetSpeed(character, StateSpeed(state));
		SRZ_LegHud.Show(state);
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
	protected static string SRZ_SaveFile(SRZ_LegState st)
	{
		return SAVE_DIR + "/" + st.m_sUid + ".txt";
	}

	//------------------------------------------------------------------------------------------------
	protected static void Save(SRZ_LegState st)
	{
		if (st.m_iState == HEALTHY)
		{
			if (FileIO.FileExists(SRZ_SaveFile(st)))
				FileIO.DeleteFile(SRZ_SaveFile(st));

			return;
		}

		FileIO.MakeDirectory(SAVE_DIR);
		FileHandle file = FileIO.OpenFile(SRZ_SaveFile(st), FileMode.WRITE);
		if (!file)
			return;

		file.WriteLine(st.m_iState.ToString());
		file.WriteLine(((int)st.m_fHealLeft).ToString());
		file.Close();
	}

	//------------------------------------------------------------------------------------------------
	protected static void Load(SRZ_LegState st)
	{
		if (!FileIO.FileExists(SRZ_SaveFile(st)))
			return;

		FileHandle file = FileIO.OpenFile(SRZ_SaveFile(st), FileMode.READ);
		if (!file)
			return;

		string line;
		file.ReadLine(line);
		st.m_iState = line.ToInt();
		file.ReadLine(line);
		st.m_fHealLeft = line.ToInt();
		if (st.m_fHealLeft <= 0)
			st.m_fHealLeft = SELF_HEAL_TIME;
		file.Close();

		if (st.m_iState == BROKEN || st.m_iState == SPLINTED)
		{
			Print("[SRZ Legs] Restored saved leg state " + st.m_iState);
			GetGame().GetCallqueue().CallLater(Apply, 3000, false, st);
		}
	}
}

// Some mutant hits can break a leg
modded class ARMST_Monster_MeleeComponent
{
	override protected void HandleDamageDelayed()
	{
		IEntity target = m_MeleeHitData.m_Entity;
		super.HandleDamageDelayed();
		SRZ_Legs.OnMutantHit(GetOwner(), target);
	}
}

// The splint is used like any ARMST medical item
modded class ARMST_Consumable
{
	override void AddConsumableDamageEffects(notnull ChimeraCharacter char, IEntity instigator)
	{
		super.AddConsumableDamageEffects(char, instigator);
		SRZ_Legs.OnItemUsed(char, Temp);
	}
}

// Measures every fall: the highest point while airborne against where the player lands
class SRZ_FallWatch
{
	protected static const float TICK = 0.1;

	protected static ref map<IEntity, float> s_mTop = new map<IEntity, float>();
	protected static ref map<IEntity, float> s_mGround = new map<IEntity, float>();
	protected static bool s_bTicking;

	//------------------------------------------------------------------------------------------------
	static void Start()
	{
		if (s_bTicking || !Replication.IsServer())
			return;

		s_bTicking = true;
		GetGame().GetCallqueue().CallLater(Tick, TICK * 1000, true);
	}

	//------------------------------------------------------------------------------------------------
	protected static void Tick()
	{
		array<int> ids = {};
		GetGame().GetPlayerManager().GetPlayers(ids);
		foreach (int id : ids)
		{
			ChimeraCharacter character = ChimeraCharacter.Cast(GetGame().GetPlayerManager().GetPlayerControlledEntity(id));
			if (character)
				Check(character);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static void Check(ChimeraCharacter character)
	{
		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller || controller.IsDead() || character.IsInVehicle())
		{
			s_mTop.Remove(character);
			s_mGround.Remove(character);
			return;
		}

		float y = character.GetOrigin()[1];
		if (controller.IsFalling())
		{
			// Start from the last height they stood at, so the drop isn't measured short
			if (!s_mTop.Contains(character))
			{
				float start = y;
				if (s_mGround.Contains(character))
					start = Math.Max(y, s_mGround.Get(character));

				s_mTop.Set(character, start);
			}
			else if (y > s_mTop.Get(character))
			{
				s_mTop.Set(character, y);
			}

			return;
		}

		s_mGround.Set(character, y);
		if (!s_mTop.Contains(character))
			return;

		float drop = s_mTop.Get(character) - y;
		s_mTop.Remove(character);
		SRZ_Legs.OnLanded(character, drop);
	}
}

modded class ARMST_PLAYER_STATS_COMPONENT
{
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		if (GetGame().InPlayMode() && !SCR_Global.IsEditMode())
			SRZ_FallWatch.Start();
	}
}
