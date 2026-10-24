// Players were dying from short falls. Fall and impact damage to players on foot is scaled down here.
// Raise or lower SRZ_FALL_DAMAGE_SCALE to tune it (1 = vanilla, 0.3 = 30% of vanilla).
modded class SCR_CharacterHitZone
{
	protected static const float SRZ_FALL_DAMAGE_SCALE = 0.3;

	//------------------------------------------------------------------------------------------------
	override float ComputeEffectiveDamage(notnull BaseDamageContext damageContext, bool isDOT)
	{
		float damage = super.ComputeEffectiveDamage(damageContext, isDOT);
		if (damage <= 0 || damageContext.damageType != EDamageType.COLLISION)
			return damage;

		ChimeraCharacter character = ChimeraCharacter.Cast(damageContext.hitEntity);
		if (!character || character.IsInVehicle() || !EntityUtils.IsPlayer(character))
			return damage;

		return damage * SRZ_FALL_DAMAGE_SCALE;
	}
}
