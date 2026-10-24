// Dead mutant bodies and props (the ones you cut for parts) are removed 5 minutes after the mutant dies, cut or not.
// A body can only be cut once. The Telepach keeps its body on death so it can be cut like the others.
modded class ARMST_USER_LOOT_BODY : ScriptedUserAction
{
	protected static const float SRZ_CORPSE_LIFETIME_MS = 300000;
	static int s_iSRZ_CorpsesAlive;
	protected static ref set<IEntity> s_SRZ_CutCorpses = new set<IEntity>();
	protected static ref set<IEntity> s_SRZ_Scheduled = new set<IEntity>();

	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.Init(pOwnerEntity, pManagerComponent);

		if (!m_ForMutants || !pOwnerEntity || !Replication.IsServer() || !GetGame().InPlayMode())
			return;

		// Live mutants carry this action too; their timer starts when they die instead of when they spawn
		ChimeraCharacter character = ChimeraCharacter.Cast(pOwnerEntity.GetRootParent());
		if (!character)
		{
			SRZ_ScheduleCorpse(pOwnerEntity.GetRootParent());
			return;
		}

		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
		if (controller)
			controller.m_OnLifeStateChanged.Insert(SRZ_OnOwnerLifeState);
	}

	//------------------------------------------------------------------------------------------------
	protected void SRZ_OnOwnerLifeState(ECharacterLifeState previousState, ECharacterLifeState newState)
	{
		if (newState == ECharacterLifeState.DEAD && GetOwner())
			SRZ_ScheduleCorpse(GetOwner().GetRootParent());
	}

	//------------------------------------------------------------------------------------------------
	static void SRZ_ScheduleCorpse(IEntity root)
	{
		if (!root || s_SRZ_Scheduled.Contains(root))
			return;

		s_SRZ_Scheduled.Insert(root);
		s_iSRZ_CorpsesAlive++;
		GetGame().GetCallqueue().CallLater(SRZ_RemoveCorpse, SRZ_CORPSE_LIFETIME_MS, false, root);
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		if (m_ForMutants && GetOwner() && s_SRZ_CutCorpses.Contains(GetOwner().GetRootParent()))
			return false;

		return super.CanBePerformedScript(user);
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!m_ForMutants || !pOwnerEntity)
		{
			super.PerformAction(pOwnerEntity, pUserEntity);
			return;
		}

		IEntity root = pOwnerEntity.GetRootParent();
		if (s_SRZ_CutCorpses.Contains(root))
			return;

		s_SRZ_CutCorpses.Insert(root);

		// Body stays until its 5 minutes are up instead of vanishing when cut
		bool deleteBody = m_bDeleteBody;
		m_bDeleteBody = false;
		super.PerformAction(pOwnerEntity, pUserEntity);
		m_bDeleteBody = deleteBody;
	}

	//------------------------------------------------------------------------------------------------
	protected static void SRZ_RemoveCorpse(IEntity root)
	{
		s_iSRZ_CorpsesAlive--;
		if (!root)
			return;

		s_SRZ_CutCorpses.RemoveItem(root);
		s_SRZ_Scheduled.RemoveItem(root);
		SCR_EntityHelper.DeleteEntityAndChildren(root);
	}
}

modded class ARMST_DEATH_COMPONENTS
{
	//------------------------------------------------------------------------------------------------
	override void OnLifeStateChanged(ECharacterLifeState previousState, ECharacterLifeState newState)
	{
		bool dying = newState == ECharacterLifeState.DEAD && previousState != ECharacterLifeState.DEAD && Replication.IsServer();
		bool keep = dying && SRZ_KeepsBody();

		ResourceName spawnOnDeath = m_PrefabToSpawnDeath;
		bool deleteOwner = m_bDeleteOwner;
		if (keep)
		{
			m_PrefabToSpawnDeath = ResourceName.Empty;
			m_bDeleteOwner = false;
		}

		super.OnLifeStateChanged(previousState, newState);

		if (keep)
		{
			m_PrefabToSpawnDeath = spawnOnDeath;
			m_bDeleteOwner = deleteOwner;
			SRZ_RemovePsyFields();
		}
	}

	//------------------------------------------------------------------------------------------------
	// The psy damage zone and defence sphere ride on the body, so they go when it dies
	protected void SRZ_RemovePsyFields()
	{
		IEntity owner = GetOwner();
		if (!owner)
			return;

		IEntity child = owner.GetChildren();
		while (child)
		{
			IEntity next = child.GetSibling();
			if (child.GetPrefabData())
			{
				string path = child.GetPrefabData().GetPrefabName();
				path.ToLower();
				if (path.Contains("damagearea") || path.Contains("sfera_defense"))
					SCR_EntityHelper.DeleteEntityAndChildren(child);
			}

			child = next;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected string SRZ_PrefabPath()
	{
		IEntity owner = GetOwner();
		if (!owner || !owner.GetPrefabData())
			return "";

		string path = owner.GetPrefabData().GetPrefabName();
		path.ToLower();
		return path;
	}

	//------------------------------------------------------------------------------------------------
	protected bool SRZ_KeepsBody()
	{
		return SRZ_PrefabPath().Contains("psy_destroyer");
	}
}
