// Contract alphas take less from every hit, set per hunt site. Every other mutant is untouched.
// Sits after SRZ_MutantBehaviourFix.c so the pool damage method already exists.
modded class Bacon_622120A5448725E3_InfectedCharacter
{
	override void SRZ_ApplyPoolDamage(float damage, Instigator instigator)
	{
		super.SRZ_ApplyPoolDamage(damage / SRZ_Contracts.AlphaToughness(this), instigator);
	}
}
