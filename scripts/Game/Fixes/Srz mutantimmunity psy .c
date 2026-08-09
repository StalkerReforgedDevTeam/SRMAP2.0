// Scripts/Game/SRZ_MutantImmunity/SRZ_MutantImmunity_Psy.c
// Mutants are immune to psy damage from ARMST_TRIGGER_PSY
modded class ARMST_TRIGGER_PSY
{
	override void OnActivate(IEntity ent)
	{
		if (!ent)
			return;
		// alive?
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
		// --- MUTANT IMMUNITY CHECK ---
		ARMST_PLAYER_STATS_COMPONENT statsComponent = ARMST_PLAYER_STATS_COMPONENT.Cast(ent.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
		if (!statsComponent)
			return;
		if (statsComponent.GetFactionKey() == ARMST_FACTION_LABEL.FACTION_MUTANTS)
			return; // immune
		// -----------------------------
		float levelrad = m_DamagePsyLevel;
		float m_fProtectionSumm = 0;
		ARMST_ITEMS_STATS_COMPONENTS StatComponent = ARMST_ITEMS_STATS_COMPONENTS.Cast(ent.FindComponent(ARMST_ITEMS_STATS_COMPONENTS));
		if (!StatComponent)
			return;
		m_fProtectionSumm = StatComponent.GetAllPsy(ent);
		if (m_fProtectionSumm > levelrad)
			return;
		statsComponent.ArmstPlayerStatSetPsy(-levelrad);
		// camera shake stuff (kept as-is)
		if (Replication.IsServer())
		{
			SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
			if (gameMode && gameMode.IsHosted())
			{
				SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(ent);
				if (character)
					character.ArmstCameraShake(1.5, 3.2, 1, 2, 3);
			}
		}
		else
		{
			SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(ent);
			if (character)
				character.ArmstCameraShake(1.5, 2.6, 0.5, 1, 2);
		}
	}
};