modded class SCR_PlayerController
{
	override protected void ServerApplyArtefactEffects(
		IEntity characterEntity,
		float toxic,
		float radiactive,
		float psy,
		float health,
		float stamina,
		float water,
		float eat
	)
	{
		if (!characterEntity)
			return;

		ARMST_PLAYER_STATS_COMPONENT statsComponent = ARMST_PLAYER_STATS_COMPONENT.Cast(characterEntity.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
		if (!statsComponent)
			return;

		CharacterStaminaComponent staminaComp = CharacterStaminaComponent.Cast(characterEntity.FindComponent(CharacterStaminaComponent));

		DamageManagerComponent damageManager = DamageManagerComponent.Cast(characterEntity.FindComponent(DamageManagerComponent));

		if (toxic != 0)
			statsComponent.ArmstPlayerStatSetToxic(toxic / 10);

		if (radiactive != 0)
			statsComponent.ArmstPlayerStatSetRadio(radiactive / 10);

		if (psy != 0)
			statsComponent.ArmstPlayerStatSetPsy(psy / 10);

		if (water != 0)
			statsComponent.ArmstPlayerStatSetWater(water / 10);

		if (eat != 0)
			statsComponent.ArmstPlayerStatSetEat(eat / 10);

		if (health != 0 && damageManager)
		{
			SCR_CharacterDamageManagerComponent charDmgMgr = SCR_CharacterDamageManagerComponent.Cast(damageManager);
			if (health > 0 && charDmgMgr)
			{
				charDmgMgr.HealHitZones(health / 20);
			}
			else
			{
				BaseDamageContext damageCtx = new BaseDamageContext();
				damageCtx.damageValue = -health / 20;
				damageCtx.damageType = EDamageType.INCENDIARY;
				damageManager.HandleDamage(damageCtx);
			}
		}

		if (stamina != 0 && staminaComp)
			staminaComp.AddStamina(stamina / 2);
	}
}