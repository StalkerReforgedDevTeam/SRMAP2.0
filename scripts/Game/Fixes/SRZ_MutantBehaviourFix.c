// Mutants: one health pool (the Health MaxHealth in the prefab), no hitzone multipliers, no bleeding, no limb effects.
// Every hit takes its raw damage off the pool. At 0 the mutant dies and the shooter gets the kill.
// Also stops the random tripping on contact and lets them try to climb while moving.

// Per mutant tuning, matched by prefab path
class SRZ_MutantTuning
{
	// Raise above 1 to make every mutant die faster, lower to make them tougher
	static const float DAMAGE_SCALE = 1.0;

	static const string NIGHTSTALKER = "nighstalker";
	static const float NIGHTSTALKER_HEALTH = 450;
	static const float NIGHTSTALKER_BLOOD_DRAIN_PERCENT = 20;

	static bool IsPrefab(IEntity ent, string match)
	{
		if (!ent || !ent.GetPrefabData())
			return false;

		string path = ent.GetPrefabData().GetPrefabName();
		path.ToLower();
		return path.Contains(match);
	}

	static float GetPool(IEntity mutant, float defaultHealth)
	{
		if (IsPrefab(mutant, NIGHTSTALKER))
			return NIGHTSTALKER_HEALTH;

		return defaultHealth;
	}

	static float GetBloodDrainPercent(IEntity mutant)
	{
		if (IsPrefab(mutant, NIGHTSTALKER))
			return NIGHTSTALKER_BLOOD_DRAIN_PERCENT;

		return 0;
	}
}

// Mutants with a blood drain take that percent of max blood on every melee hit
modded class ARMST_Monster_MeleeComponent
{
	override protected void HandleDamageDelayed()
	{
		IEntity target = m_MeleeHitData.m_Entity;
		super.HandleDamageDelayed();

		if (!Replication.IsServer())
			return;

		float drain = SRZ_MutantTuning.GetBloodDrainPercent(GetOwner());
		if (drain <= 0)
			return;

		ChimeraCharacter victim = ChimeraCharacter.Cast(target);
		if (!victim || Bacon_622120A5448725E3_InfectedCharacter.Cast(victim))
			return;

		SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(victim.GetDamageManager());
		if (!damageMgr)
			return;

		SCR_CharacterBloodHitZone blood = damageMgr.GetBloodHitZone();
		if (!blood)
			return;

		float newBlood = blood.GetHealth() - blood.GetMaxHealth() * drain / 100;
		if (newBlood < 0)
			newBlood = 0;

		blood.SetHealth(newBlood);
	}
}

modded class Bacon_622120A5448725E3_InfectedCharacter
{
	protected float m_fSRZ_Pool = -1;

	void SRZ_ApplyPoolDamage(float damage, Instigator instigator)
	{
		if (!m_damageManager || !m_controller || m_controller.GetLifeState() == ECharacterLifeState.DEAD)
			return;

		if (m_fSRZ_Pool < 0)
		{
			HitZone health = m_damageManager.GetDefaultHitZone();
			if (!health)
				return;

			m_fSRZ_Pool = SRZ_MutantTuning.GetPool(this, health.GetMaxHealth());
		}

		m_fSRZ_Pool -= damage;
		if (m_fSRZ_Pool > 0)
			return;

		if (instigator)
			m_damageManager.Kill(instigator);
		else
			m_damageManager.Kill(Instigator.CreateInstigator(null));
	}

	override void TryTrip()
	{
	}
}

modded class SCR_CharacterHitZone
{
	override float ComputeEffectiveDamage(notnull BaseDamageContext damageContext, bool isDOT)
	{
		Bacon_622120A5448725E3_InfectedCharacter mutant = Bacon_622120A5448725E3_InfectedCharacter.Cast(damageContext.hitEntity);
		if (!mutant)
			return super.ComputeEffectiveDamage(damageContext, isDOT);

		if (!Replication.IsServer() || damageContext.damageType == EDamageType.BLEEDING)
			return 0;

		// Count each hit once: on the zone that was struck, or on the default zone when none was given
		HitZone struck = damageContext.struckHitZone;
		if (struck && struck != this)
			return 0;

		if (!struck && mutant.m_damageManager && mutant.m_damageManager.GetDefaultHitZone() != this)
			return 0;

		// Real damage this hit would normally do (weapon, armor and body multipliers), taken off the single pool
		float effective = super.ComputeEffectiveDamage(damageContext, isDOT);
		if (effective > 0)
			mutant.SRZ_ApplyPoolDamage(effective * SRZ_MutantTuning.DAMAGE_SCALE, damageContext.instigator);

		return 0;
	}
}

modded class Bacon_622120A5448725E3_ClumsinessComponent
{
	override void EOnContact(IEntity owner, IEntity other, Contact contact)
	{
	}
}

// Bacon only tried to climb at exactly zero speed, which almost never happens while the AI is pushing into a wall
modded class Bacon_622120A5448725E3_AIRequestClimb
{
	protected static const float SRZ_CLIMB_MAX_SPEED = 1.0;

	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_Character || m_Character.IsInVehicle() || !m_Character.GetPhysics())
			return ENodeResult.FAIL;

		bool canClimb;
		if (m_Character.GetPhysics().GetVelocity().Length() <= SRZ_CLIMB_MAX_SPEED)
		{
			CharacterCommandClimbResult climbResult = new CharacterCommandClimbResult();
			CharacterCommandClimb.DoClimbTest(m_Character, m_ClimbSettings, climbResult);

			if (climbResult.m_bIsClimbOver)
			{
				canClimb = true;
				if (climbResult.m_bFinishWithFall && m_Character.ShouldPreventFalling())
				{
					TraceParam trace = new TraceParam();
					trace.Start = climbResult.m_ClimbOverStandPointLS;
					trace.End = climbResult.m_ClimbOverStandPointLS - (vector.Up * MAX_FALL_HEIGHT);
					trace.LayerMask = EPhysicsLayerDefs.Projectile;
					trace.Flags = TraceFlags.ENTS | TraceFlags.OCEAN | TraceFlags.WORLD;

					if (GetGame().GetWorld().TraceMove(trace, TraceFilterAlwaysAbort) > 0.99)
						canClimb = false;
				}
			}
			else if (climbResult.m_bIsClimb)
			{
				canClimb = true;
			}
		}

		SetVariableOut("CanClimb", canClimb);
		if (!canClimb)
			return ENodeResult.FAIL;

		return ENodeResult.SUCCESS;
	}
}
