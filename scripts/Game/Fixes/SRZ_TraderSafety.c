// Mutants and zombies are removed when they get within RADIUS of a trader, barman or anyone else you can talk to,
// the same way ARMST safe zones remove wandering AI. Traders outside a safe zone were getting mobbed.
// Only mutants hostile to that trader's faction are removed, so Monolith (mutant faction) traders and their mutants are left alone.
// Only animals and zombies from Prefabs/Monsters count. Monolith soldiers, any human NPC and all players are never touched,
// so a Monolith raid on a trader plays out normally.
class SRZ_TraderSafety
{
	protected static const float RADIUS = 25;
	protected static const float TICK = 3;

	protected static ref SRZ_TraderSafety s_Instance;
	protected ref array<IEntity> m_aTraders = {};
	protected ref array<IEntity> m_aFound = {};
	protected Faction m_TraderFaction;

	//------------------------------------------------------------------------------------------------
	static void Register(IEntity trader)
	{
		if (!trader || !Replication.IsServer())
			return;

		if (!s_Instance)
		{
			s_Instance = new SRZ_TraderSafety();
			GetGame().GetCallqueue().CallLater(s_Instance.Tick, TICK * 1000, true);
		}

		if (!s_Instance.m_aTraders.Contains(trader))
			s_Instance.m_aTraders.Insert(trader);
	}

	//------------------------------------------------------------------------------------------------
	protected void Tick()
	{
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return;

		for (int i = m_aTraders.Count() - 1; i >= 0; i--)
		{
			IEntity trader = m_aTraders[i];
			if (!trader)
			{
				m_aTraders.Remove(i);
				continue;
			}

			m_aFound.Clear();
			m_TraderFaction = FactionOf(trader);
			world.QueryEntitiesBySphere(trader.GetOrigin(), RADIUS, Collect, null, EQueryEntitiesFlags.DYNAMIC);
			foreach (IEntity mutant : m_aFound)
			{
				if (mutant)
					SCR_EntityHelper.DeleteEntityAndChildren(mutant);
			}
		}

		m_aFound.Clear();
	}

	//------------------------------------------------------------------------------------------------
	protected Faction FactionOf(IEntity ent)
	{
		if (!ent)
			return null;

		FactionAffiliationComponent aff = FactionAffiliationComponent.Cast(ent.FindComponent(FactionAffiliationComponent));
		if (!aff)
			return null;

		return aff.GetAffiliatedFaction();
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsMonster(IEntity ent)
	{
		EntityPrefabData data = ent.GetPrefabData();
		if (!data)
			return false;

		string path = data.GetPrefabName();
		path.ToLower();
		return path.Contains("prefabs/monsters/");
	}

	//------------------------------------------------------------------------------------------------
	protected bool Collect(IEntity ent)
	{
		Bacon_622120A5448725E3_InfectedCharacter mutant = Bacon_622120A5448725E3_InfectedCharacter.Cast(ent);
		if (!mutant || EntityUtils.IsPlayer(mutant) || m_aFound.Contains(mutant) || mutant.FindComponent(ARMST_DIALOGS_COMPONENT))
			return true;

		if (!IsMonster(mutant))
			return true;

		CharacterControllerComponent controller = mutant.GetCharacterController();
		if (controller && controller.IsDead())
			return true;

		// Friendly to this trader (same faction or allied), so it belongs here
		Faction mutantFaction = FactionOf(mutant);
		if (m_TraderFaction && mutantFaction && (mutantFaction == m_TraderFaction || m_TraderFaction.IsFactionFriendly(mutantFaction)))
			return true;

		m_aFound.Insert(mutant);
		return true;
	}
}

modded class ARMST_DIALOGS_COMPONENT
{
	override protected void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		if (GetGame().InPlayMode() && !SCR_Global.IsEditMode())
			SRZ_TraderSafety.Register(owner);
	}
}
