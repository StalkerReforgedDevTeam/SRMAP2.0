// Boars and toxic bichura sometimes stop in front of a player without swinging.
// If one stands in reach of a hostile player and its own melee hasn't landed, it attacks on its own.
class SRZ_MutantReach
{
	protected static const float TICK = 0.5;
	protected static const float REACH = 2.3;
	protected static const float MAX_HEIGHT_DIFF = 1.6;
	protected static const float FACING_DOT = 0.5;
	protected static const float NATIVE_GRACE = 2.5;
	protected static const float COOLDOWN = 1.8;
	protected static const float HIT_DELAY = 0.45;

	protected static ref array<string> s_aMatches = { "/boar/", "/zombie/toxic/" };
	protected static ref array<Bacon_622120A5448725E3_InfectedCharacter> s_aMutants = {};
	protected static bool s_bTicking;

	//------------------------------------------------------------------------------------------------
	static bool ShouldTrack(IEntity mutant)
	{
		foreach (string match : s_aMatches)
		{
			if (SRZ_MutantTuning.IsPrefab(mutant, match))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	static void Register(Bacon_622120A5448725E3_InfectedCharacter mutant)
	{
		if (!Replication.IsServer() || !mutant || s_aMutants.Contains(mutant))
			return;

		s_aMutants.Insert(mutant);
		if (!s_bTicking)
		{
			s_bTicking = true;
			GetGame().GetCallqueue().CallLater(Tick, TICK * 1000, true);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static void Tick()
	{
		array<int> ids = {};
		GetGame().GetPlayerManager().GetPlayers(ids);
		array<IEntity> players = {};
		foreach (int id : ids)
		{
			IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(id);
			if (ent && !IsDead(ent))
				players.Insert(ent);
		}

		float now = GetGame().GetWorld().GetWorldTime() / 1000;
		for (int i = s_aMutants.Count() - 1; i >= 0; i--)
		{
			Bacon_622120A5448725E3_InfectedCharacter mutant = s_aMutants[i];
			if (!mutant || IsDead(mutant))
			{
				s_aMutants.Remove(i);
				continue;
			}

			if (players.IsEmpty())
				continue;

			ARMST_Monster_MeleeComponent melee = ARMST_Monster_MeleeComponent.Cast(mutant.FindComponent(ARMST_Monster_MeleeComponent));
			if (!melee)
				continue;

			IEntity target = FindTarget(mutant, players);
			melee.SRZ_ReachUpdate(target, now);
		}

		if (s_aMutants.IsEmpty())
		{
			GetGame().GetCallqueue().Remove(Tick);
			s_bTicking = false;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static IEntity FindTarget(IEntity mutant, array<IEntity> players)
	{
		vector origin = mutant.GetOrigin();
		vector forward = mutant.GetTransformAxis(2);
		forward[1] = 0;
		forward.Normalize();

		IEntity best;
		float bestDist = REACH;
		foreach (IEntity player : players)
		{
			vector to = player.GetOrigin() - origin;
			if (Math.AbsFloat(to[1]) > MAX_HEIGHT_DIFF)
				continue;

			to[1] = 0;
			float dist = to.Length();
			if (dist > bestDist || dist < 0.01)
				continue;

			to.Normalize();
			if (vector.Dot(forward, to) < FACING_DOT)
				continue;

			if (IsFriendly(mutant, player))
				continue;

			best = player;
			bestDist = dist;
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	static bool InReach(IEntity mutant, IEntity target)
	{
		if (!mutant || !target || IsDead(target))
			return false;

		vector to = target.GetOrigin() - mutant.GetOrigin();
		if (Math.AbsFloat(to[1]) > MAX_HEIGHT_DIFF)
			return false;

		to[1] = 0;
		return to.Length() <= REACH + 0.4;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool IsFriendly(IEntity mutant, IEntity player)
	{
		FactionAffiliationComponent mf = FactionAffiliationComponent.Cast(mutant.FindComponent(FactionAffiliationComponent));
		FactionAffiliationComponent pf = FactionAffiliationComponent.Cast(player.FindComponent(FactionAffiliationComponent));
		if (!mf || !pf)
			return false;

		SCR_Faction a = SCR_Faction.Cast(mf.GetAffiliatedFaction());
		Faction b = pf.GetAffiliatedFaction();
		if (!a || !b)
			return false;

		return a == b || a.IsFactionFriendly(b);
	}

	//------------------------------------------------------------------------------------------------
	static bool IsDead(IEntity ent)
	{
		ChimeraCharacter chimera = ChimeraCharacter.Cast(ent);
		if (!chimera)
			return true;

		CharacterControllerComponent controller = chimera.GetCharacterController();
		return !controller || controller.IsDead();
	}

	//------------------------------------------------------------------------------------------------
	static float GetCooldown() { return COOLDOWN; }
	static float GetGrace() { return NATIVE_GRACE; }
	static float GetHitDelay() { return HIT_DELAY; }
}

modded class ARMST_Monster_MeleeComponent
{
	protected float m_fSRZ_LastHit = -100;
	protected float m_fSRZ_LastReach = -100;
	protected float m_fSRZ_InReachSince = -1;
	protected IEntity m_SRZ_ReachTarget;

	//------------------------------------------------------------------------------------------------
	override protected void HandleDamageDelayed()
	{
		super.HandleDamageDelayed();
		m_fSRZ_LastHit = GetGame().GetWorld().GetWorldTime() / 1000;
	}

	//------------------------------------------------------------------------------------------------
	void SRZ_ReachUpdate(IEntity target, float now)
	{
		if (!target)
		{
			m_fSRZ_InReachSince = -1;
			return;
		}

		if (m_fSRZ_InReachSince < 0)
			m_fSRZ_InReachSince = now;

		// Give the normal melee a chance first
		if (now - m_fSRZ_InReachSince < 1)
			return;

		if (now - m_fSRZ_LastHit < SRZ_MutantReach.GetGrace())
			return;

		if (now - m_fSRZ_LastReach < SRZ_MutantReach.GetCooldown())
			return;

		m_fSRZ_LastReach = now;
		m_SRZ_ReachTarget = target;

		if (m_fMonsterAnimationToggle)
		{
			ARMST_PLAYER_ANIMATION anim = ARMST_PLAYER_ANIMATION.Cast(GetOwner().FindComponent(ARMST_PLAYER_ANIMATION));
			if (anim)
				anim.StartAnimationsPlayer(m_sMonsterAnimationASI, false, false, m_TimerMonsterAnimation);
		}

		GetGame().GetCallqueue().CallLater(SRZ_ReachHit, SRZ_MutantReach.GetHitDelay() * 1000, false);
	}

	//------------------------------------------------------------------------------------------------
	protected void SRZ_ReachHit()
	{
		IEntity target = m_SRZ_ReachTarget;
		m_SRZ_ReachTarget = null;
		if (!GetOwner() || SRZ_MutantReach.IsDead(GetOwner()) || !SRZ_MutantReach.InReach(GetOwner(), target))
			return;

		SCR_CharacterDamageManagerComponent damageManager = SCR_CharacterDamageManagerComponent.Cast(target.FindComponent(SCR_CharacterDamageManagerComponent));
		if (!damageManager)
			return;

		HitZone hitZone = damageManager.GetHitZoneByName("Chest");
		if (!hitZone)
			hitZone = damageManager.GetDefaultHitZone();

		if (!hitZone)
			return;

		float damage = m_fMonsterDamage;
		ARMST_ITEMS_STATS_COMPONENTS stats = ARMST_ITEMS_STATS_COMPONENTS.Cast(target.FindComponent(ARMST_ITEMS_STATS_COMPONENTS));
		if (stats)
			damage = m_fMonsterDamage * (1.0 - stats.GetAllPhysicals(target) / 100.0);

		vector dir = target.GetOrigin() - GetOwner().GetOrigin();
		dir.Normalize();
		vector hitPosDirNorm[3];
		hitPosDirNorm[0] = target.GetOrigin() + "0 1 0";
		hitPosDirNorm[1] = dir;
		hitPosDirNorm[2] = -dir;

		SCR_DamageContext context = new SCR_DamageContext(EDamageType.MELEE, damage, hitPosDirNorm, target, hitZone, Instigator.CreateInstigator(GetOwner()), null, -1, -1);
		context.damageEffect = new SCR_MeleeDamageEffect();
		damageManager.HandleDamage(context);
		m_fSRZ_LastHit = GetGame().GetWorld().GetWorldTime() / 1000;

		if (m_fPlayerAnimationToggle && EntityUtils.IsPlayer(target) && m_ChancePlayerAnimation > Math.RandomInt(0, 99))
		{
			ARMST_PLAYER_ANIMATION playerAnim = ARMST_PLAYER_ANIMATION.Cast(target.FindComponent(ARMST_PLAYER_ANIMATION));
			if (playerAnim)
				playerAnim.StartAnimationsPlayerByName("Falling");
		}
	}
}

modded class Bacon_622120A5448725E3_InfectedCharacter
{
	override protected void DelayedInitServer()
	{
		super.DelayedInitServer();

		if (SRZ_MutantReach.ShouldTrack(this))
			SRZ_MutantReach.Register(this);
	}
}

// The melee line of sight check only ignored the two characters, not their attached gear,
// so particles and damage areas carried by some mutants could block it
modded class Bacon_622120A5448725E3_AICanMeleeTarget
{
	override bool TestFunction(AIAgent owner)
	{
		IEntity target;
		if (!GetVariableIn(TARGET_PORT, target) || !target)
			return false;

		IEntity entity = owner.GetControlledEntity();
		if (!entity)
			return false;

		if (!m_tParams)
		{
			m_tParams = new TraceParam();
			m_tParams.Flags = TraceFlags.ANY_CONTACT;
			m_tParams.LayerMask = EPhysicsLayerDefs.Default;
		}

		array<IEntity> excluded = {};
		SRZ_AddHierarchy(entity, excluded);
		SRZ_AddHierarchy(target, excluded);
		m_tParams.ExcludeArray = excluded;
		m_tParams.Start = entity.GetOrigin() + "0 1 0";
		m_tParams.End = target.GetOrigin() + "0 1 0";

		return GetGame().GetWorld().TraceMove(m_tParams, null) == 1;
	}

	protected void SRZ_AddHierarchy(IEntity ent, array<IEntity> list)
	{
		if (!ent || list.Count() > 64)
			return;

		list.Insert(ent);
		IEntity child = ent.GetChildren();
		while (child)
		{
			SRZ_AddHierarchy(child, list);
			child = child.GetSibling();
		}
	}
}
