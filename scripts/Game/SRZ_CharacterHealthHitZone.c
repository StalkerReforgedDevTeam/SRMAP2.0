modded class SCR_CharacterHealthHitZone : SCR_HitZone
{
	[Attribute("1", UIWidgets.Auto, "Multiplier of how toxic damage is applied")]
	protected float m_toxicDamageMultiplier;
	
	[Attribute("1", UIWidgets.Auto, "Multiplier of how radiation damage is applied")]
	protected float m_radiationDamageMultiplier;
	
	override float ComputeEffectiveDamage(notnull BaseDamageContext damageContext, bool isDOT)
	{
		float result = 0;
		
		switch (damageContext.damageType)
		{
			case EDamageType.Radiactive:
			{
				result = damageContext.damageValue * m_radiationDamageMultiplier;
				break;
			}
			case EDamageType.Toxic:
			{
				result = damageContext.damageValue * m_toxicDamageMultiplier;
				break;
			}
			default:
			{
				result = super.ComputeEffectiveDamage(damageContext, isDOT);
			}
		}
		
		return result;
	}
}