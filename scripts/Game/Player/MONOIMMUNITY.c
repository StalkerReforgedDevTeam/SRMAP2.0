modded class ARMST_RADIATIONSEntity
{
	override void OnActivate(IEntity ent)
	{
		if (!ent)
			return;

		if (!IsAlive(ent))
			return;

		if (!EntityUtils.IsPlayer(ent))
			return;

		FactionAffiliationComponent factionComp = FactionAffiliationComponent.Cast(ent.FindComponent(FactionAffiliationComponent));
		if (factionComp)
		{
			Faction faction = factionComp.GetAffiliatedFaction();
			if (faction && faction.GetFactionKey() == "FACTION_MUTANT")
			{
				ARMST_PLAYER_STATS_COMPONENT skipStats = ARMST_PLAYER_STATS_COMPONENT.Cast(ent.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
				if (skipStats)
					skipStats.ArmstRadiactiveLevelSet(0);

				return;
			}
		}

		super.OnActivate(ent);
	}
};