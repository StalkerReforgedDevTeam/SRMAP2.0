// Dead mutant props (the ones you cut for parts) never despawned, so they piled up all over the map.
// Now every one is removed 5 minutes after the mutant dies, cut or not. A body can only be cut once.
modded class ARMST_USER_LOOT_BODY : ScriptedUserAction
{
	protected static const float SRZ_CORPSE_LIFETIME_MS = 300000;
	static int s_iSRZ_CorpsesAlive;
	protected static ref set<IEntity> s_SRZ_CutCorpses = new set<IEntity>();

	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.Init(pOwnerEntity, pManagerComponent);

		if (!m_ForMutants || !pOwnerEntity || !Replication.IsServer() || !GetGame().InPlayMode())
			return;

		s_iSRZ_CorpsesAlive++;
		GetGame().GetCallqueue().CallLater(SRZ_RemoveCorpse, SRZ_CORPSE_LIFETIME_MS, false, pOwnerEntity.GetRootParent());
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
		if (!root)
			return;

		s_iSRZ_CorpsesAlive--;
		s_SRZ_CutCorpses.RemoveItem(root);
		SCR_EntityHelper.DeleteEntityAndChildren(root);
	}
}
