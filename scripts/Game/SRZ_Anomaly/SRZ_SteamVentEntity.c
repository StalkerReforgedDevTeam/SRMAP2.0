class SRZ_SteamVentEntityClass : ARMST_DamagingTriggerEntityClass
{
}

class SRZ_SteamVictim
{
	IEntity m_Entity;
	float m_fNextHit;
}

class SRZ_SteamVentEntity : ARMST_DamagingTriggerEntity
{
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Steam burst effect, follows the prefab's rotation so point it out of the pipe", category: "7. Steam", params: "ptc")]
	ResourceName m_SteamEffect;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Faint steam that always leaks out, so players can spot the vent", category: "7. Steam", params: "ptc")]
	ResourceName m_IdleEffect;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Sputtering steam during the warning before a burst", category: "7. Steam", params: "ptc")]
	ResourceName m_WarnEffect;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Sound project for the hiss and the burst", category: "7. Steam", params: "acp")]
	ResourceName m_SoundProject;

	[Attribute("", UIWidgets.EditBox, "Warning hiss before a burst", category: "7. Steam")]
	string m_sWarnSoundEvent;

	[Attribute("", UIWidgets.EditBox, "Sound of the burst", category: "7. Steam")]
	string m_sBurstSoundEvent;

	[Attribute("8", UIWidgets.Slider, "Shortest wait between bursts, seconds", "1 120 1", category: "7. Steam")]
	float m_fWaitMin;

	[Attribute("20", UIWidgets.Slider, "Longest wait between bursts, seconds", "1 120 1", category: "7. Steam")]
	float m_fWaitMax;

	[Attribute("2", UIWidgets.Slider, "Seconds of hiss before the burst, the time to get clear", "0 5 0.1", category: "7. Steam")]
	float m_fWarnSeconds;

	[Attribute("6", UIWidgets.Slider, "How long a burst lasts, seconds", "0.5 15 0.5", category: "7. Steam")]
	float m_fBurstSeconds;

	[Attribute("0.5", UIWidgets.Slider, "Seconds between damage ticks during a burst. Damage per tick is the normal Damage Value", "0.25 5 0.25", category: "7. Steam")]
	float m_fDamageInterval;

	[Attribute("100", UIWidgets.Slider, "Effects and sounds play for players within this range", "20 500 10", category: "7. Steam")]
	float m_fEffectRange;

	protected bool m_bBursting;
	protected ParticleEffectEntity m_Idle;
	protected ref array<ref SRZ_SteamVictim> m_aVictims = {};

	override void OnInit(IEntity owner)
	{
		super.OnInit(owner);
		if (!GetGame().InPlayMode())
			return;

		if (!SRZ_FXRange.IsHeadless() && !m_IdleEffect.IsEmpty())
			m_Idle = SpawnAttached(m_IdleEffect);

		if (!Replication.IsServer())
			return;

		GetGame().GetCallqueue().CallLater(Warn, Wait() * 1000, false);
	}

	void ~SRZ_SteamVentEntity()
	{
		if (!GetGame() || !GetGame().GetCallqueue())
			return;

		GetGame().GetCallqueue().Remove(Warn);
		GetGame().GetCallqueue().Remove(Burst);
		GetGame().GetCallqueue().Remove(EndBurst);
	}

	override void OnActivate(IEntity ent)
	{
		if (!m_bBursting || !Replication.IsServer() || !ent || !IsAlive(ent))
			return;

		ChimeraCharacter character = ChimeraCharacter.Cast(ent);
		if (!character || character.IsInVehicle())
			return;

		if (!m_fMonsterDamage && !EntityUtils.IsPlayer(ent))
			return;

		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller || controller.GetLifeState() == ECharacterLifeState.DEAD)
			return;

		SRZ_SteamVictim victim = FindVictim(ent);
		float now = Now();
		if (now < victim.m_fNextHit)
			return;

		victim.m_fNextHit = now + m_fDamageInterval;
		ArmstDamageStart(ent);
	}

	protected float Wait()
	{
		return Math.RandomFloat(Math.Min(m_fWaitMin, m_fWaitMax), Math.Max(m_fWaitMin, m_fWaitMax));
	}

	protected float Now()
	{
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return 0;

		return world.GetWorldTime() * 0.001;
	}

	protected SRZ_SteamVictim FindVictim(IEntity ent)
	{
		for (int i = m_aVictims.Count() - 1; i >= 0; i--)
		{
			if (!m_aVictims[i].m_Entity || m_aVictims[i].m_Entity.IsDeleted())
			{
				m_aVictims.Remove(i);
				continue;
			}

			if (m_aVictims[i].m_Entity == ent)
				return m_aVictims[i];
		}

		SRZ_SteamVictim victim = new SRZ_SteamVictim();
		victim.m_Entity = ent;
		m_aVictims.Insert(victim);
		return victim;
	}

	protected void Warn()
	{
		WarnLocal();
		Rpc(RpcDo_Warn);
		GetGame().GetCallqueue().CallLater(Burst, m_fWarnSeconds * 1000, false);
	}

	protected void Burst()
	{
		m_bBursting = true;
		BurstLocal(m_fBurstSeconds);
		Rpc(RpcDo_Burst, m_fBurstSeconds);
		GetGame().GetCallqueue().CallLater(EndBurst, m_fBurstSeconds * 1000, false);
	}

	protected void EndBurst()
	{
		m_bBursting = false;
		m_aVictims.Clear();
		GetGame().GetCallqueue().CallLater(Warn, Wait() * 1000, false);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_Warn()
	{
		WarnLocal();
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_Burst(float seconds)
	{
		BurstLocal(seconds);
	}

	protected void WarnLocal()
	{
		SRZ_FXRange.Sound(m_SoundProject, m_sWarnSoundEvent, GetOrigin(), m_fEffectRange);
		if (m_WarnEffect.IsEmpty() || !SRZ_FXRange.InRange(GetOrigin(), m_fEffectRange))
			return;

		ParticleEffectEntity fx = SpawnAttached(m_WarnEffect);
		if (fx)
			GetGame().GetCallqueue().CallLater(SCR_EntityHelper.DeleteEntityAndChildren, (m_fWarnSeconds + 0.3) * 1000, false, fx);
	}

	protected ParticleEffectEntity SpawnAttached(ResourceName effect)
	{
		ParticleEffectEntitySpawnParams params = new ParticleEffectEntitySpawnParams();
		params.Parent = this;
		return ParticleEffectEntity.SpawnParticleEffect(effect, params);
	}

	protected void BurstLocal(float seconds)
	{
		AudioHandle handle = SRZ_FXRange.Sound(m_SoundProject, m_sBurstSoundEvent, GetOrigin(), m_fEffectRange);
		if (handle != AudioHandle.Invalid)
			GetGame().GetCallqueue().CallLater(SRZ_FXRange.StopSound, seconds * 1000, false, handle);

		if (m_SteamEffect.IsEmpty() || !SRZ_FXRange.InRange(GetOrigin(), m_fEffectRange))
			return;

		ParticleEffectEntity fx = SpawnAttached(m_SteamEffect);
		if (fx)
			GetGame().GetCallqueue().CallLater(SCR_EntityHelper.DeleteEntityAndChildren, seconds * 1000, false, fx);
	}
}
