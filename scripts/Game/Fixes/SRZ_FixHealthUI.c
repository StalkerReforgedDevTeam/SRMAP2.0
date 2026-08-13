[BaseContainerProps()]
modded class ARMST_PLAYER_STATS_COMPONENT : ScriptComponent
{
	override float ArmstPlayerStatGetHealth()
	{
		DamageManagerComponent damageManager = DamageManagerComponent.Cast(
			GetOwner().FindComponent(DamageManagerComponent)
		);
		float curHP;
		float maxHP;
		if (damageManager){
			curHP = damageManager.GetHealth();
			maxHP = damageManager.GetMaxHealth();
		}
		if (maxHP <= 0) 
			return 0;
		

		m_armst_player_stat_health = Math.Clamp( ( curHP / maxHP ), 0.0, 1.0) * 100;
		return m_armst_player_stat_health;
	}
}