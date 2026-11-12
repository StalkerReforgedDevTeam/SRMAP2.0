// Health for the HUD comes from the server. The client's own copy of the health hit zone could read 0 after a hit
// while the player was still well alive, and stayed there until something healed them.
[BaseContainerProps()]
modded class ARMST_PLAYER_STATS_COMPONENT : ScriptComponent
{
	[RplProp()]
	protected float m_fSRZHealthPct = 100;

	protected static const int SRZ_HEALTH_TICK = 500;

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		if (!Replication.IsServer() || !ChimeraCharacter.Cast(owner))
			return;

		GetGame().GetCallqueue().Remove(SRZ_HealthTick);
		GetGame().GetCallqueue().CallLater(SRZ_HealthTick, SRZ_HEALTH_TICK, true);
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		GetGame().GetCallqueue().Remove(SRZ_HealthTick);
		super.OnDelete(owner);
	}

	//------------------------------------------------------------------------------------------------
	protected float SRZ_ReadHealth()
	{
		IEntity owner = GetOwner();
		if (!owner)
			return 0;

		DamageManagerComponent damageManager = DamageManagerComponent.Cast(owner.FindComponent(DamageManagerComponent));
		if (!damageManager)
			return 0;

		float maxHP = damageManager.GetMaxHealth();
		if (maxHP <= 0)
			return 0;

		return Math.Clamp(damageManager.GetHealth() / maxHP, 0.0, 1.0) * 100;
	}

	//------------------------------------------------------------------------------------------------
	protected void SRZ_HealthTick()
	{
		if (!GetOwner())
		{
			GetGame().GetCallqueue().Remove(SRZ_HealthTick);
			return;
		}

		float pct = SRZ_ReadHealth();
		if (Math.AbsFloat(pct - m_fSRZHealthPct) < 0.5 && (pct > 0 || m_fSRZHealthPct <= 0))
			return;

		m_fSRZHealthPct = pct;
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	override float ArmstPlayerStatGetHealth()
	{
		if (Replication.IsServer())
			m_armst_player_stat_health = SRZ_ReadHealth();
		else
			m_armst_player_stat_health = m_fSRZHealthPct;

		return m_armst_player_stat_health;
	}
}
