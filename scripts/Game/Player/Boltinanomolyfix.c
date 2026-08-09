// Extends ARMST anomaly triggers so bolt-like throwables (bolt, chip, bottles, bricks, bullet, etc.):
// 1) Play type-specific bolt-hit VFX/sound on all clients
// 2) Fully despawn the anomaly (removes rock/collision leftovers)
// 3) Respawn after a random delay (default 10-60s)
//
// VFX/sound are broadcast to clients via GameMode RPC. Dedicated servers do not render particles.

class BTR_AnomalyRespawnPayload
{
	ResourceName m_Prefab;
	vector m_vMat[4];
}

class BTR_AnomalyBoltFxHelper
{
	protected static const ResourceName BTR_FALLBACK_BOLT_FX = "{A1292362DBE60921}Prefabs/Anomals/anom_base/anomal_bolt_actions.ptc";
	protected static const ResourceName BTR_FALLBACK_BLAST_FX = "{52CF1B2D3E1DC41C}Prefabs/Anomals/anom_base/data/anomal_blast.ptc";

	//------------------------------------------------------------------------------------------------
	static void Play(vector mat[4], ResourceName particleBolt, ResourceName particleAttack, ResourceName particleAttack2, ResourceName soundProjectBolt, ResourceName soundProjectAttack, string soundEventBolt, string soundEventAttack)
	{
		PlayParticles(mat, particleBolt, particleAttack, particleAttack2);
		PlaySounds(mat, soundProjectBolt, soundProjectAttack, soundEventBolt, soundEventAttack);
	}

	//------------------------------------------------------------------------------------------------
	static void PlayParticles(vector mat[4], ResourceName particleBolt, ResourceName particleAttack, ResourceName particleAttack2)
	{
		bool spawnedFx;

		spawnedFx |= SpawnParticleAt(particleBolt, mat);

		if (particleAttack != ResourceName.Empty && particleAttack != particleBolt)
			spawnedFx |= SpawnParticleAt(particleAttack, mat);

		if (particleAttack2 != ResourceName.Empty && particleAttack2 != particleBolt && particleAttack2 != particleAttack)
			spawnedFx |= SpawnParticleAt(particleAttack2, mat);

		if (!spawnedFx)
		{
			SpawnParticleAt(BTR_FALLBACK_BOLT_FX, mat);
			SpawnParticleAt(BTR_FALLBACK_BLAST_FX, mat);
		}
	}

	//------------------------------------------------------------------------------------------------
	static void PlaySounds(vector mat[4], ResourceName soundProjectBolt, ResourceName soundProjectAttack, string soundEventBolt, string soundEventAttack)
	{
		PlaySoundAt(mat, soundProjectBolt, soundEventBolt);
		if (soundProjectAttack != ResourceName.Empty && (soundProjectBolt != soundProjectAttack || soundEventBolt != soundEventAttack))
			PlaySoundAt(mat, soundProjectAttack, soundEventAttack);
	}

	//------------------------------------------------------------------------------------------------
	protected static bool SpawnParticleAt(ResourceName particle, vector mat[4])
	{
		if (particle == ResourceName.Empty)
			return false;

		ParticleEffectEntitySpawnParams spawnParams = new ParticleEffectEntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;
		spawnParams.Transform[0] = mat[0];
		spawnParams.Transform[1] = mat[1];
		spawnParams.Transform[2] = mat[2];
		spawnParams.Transform[3] = mat[3];
		spawnParams.PlayOnSpawn = true;
		spawnParams.DeleteWhenStopped = true;
		return ParticleEffectEntity.SpawnParticleEffect(particle, spawnParams) != null;
	}

	//------------------------------------------------------------------------------------------------
	protected static void PlaySoundAt(vector mat[4], ResourceName soundProject, string soundEvent)
	{
		if (soundProject == ResourceName.Empty || soundEvent == string.Empty)
			return;

		SCR_AudioSourceConfiguration config = new SCR_AudioSourceConfiguration();
		config.m_sSoundProject = soundProject;
		config.m_sSoundEventName = soundEvent;

		SCR_SoundManagerModule soundModule = SCR_SoundManagerModule.GetInstance(GetGame().GetWorld());
		if (!soundModule)
			return;

		SCR_AudioSource source = soundModule.CreateAudioSource(config, mat[3]);
		if (source)
			source.Play();
	}
}

modded class SCR_BaseGameMode
{
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcBTR_PlayAnomalyBoltParticles(vector m0, vector m1, vector m2, vector m3, ResourceName particleBolt, ResourceName particleAttack, ResourceName particleAttack2)
	{
		vector mat[4];
		mat[0] = m0;
		mat[1] = m1;
		mat[2] = m2;
		mat[3] = m3;
		BTR_AnomalyBoltFxHelper.PlayParticles(mat, particleBolt, particleAttack, particleAttack2);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcBTR_PlayAnomalyBoltSounds(vector m0, vector m1, vector m2, vector m3, ResourceName soundProjectBolt, ResourceName soundProjectAttack, string soundEventBolt, string soundEventAttack)
	{
		vector mat[4];
		mat[0] = m0;
		mat[1] = m1;
		mat[2] = m2;
		mat[3] = m3;
		BTR_AnomalyBoltFxHelper.PlaySounds(mat, soundProjectBolt, soundProjectAttack, soundEventBolt, soundEventAttack);
	}

	//------------------------------------------------------------------------------------------------
	void BTR_BroadcastAnomalyBoltEffects(vector mat[4], ResourceName particleBolt, ResourceName particleAttack, ResourceName particleAttack2, ResourceName soundProjectBolt, ResourceName soundProjectAttack, string soundEventBolt, string soundEventAttack)
	{
		Rpc(RpcBTR_PlayAnomalyBoltParticles, mat[0], mat[1], mat[2], mat[3], particleBolt, particleAttack, particleAttack2);
		Rpc(RpcBTR_PlayAnomalyBoltSounds, mat[0], mat[1], mat[2], mat[3], soundProjectBolt, soundProjectAttack, soundEventBolt, soundEventAttack);

		// Broadcast RPC does not run on the authority. Play locally on listen-server hosts.
		if (GetGame().GetPlayerController())
			BTR_AnomalyBoltFxHelper.Play(mat, particleBolt, particleAttack, particleAttack2, soundProjectBolt, soundProjectAttack, soundEventBolt, soundEventAttack);
	}
}

modded class ARMST_DamagingTriggerEntity
{
	protected static const string BTR_THROWABLE_PATH_TOKEN = "armst_throw_";

	[Attribute("10", UIWidgets.EditBox, desc: "Minimum respawn delay after a bolt hit (seconds)", params: "1 600 1", category: "BTR Bolt Respawn")]
	protected int m_iBTR_RespawnMinSec;

	[Attribute("60", UIWidgets.EditBox, desc: "Maximum respawn delay after a bolt hit (seconds)", params: "1 600 1", category: "BTR Bolt Respawn")]
	protected int m_iBTR_RespawnMaxSec;

	protected bool m_bBTR_RespawnScheduled;

	//------------------------------------------------------------------------------------------------
	override bool ScriptedEntityFilterForQuery(IEntity ent)
	{
		if (BTR_IsThrowableProbe(ent))
			return true;

		return super.ScriptedEntityFilterForQuery(ent);
	}

	//------------------------------------------------------------------------------------------------
	override protected event void OnActivate(IEntity ent)
	{
		if (BTR_IsThrowableProbe(ent))
		{
			if (!Replication.IsServer())
				return;

			if (m_bBTR_RespawnScheduled)
				return;

			IEntity anomalyRoot = BTR_GetAnomalyRoot();
			if (!anomalyRoot)
				return;

			EntityPrefabData prefabData = anomalyRoot.GetPrefabData();
			if (!prefabData)
				return;

			ResourceName prefabName = prefabData.GetPrefabName();
			if (prefabName == ResourceName.Empty)
				return;

			vector mat[4];
			anomalyRoot.GetTransform(mat);

			BTR_BroadcastBoltEffects(mat);
			BTR_ScheduleRespawn(prefabName, mat);
			BTR_DespawnAnomalyRoot(anomalyRoot);
			return;
		}

		super.OnActivate(ent);
	}

	//------------------------------------------------------------------------------------------------
	protected bool BTR_IsThrowableProbe(IEntity ent)
	{
		if (!ent)
			return false;

		EntityPrefabData prefabData = ent.GetPrefabData();
		if (!prefabData)
			return false;

		ResourceName prefabName = prefabData.GetPrefabName();
		if (prefabName == ResourceName.Empty)
			return false;

		string prefabPath = prefabName;
		return prefabPath.IndexOf(BTR_THROWABLE_PATH_TOKEN) != -1;
	}

	//------------------------------------------------------------------------------------------------
	protected IEntity BTR_GetAnomalyRoot()
	{
		IEntity parent = GetParent();
		if (parent)
			return parent;

		return this;
	}

	//------------------------------------------------------------------------------------------------
	protected void BTR_BroadcastBoltEffects(vector mat[4])
	{
		ResourceName soundProjectBolt = ResourceName.Empty;
		ResourceName soundProjectAttack = ResourceName.Empty;
		string soundEventBolt = string.Empty;
		string soundEventAttack = string.Empty;

		if (m_AudioSourceConfiguration2 && m_AudioSourceConfiguration2.IsValid())
		{
			soundProjectBolt = m_AudioSourceConfiguration2.m_sSoundProject;
			soundEventBolt = m_AudioSourceConfiguration2.m_sSoundEventName;
		}

		if (m_AudioSourceConfiguration && m_AudioSourceConfiguration.IsValid())
		{
			soundProjectAttack = m_AudioSourceConfiguration.m_sSoundProject;
			soundEventAttack = m_AudioSourceConfiguration.m_sSoundEventName;
		}

		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
			return;

		gameMode.BTR_BroadcastAnomalyBoltEffects(mat, m_sParticle_Bolt, m_sParticle_Attack, m_sParticle_Attack2, soundProjectBolt, soundProjectAttack, soundEventBolt, soundEventAttack);
	}

	//------------------------------------------------------------------------------------------------
	protected void BTR_DespawnAnomalyRoot(IEntity anomalyRoot)
	{
		if (!anomalyRoot)
			return;

		ParticleEffectEntity particleFx = ParticleEffectEntity.Cast(anomalyRoot);
		if (particleFx)
			particleFx.Stop();

		SCR_SoundManagerModule soundModule = SCR_SoundManagerModule.GetInstance(GetGame().GetWorld());
		if (soundModule)
			soundModule.TerminateAudioSource(anomalyRoot);

		SCR_EntityHelper.DeleteEntityAndChildren(anomalyRoot);
	}

	//------------------------------------------------------------------------------------------------
	protected void BTR_ScheduleRespawn(ResourceName prefabName, vector mat[4])
	{
		int minSec = m_iBTR_RespawnMinSec;
		int maxSec = m_iBTR_RespawnMaxSec;

		if (minSec < 1)
			minSec = 1;

		if (maxSec < minSec)
			maxSec = minSec;

		BTR_AnomalyRespawnPayload payload = new BTR_AnomalyRespawnPayload();
		payload.m_Prefab = prefabName;
		payload.m_vMat[0] = mat[0];
		payload.m_vMat[1] = mat[1];
		payload.m_vMat[2] = mat[2];
		payload.m_vMat[3] = mat[3];

		int delaySec = Math.RandomIntInclusive(minSec, maxSec);
		m_bBTR_RespawnScheduled = true;
		GetGame().GetCallqueue().CallLater(BTR_DoRespawnPayload, delaySec * 1000, false, payload);
	}

	//------------------------------------------------------------------------------------------------
	protected static void BTR_DoRespawnPayload(BTR_AnomalyRespawnPayload payload)
	{
		if (!payload || !Replication.IsServer())
			return;

		BTR_SpawnAnomalyPrefab(payload.m_Prefab, payload.m_vMat);
	}

	//------------------------------------------------------------------------------------------------
	protected static IEntity BTR_SpawnAnomalyPrefab(ResourceName prefabName, vector mat[4])
	{
		Resource prefab = Resource.Load(prefabName);
		if (!prefab)
			return null;

		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;
		spawnParams.Transform[0] = mat[0];
		spawnParams.Transform[1] = mat[1];
		spawnParams.Transform[2] = mat[2];
		spawnParams.Transform[3] = mat[3];

		return GetGame().SpawnEntityPrefab(prefab, GetGame().GetWorld(), spawnParams);
	}
}