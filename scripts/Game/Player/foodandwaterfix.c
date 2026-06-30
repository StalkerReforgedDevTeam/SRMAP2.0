// scripts/Game/Fixes/SRZ_PlayerVitalsEffects.c

modded class ARMST_PLAYER_STATS_COMPONENT
{
	// --- Tunables ---
	protected const float SRZ_VITALS_TICK_INTERVAL = 5000;   // ms between checks
	protected const float SRZ_LOW_WATER_THRESHOLD = 20;      // below this, water damage applies
	protected const float SRZ_LOW_FOOD_THRESHOLD = 20;       // below this, food damage applies
	protected const float SRZ_WATER_DAMAGE_PER_TICK = 1.0;   // health lost per tick when dehydrated
	protected const float SRZ_FOOD_DAMAGE_PER_TICK = 1.0;    // health lost per tick when starving
	protected const float SRZ_VITALS_MIN_HEALTH = 1;         // starvation/dehydration alone won't kill, stops here

	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		if (!GetGame().InPlayMode())
			return;

		if (!Replication.IsServer())
			return;

		GetGame().GetCallqueue().CallLater(SRZ_VitalsTick, SRZ_VITALS_TICK_INTERVAL, true);
	}

	void SRZ_VitalsTick()
	{
		IEntity owner = GetOwner();
		if (!owner)
			return;

		float water = ArmstPlayerStatGetWater();
		float food = ArmstPlayerStatGetEat();

		if (water > SRZ_LOW_WATER_THRESHOLD && food > SRZ_LOW_FOOD_THRESHOLD)
			return;

		DamageManagerComponent damageManager = DamageManagerComponent.Cast(owner.FindComponent(DamageManagerComponent));
		if (!damageManager)
			return;

		HitZone defaultHitZone = damageManager.GetDefaultHitZone();
		if (!defaultHitZone)
			return;

		float currentHealth = defaultHitZone.GetHealth();
		if (currentHealth <= SRZ_VITALS_MIN_HEALTH)
			return;

		float totalDamage = 0;

		if (water <= SRZ_LOW_WATER_THRESHOLD)
			totalDamage += SRZ_WATER_DAMAGE_PER_TICK;

		if (food <= SRZ_LOW_FOOD_THRESHOLD)
			totalDamage += SRZ_FOOD_DAMAGE_PER_TICK;

		float newHealth = Math.Max(SRZ_VITALS_MIN_HEALTH, currentHealth - totalDamage);
		defaultHitZone.SetHealth(newHealth);
	}
}