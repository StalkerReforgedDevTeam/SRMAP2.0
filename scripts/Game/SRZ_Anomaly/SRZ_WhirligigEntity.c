class SRZ_WhirligigEntityClass : ARMST_DamagingTriggerEntityClass
{
}

class SRZ_WhirligigEntity : ARMST_DamagingTriggerEntity
{
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Idle effect, plays on every client near it", category: "7. Whirligig", params: "ptc")]
	ResourceName m_IdleEffect;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Effect while it spins", category: "7. Whirligig", params: "ptc")]
	ResourceName m_SpinEffect;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Effect when it collapses", category: "7. Whirligig", params: "ptc")]
	ResourceName m_CollapseEffect;

	[Attribute("0", UIWidgets.Slider, "Height the idle effect floats above the anomaly, metres", "0 5 0.1", category: "7. Whirligig")]
	float m_fIdleHeight;

	[Attribute("", UIWidgets.EditBox, "Looping idle sound event, played through the prefab's Sound Component", category: "7. Whirligig")]
	string m_sIdleSoundEvent;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Sound project for the spin", category: "7. Whirligig", params: "acp")]
	ResourceName m_SpinSoundProject;

	[Attribute("", UIWidgets.EditBox, "Sound event when it starts to spin, stopped at the collapse", category: "7. Whirligig")]
	string m_sSpinSoundEvent;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Sound project for the collapse", category: "7. Whirligig", params: "acp")]
	ResourceName m_CollapseSoundProject;

	[Attribute("", UIWidgets.EditBox, "Sound event when it collapses", category: "7. Whirligig")]
	string m_sCollapseSoundEvent;

	[Attribute("1", UIWidgets.CheckBox, "Lift and spin victims. Off turns it into a Vortex: pull in, then crush and throw", category: "7. Whirligig")]
	bool m_bSpin;

	[Attribute("3", UIWidgets.Slider, "Vortex only: seconds victims stay down after the crush", "1 10 0.5", category: "7. Whirligig")]
	float m_fCrushDownSeconds;

	[Attribute("1.2", UIWidgets.Slider, "Seconds between being caught and being lifted. Anyone who gets out of the sphere in time escapes", "0 5 0.1", category: "7. Whirligig")]
	float m_fPullSeconds;

	[Attribute("0", UIWidgets.CheckBox, "Plays the spin effect and sound as a warning while it is about to lift you", category: "7. Whirligig")]
	bool m_bWindUp;

	[Attribute("0", UIWidgets.CheckBox, "Everyone inside the crush radius at the collapse dies outright", category: "7. Whirligig")]
	bool m_bLethal;

	[Attribute("4", UIWidgets.Slider, "Shortest spin, seconds", "1 20 0.5", category: "7. Whirligig")]
	float m_fSpinMinSeconds;

	[Attribute("6", UIWidgets.Slider, "Longest spin, seconds", "1 20 0.5", category: "7. Whirligig")]
	float m_fSpinMaxSeconds;

	[Attribute("4", UIWidgets.Slider, "Spin height above the anomaly, metres", "1 10 0.1", category: "7. Whirligig")]
	float m_fSpinHeight;

	[Attribute("1", UIWidgets.Slider, "Spin radius, metres", "0.2 4 0.1", category: "7. Whirligig")]
	float m_fSpinRadius;

	[Attribute("7", UIWidgets.Slider, "Spin speed, metres per second", "1 20 0.5", category: "7. Whirligig")]
	float m_fSpinSpeed;

	[Attribute("5", UIWidgets.Slider, "How hard the body is held on the circle", "1 15 0.5", category: "7. Whirligig")]
	float m_fSpinGrip;

	[Attribute("1.5", UIWidgets.Slider, "Within this distance of the centre at the collapse, full damage", "0 5 0.1", category: "7. Whirligig")]
	float m_fCrushRadius;

	[Attribute("0.4", UIWidgets.Slider, "Damage share outside the crush radius", "0 1 0.05", category: "7. Whirligig")]
	float m_fOuterDamageScale;

	[Attribute("8", UIWidgets.Slider, "Slowest outward throw, m/s", "0 40 1", category: "7. Whirligig")]
	float m_fThrowMin;

	[Attribute("16", UIWidgets.Slider, "Fastest outward throw, m/s", "0 40 1", category: "7. Whirligig")]
	float m_fThrowMax;

	[Attribute("4", UIWidgets.Slider, "Upward part of the throw, m/s", "0 20 0.5", category: "7. Whirligig")]
	float m_fThrowUp;

	[Attribute("6", UIWidgets.Slider, "Seconds before it can catch again", "0 60 1", category: "7. Whirligig")]
	float m_fCooldownSeconds;

	[Attribute("12", UIWidgets.Slider, "Speed a bolt is flung away at", "0 40 1", category: "7. Whirligig")]
	float m_fBoltThrow;

	[Attribute("200", UIWidgets.Slider, "Effects and sounds play for players within this range", "20 1000 10", category: "7. Whirligig")]
	float m_fEffectRange;

	protected static const int STATE_IDLE = 0;
	protected static const int STATE_PULL = 1;
	protected static const int STATE_SPIN = 2;
	protected static const int STATE_COOLDOWN = 3;
	protected static const float BOLT_SPIN_S = 1.5;

	protected int m_iState = STATE_IDLE;
	protected ref array<ChimeraCharacter> m_aVictims = {};
	protected ref array<ChimeraCharacter> m_aInside = {};
	protected RplComponent m_Rpl;
	protected ParticleEffectEntity m_Idle;

	override void OnInit(IEntity owner)
	{
		super.OnInit(owner);
		m_Rpl = RplComponent.Cast(FindComponent(RplComponent));
		if (!GetGame().InPlayMode() || SRZ_FXRange.IsHeadless())
			return;

		SoundComponent sound = SoundComponent.Cast(FindComponent(SoundComponent));
		if (sound && !m_sIdleSoundEvent.IsEmpty())
			sound.SoundEvent(m_sIdleSoundEvent);

		if (m_IdleEffect.IsEmpty())
			return;

		ParticleEffectEntitySpawnParams params = new ParticleEffectEntitySpawnParams();
		if (m_fIdleHeight > 0)
		{
			params.TransformMode = ETransformMode.WORLD;
			params.Transform[3] = GetOrigin() + Vector(0, m_fIdleHeight, 0);
		}
		else
		{
			params.Parent = this;
		}

		m_Idle = ParticleEffectEntity.SpawnParticleEffect(m_IdleEffect, params);
	}

	void ~SRZ_WhirligigEntity()
	{
		if (!GetGame() || !GetGame().GetCallqueue())
			return;

		GetGame().GetCallqueue().Remove(StartSpin);
		GetGame().GetCallqueue().Remove(Collapse);
		GetGame().GetCallqueue().Remove(EndCooldown);
		if (m_Idle && m_fIdleHeight > 0)
			SCR_EntityHelper.DeleteEntityAndChildren(m_Idle);
	}

	override void OnActivate(IEntity ent)
	{
		if (!Replication.IsServer() || !ent)
			return;

		ChimeraCharacter character = ChimeraCharacter.Cast(ent);
		if (!character)
		{
			super.OnActivate(ent);
			TryBolt(ent);
			return;
		}

		if (!CanCatch(character))
			return;

		if (!m_aInside.Contains(character))
			m_aInside.Insert(character);

		if (m_iState == STATE_IDLE)
			Begin();

		if (m_iState == STATE_PULL)
			Catch(character);
	}

	override void OnDeactivate(IEntity ent)
	{
		super.OnDeactivate(ent);
		ChimeraCharacter character = ChimeraCharacter.Cast(ent);
		if (character)
			m_aInside.RemoveItem(character);
	}

	protected bool CanCatch(ChimeraCharacter character)
	{
		if (!character || character.IsDeleted())
			return false;

		if (!m_fMonsterDamage && !EntityUtils.IsPlayer(character))
			return false;

		if (!IsAlive(character) || character.IsInVehicle())
			return false;

		CharacterControllerComponent controller = character.GetCharacterController();
		return controller && controller.GetLifeState() != ECharacterLifeState.DEAD;
	}

	// Anyone already standing in it, including people who walked in while it was busy
	protected bool CatchEveryoneInside()
	{
		bool any;
		for (int i = m_aInside.Count() - 1; i >= 0; i--)
		{
			ChimeraCharacter c = m_aInside[i];
			if (!CanCatch(c) || !StillInside(c))
			{
				m_aInside.Remove(i);
				continue;
			}

			any = true;
			if (m_iState == STATE_PULL)
				Catch(c);
		}

		return any;
	}

	protected void Begin()
	{
		if (SRZ_WhirlBody.DEBUG)
			Print("[SRZ Whirl] caught something, pulling", LogLevel.NORMAL);

		m_iState = STATE_PULL;
		m_aVictims.Clear();
		if (m_bWindUp)
			PlayFx(m_SpinEffect, m_SpinSoundProject, m_sSpinSoundEvent, GetOrigin(), m_fPullSeconds);

		GetGame().GetCallqueue().CallLater(StartSpin, Math.Max(m_fPullSeconds, 0.1) * 1000, false);
		CatchEveryoneInside();
	}

	protected void Catch(ChimeraCharacter character)
	{
		if (m_aVictims.Contains(character))
			return;

		m_aVictims.Insert(character);
		if (!m_bWindUp)
			return;

		SCR_ChimeraCharacter chimera = SCR_ChimeraCharacter.Cast(character);
		if (chimera)
			chimera.ArmstCameraShake(0.6, 3, 0.4, Math.Max(m_fPullSeconds - 0.8, 0.2), 0.4);
	}

	protected bool StillInside(ChimeraCharacter victim)
	{
		return vector.DistanceXZ(victim.GetOrigin(), GetOrigin()) <= GetSphereRadius() + 0.5;
	}

	protected void StartSpin()
	{
		if (!m_bSpin)
		{
			Crush();
			return;
		}

		m_iState = STATE_SPIN;
		float spinS = Math.RandomFloat(Math.Min(m_fSpinMinSeconds, m_fSpinMaxSeconds), Math.Max(m_fSpinMinSeconds, m_fSpinMaxSeconds));
		vector centre = GetOrigin();
		float dir = 1;
		if (Math.RandomFloat01() < 0.5)
			dir = -1;

		for (int i = m_aVictims.Count() - 1; i >= 0; i--)
		{
			ChimeraCharacter victim = m_aVictims[i];
			SCR_CharacterControllerComponent controller;
			if (victim)
				controller = SCR_CharacterControllerComponent.Cast(victim.GetCharacterController());

			if (!controller || controller.GetLifeState() == ECharacterLifeState.DEAD || victim.IsInVehicle() || !StillInside(victim))
			{
				m_aVictims.Remove(i);
				continue;
			}

			controller.SRZ_WhirlRagdoll((spinS + 3) * 1000);
			controller.SRZ_BroadcastWhirlSpin(centre, m_fSpinHeight, m_fSpinRadius, m_fSpinSpeed, m_fSpinGrip, spinS + 0.5, dir);
		}

		if (SRZ_WhirlBody.DEBUG)
			Print(string.Format("[SRZ Whirl] spinning %1 victim(s) for %2 s", m_aVictims.Count(), spinS), LogLevel.NORMAL);

		PlayFx(m_SpinEffect, m_SpinSoundProject, m_sSpinSoundEvent, centre, spinS);
		GetGame().GetCallqueue().CallLater(Collapse, spinS * 1000, false);
	}

	protected void Crush()
	{
		m_iState = STATE_SPIN;
		for (int i = m_aVictims.Count() - 1; i >= 0; i--)
		{
			ChimeraCharacter victim = m_aVictims[i];
			SCR_CharacterControllerComponent controller;
			if (victim)
				controller = SCR_CharacterControllerComponent.Cast(victim.GetCharacterController());

			if (!controller || controller.GetLifeState() == ECharacterLifeState.DEAD || victim.IsInVehicle() || !StillInside(victim))
			{
				m_aVictims.Remove(i);
				continue;
			}

			controller.SRZ_WhirlRagdoll(m_fCrushDownSeconds * 1000);
		}

		Collapse();
	}

	protected void Collapse()
	{
		if (SRZ_WhirlBody.DEBUG)
			Print(string.Format("[SRZ Whirl] collapse, %1 victim(s)", m_aVictims.Count()), LogLevel.NORMAL);

		vector centre = GetOrigin();
		PlayFx(m_CollapseEffect, m_CollapseSoundProject, m_sCollapseSoundEvent, centre + Vector(0, m_fSpinHeight * 0.6, 0), 0);

		float baseDamage = m_damageValue;
		bool baseThrow = m_DamageForceAction;
		m_DamageForceAction = false;

		foreach (ChimeraCharacter victim : m_aVictims)
		{
			if (!victim || victim.IsDeleted())
				continue;

			SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(victim.GetCharacterController());
			if (!controller)
				continue;

			vector body = SRZ_WhirlBody.BodyPosition(victim);
			vector away = body - centre;
			away[1] = 0;
			float dist = away.Length();
			if (dist > 0.05)
				away = away * (1 / dist);
			else
				away = Vector(Math.RandomFloat(-1, 1), 0, Math.RandomFloat(-1, 1)).Normalized();

			if (controller.GetLifeState() != ECharacterLifeState.DEAD)
			{
				m_damageValue = baseDamage;
				if (dist > m_fCrushRadius)
					m_damageValue = baseDamage * m_fOuterDamageScale;

				if (m_bLethal && dist <= m_fCrushRadius)
					Kill(victim);
				else
					ArmstDamageStart(victim);
			}

			vector fling = away * Math.RandomFloat(Math.Min(m_fThrowMin, m_fThrowMax), Math.Max(m_fThrowMin, m_fThrowMax)) + Vector(0, m_fThrowUp, 0);
			controller.SRZ_BroadcastWhirlFling(fling);
		}

		m_damageValue = baseDamage;
		m_DamageForceAction = baseThrow;
		m_aVictims.Clear();
		m_iState = STATE_COOLDOWN;
		GetGame().GetCallqueue().CallLater(EndCooldown, Math.Max(m_fCooldownSeconds, 0.5) * 1000, false);
	}

	protected void Kill(ChimeraCharacter victim)
	{
		SCR_CharacterDamageManagerComponent damageManager = SCR_CharacterDamageManagerComponent.Cast(victim.FindComponent(SCR_CharacterDamageManagerComponent));
		if (!damageManager)
			return;

		BaseDamageContext ctx = new BaseDamageContext();
		ctx.damageValue = 100000;
		ctx.damageType = EDamageType.TRUE;
		ctx.struckHitZone = damageManager.GetDefaultHitZone();
		damageManager.HandleDamage(ctx);
		SRZ_RuinIfDead(victim);
	}

	protected void EndCooldown()
	{
		m_iState = STATE_IDLE;
		if (CatchEveryoneInside())
		{
			Begin();
			CatchEveryoneInside();
		}
	}

	protected void TryBolt(IEntity ent)
	{
		if (!m_fBoltAction || !ent.FindComponent(GrenadeMoveComponent))
			return;

		Physics physics = ent.GetPhysics();
		if (physics && physics.IsDynamic())
		{
			vector away = ent.GetOrigin() - GetOrigin();
			away[1] = 0;
			if (away.LengthSq() < 0.01)
				away = Vector(1, 0, 0);

			away.Normalize();
			physics.SetVelocity(away * m_fBoltThrow + Vector(0, m_fBoltThrow * 0.6, 0));
		}

		if (m_iState != STATE_IDLE)
			return;

		PlayFx(m_SpinEffect, m_SpinSoundProject, m_sSpinSoundEvent, GetOrigin(), BOLT_SPIN_S);
		m_iState = STATE_COOLDOWN;
		GetGame().GetCallqueue().CallLater(EndCooldown, (BOLT_SPIN_S + 1) * 1000, false);
	}

	protected void PlayFx(ResourceName effect, ResourceName soundProject, string soundEvent, vector pos, float lifeS)
	{
		FxLocal(effect, soundProject, soundEvent, pos, lifeS);
		if (m_Rpl && !m_Rpl.IsProxy())
			Rpc(RpcDo_Fx, effect, soundProject, soundEvent, pos, lifeS);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_Fx(string effect, string soundProject, string soundEvent, vector pos, float lifeS)
	{
		FxLocal(effect, soundProject, soundEvent, pos, lifeS);
	}

	protected void FxLocal(ResourceName effect, ResourceName soundProject, string soundEvent, vector pos, float lifeS)
	{
		ParticleEffectEntity fx = SRZ_FXRange.Particle(effect, pos, m_fEffectRange);
		if (fx && lifeS > 0)
			GetGame().GetCallqueue().CallLater(SCR_EntityHelper.DeleteEntityAndChildren, lifeS * 1000, false, fx);

		AudioHandle handle = SRZ_FXRange.Sound(soundProject, soundEvent, pos, m_fEffectRange);
		if (lifeS > 0 && handle != AudioHandle.Invalid)
			GetGame().GetCallqueue().CallLater(SRZ_FXRange.StopSound, lifeS * 1000, false, handle);
	}
}
