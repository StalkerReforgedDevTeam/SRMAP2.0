modded class SCR_CharacterHitZone
{
	override float ComputeEffectiveDamage(notnull BaseDamageContext damageContext, bool isDOT)
	{
		if (damageContext.hitEntity && Bacon_622120A5448725E3_InfectedCharacter.Cast(damageContext.hitEntity))
			return super.ComputeEffectiveDamage(damageContext, isDOT);

		if (IsFriendlyFire(damageContext.instigator, damageContext.hitEntity))
			return 0;

		return super.ComputeEffectiveDamage(damageContext, isDOT);
	}
};