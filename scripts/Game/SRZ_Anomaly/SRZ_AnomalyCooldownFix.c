modded class ARMST_DamagingTriggerEntity
{
	[Attribute("0", UIWidgets.CheckBox, "Make Attack Timer a real cooldown: after an attack it waits that many seconds before it can attack again", category: "SRZ")]
	bool m_bSRZ_UseCooldown;

	protected float m_fSRZ_NextAttack;

	protected float SRZ_Now()
	{
		BaseWorld world = GetWorld();
		if (!world)
			return 0;

		return world.GetWorldTime() * 0.001;
	}

	override void OnActivate(IEntity ent)
	{
		if (m_bSRZ_UseCooldown && ChimeraCharacter.Cast(ent) && SRZ_Now() < m_fSRZ_NextAttack)
			return;

		super.OnActivate(ent);
	}

	override void SpawnAttackEffects()
	{
		super.SpawnAttackEffects();
		if (m_bSRZ_UseCooldown)
			m_fSRZ_NextAttack = SRZ_Now() + m_Attack_Timer;
	}
}
