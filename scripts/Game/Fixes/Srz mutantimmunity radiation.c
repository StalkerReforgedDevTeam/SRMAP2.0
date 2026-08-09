// Scripts/Game/SRZ_MutantImmunity/SRZ_MutantImmunity_Radiation.c
// Mutants are immune to radiation from ARMST_RADIATIONSEntity
modded class ARMST_RADIATIONSEntity
{
	override void OnActivate(IEntity ent)
	{
		if (!ent)
			return;
		if (!IsAlive(ent))
			return;
		SCR_ChimeraCharacter owner2 = SCR_ChimeraCharacter.Cast(ent);
		if (!owner2)
			return;
		CharacterControllerComponent contr = owner2.GetCharacterController();
		if (!contr)
			return;
		if (contr.GetLifeState() == ECharacterLifeState.DEAD)
			return;
		if (!EntityUtils.IsPlayer(ent))
			return;
		float levelrad = armst_distance_level(ent);
		float m_fProtectionSumm = 0;
		ARMST_ITEMS_STATS_COMPONENTS StatComponent = ARMST_ITEMS_STATS_COMPONENTS.Cast(ent.FindComponent(ARMST_ITEMS_STATS_COMPONENTS));
		if (!StatComponent)
			return;
		m_fProtectionSumm = StatComponent.GetAllRadiactive(ent);
		ARMST_PLAYER_STATS_COMPONENT statsComponent = ARMST_PLAYER_STATS_COMPONENT.Cast(ent.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
		if (!statsComponent)
			return;
		// --- MUTANT IMMUNITY CHECK ---
		if (statsComponent.GetFactionKey() == ARMST_FACTION_LABEL.FACTION_MUTANTS)
		{
			// Keep their "radiation around player" level clean too
			statsComponent.ArmstRadiactiveLevelSet(0);
			return; // immune
		}
		// -----------------------------
		statsComponent.ArmstRadiactiveLevelSet(levelrad);
		if (m_fProtectionSumm > levelrad)
			return;
		statsComponent.ArmstPlayerStatSetRadio(levelrad / 10);
	}
	override void OnDeactivate(IEntity ent)
	{
		if (!ent)
			return;
		ARMST_PLAYER_STATS_COMPONENT statsComponent = ARMST_PLAYER_STATS_COMPONENT.Cast(ent.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
		if (!statsComponent)
			return;
		// Always clear radiation level on exit (same as original)
		statsComponent.ArmstRadiactiveLevelSet(0);
	}
};