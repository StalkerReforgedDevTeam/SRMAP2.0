// Mutant spawner rework (replaces the old null fix, which is included here).
// Checks every 120 s. Spawns when a player is within 300 m and none within 35 m.
// A wiped group locks the spawner for 30 min. A group despawned because nobody is around has no lock.
// Night spawners with no night groups use the day groups.

modded class ARMST_ALIFE_SPAWNGROUPS : ScriptComponent
{
	protected static const int SRZ_CHECK_INTERVAL = 120;
	protected static const int SRZ_CANCEL_RADIUS = 35;
	protected static const int SRZ_DETECTION_RADIUS = 300;
	protected static const float SRZ_GROUP_PLAYER_RADIUS = 200;
	protected static const float SRZ_WIPE_LOCK_MS = 1800000;

	protected bool m_bSRZ_Spawned;
	protected bool m_bSRZ_SawAlive;
	protected float m_fSRZ_LockUntil;
	protected IEntity m_SRZ_Waypoint;

	override void EOnInit(IEntity owner)
	{
		m_CheckInterval = SRZ_CHECK_INTERVAL;
		m_CheckPlayerRadius = SRZ_CANCEL_RADIUS;
		m_PlayerDetectionRadius = SRZ_DETECTION_RADIUS;

		super.EOnInit(owner);
	}

	override void OnDelete(IEntity owner)
	{
		super.OnDelete(owner);
		GetGame().GetCallqueue().Remove(SRZ_MarkAlive);
	}

	override void CheckGroupStatus()
	{
		if (!m_IsSpawnerActive || !Replication.IsServer() || !ALife || !ALife.m_AlifeMonstersEnable)
			return;

		if (!IsAlifeMonstersEnabled())
			return;

		if (m_bSRZ_Spawned)
		{
			IEntity alive = SRZ_GetAliveMember();
			if (!alive)
			{
				if (m_bSRZ_SawAlive)
					m_fSRZ_LockUntil = GetGame().GetWorld().GetWorldTime() + SRZ_WIPE_LOCK_MS;

				SRZ_ClearGroup(false);
			}
			else if (!SRZ_PlayerWithin(GetOwner().GetOrigin(), m_PlayerDetectionRadius) && !SRZ_PlayerWithin(alive.GetOrigin(), SRZ_GROUP_PLAYER_RADIUS))
			{
				SRZ_ClearGroup(true);
			}
		}
		else if (GetGame().GetWorld().GetWorldTime() >= m_fSRZ_LockUntil && CheckSpawnChance())
		{
			AlifeSpawn();
		}

		GetGame().GetCallqueue().CallLater(CheckGroupStatus, SRZ_CHECK_INTERVAL * 1000, false);
	}

	override void DeleteGroup()
	{
		SRZ_ClearGroup(true);
	}

	override void AlifeSpawn()
	{
		if (!m_IsSpawnerActive || !ALife || !ALife.m_AlifeMonstersEnable || m_bSRZ_Spawned)
			return;

		vector origin = GetOwner().GetOrigin();
		if (!SRZ_PlayerWithin(origin, m_PlayerDetectionRadius))
			return;

		if (SRZ_PlayerWithin(origin, m_CheckPlayerRadius))
			return;

		if (m_fMonsterNight && SRZ_IsNight())
		{
			if (SRZ_SpawnFromList(m_SpawnGroupsNight))
				return;
		}

		SRZ_SpawnFromList(m_SpawnGroups);
	}

	override void SpawnGroup(Resource lootResource)
	{
		if (!m_IsSpawnerActive || !lootResource || !lootResource.IsValid())
			return;

		EntitySpawnParams params = new EntitySpawnParams();
		params.Transform[3] = GetOwner().GetOrigin();
		params.TransformMode = ETransformMode.WORLD;

		SCR_AIGroup group = SCR_AIGroup.Cast(GetGame().SpawnEntityPrefab(lootResource, null, params));
		if (!group)
			return;

		m_Group = group;
		owners = group;
		m_bSRZ_Spawned = true;
		m_bSRZ_SawAlive = false;

		Resource wpResource = Resource.Load(m_WaypointPrefab);
		if (wpResource && wpResource.IsValid())
		{
			IEntity wpEntity = GetGame().SpawnEntityPrefab(wpResource, GetGame().GetWorld(), params);
			AIWaypoint wp = AIWaypoint.Cast(wpEntity);
			if (wp)
			{
				wp.SetCompletionRadius(m_RadiusPatrol);
				m_Group.AddWaypoint(wp);
				m_SRZ_Waypoint = wpEntity;
			}
			else if (wpEntity)
			{
				SCR_EntityHelper.DeleteEntityAndChildren(wpEntity);
			}
		}

		GetGame().GetCallqueue().CallLater(SRZ_MarkAlive, 5000, false);
	}

	// Members spawn a moment after the group, so mark the group as real once they exist
	protected void SRZ_MarkAlive()
	{
		if (m_bSRZ_Spawned && SRZ_GetAliveMember())
			m_bSRZ_SawAlive = true;
	}

	protected bool SRZ_SpawnFromList(array<ref ResourceName> configs)
	{
		if (!configs)
			return false;

		array<ref ARMST_PLAYER_START_CONF_DATA> items = {};
		foreach (ResourceName configResource : configs)
		{
			if (configResource.IsEmpty())
				continue;

			Resource resource = BaseContainerTools.LoadContainer(configResource);
			if (!resource || !resource.IsValid())
				continue;

			BaseContainer container = resource.GetResource().ToBaseContainer();
			if (!container)
				continue;

			ARMST_PLAYER_START_CONF_Config config = ARMST_PLAYER_START_CONF_Config.Cast(BaseContainerTools.CreateInstanceFromContainer(container));
			if (!config || !config.m_WikiData)
				continue;

			foreach (ARMST_PLAYER_START_CONF_DATA entry : config.m_WikiData)
			{
				if (entry && !entry.m_PrefabTrader.IsEmpty())
					items.Insert(entry);
			}
		}

		if (items.IsEmpty())
			return false;

		Resource groupResource = Resource.Load(items[Math.RandomInt(0, items.Count())].m_PrefabTrader);
		if (!groupResource || !groupResource.IsValid())
			return false;

		SpawnGroup(groupResource);
		return m_bSRZ_Spawned;
	}

	protected IEntity SRZ_GetAliveMember()
	{
		if (!m_Group)
			return null;

		array<AIAgent> agents = {};
		m_Group.GetAgents(agents);
		foreach (AIAgent agent : agents)
		{
			if (!agent)
				continue;

			SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(agent.GetControlledEntity());
			if (!character)
				continue;

			CharacterControllerComponent controller = character.GetCharacterController();
			if (controller && controller.GetLifeState() != ECharacterLifeState.DEAD)
				return character;
		}

		return null;
	}

	// Despawn removes living members too. A wipe leaves the bodies for looting.
	protected void SRZ_ClearGroup(bool removeMembers)
	{
		GetGame().GetCallqueue().Remove(SRZ_MarkAlive);

		if (m_Group && removeMembers)
		{
			array<AIAgent> agents = {};
			m_Group.GetAgents(agents);
			foreach (AIAgent agent : agents)
			{
				if (!agent)
					continue;

				IEntity member = agent.GetControlledEntity();
				if (member)
					SCR_EntityHelper.DeleteEntityAndChildren(member);
			}
		}

		if (owners)
			SCR_EntityHelper.DeleteEntityAndChildren(owners);

		if (m_SRZ_Waypoint)
			SCR_EntityHelper.DeleteEntityAndChildren(m_SRZ_Waypoint);

		owners = null;
		m_Group = null;
		m_SRZ_Waypoint = null;
		m_bSRZ_Spawned = false;
		m_bSRZ_SawAlive = false;
		m_CanRespawn = true;
	}

	protected bool SRZ_PlayerWithin(vector pos, float radius)
	{
		playersNearby = false;
		GetGame().GetWorld().QueryEntitiesBySphere(pos, radius, CheckForPlayers);
		return playersNearby;
	}

	protected bool SRZ_IsNight()
	{
		ChimeraWorld world = ChimeraWorld.CastFrom(GetGame().GetWorld());
		if (!world)
			return false;

		m_TimeMgr = world.GetTimeAndWeatherManager();
		if (!m_TimeMgr)
			return false;

		m_TimeMgr.GetHoursMinutesSeconds(m_iHours, m_iMinutes, m_iSeconds);
		return m_iHours < 6;
	}
}
