modded class ARMST_PLAYER_STATS_COMPONENT
{
	[Attribute("70", UIWidgets.EditBox, "Radiation sickness threshold (0-100)", category: "RADIATION SICKNESS")]
	protected float m_fRadiationSicknessThreshold;

	[Attribute("1.0", UIWidgets.EditBox, "Health lost per tick while sick", category: "RADIATION SICKNESS")]
	protected float m_fSicknessHealthDrainPerTick;

	[Attribute("0.05", UIWidgets.EditBox, "Stamina drained per tick while sick (0-1 scale)", category: "RADIATION SICKNESS")]
	protected float m_fSicknessStaminaDrainPerTick;

	[Attribute("0.5", UIWidgets.EditBox, "Move speed multiplier while sick", category: "RADIATION SICKNESS")]
	protected float m_fSicknessMoveSpeedMult;

	[Attribute("2.0", UIWidgets.EditBox, "Radiation decayed per tick while outside a radiation zone", category: "RADIATION SICKNESS")]
	protected float m_fRadiationDecayPerTick;

	[Attribute("5.0", UIWidgets.EditBox, "Seconds between radiation ticks", category: "RADIATION SICKNESS")]
	protected float m_fRadiationTickInterval;

	protected bool m_bSicknessTickStarted;

	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		if (SCR_Global.IsEditMode())
			return;
		if (!GetGame().InPlayMode())
			return;
		if (!Replication.IsServer())
			return;
		if (m_bSicknessTickStarted)
			return;

		m_bSicknessTickStarted = true;
		GetGame().GetCallqueue().CallLater(RadiationTick, m_fRadiationTickInterval * 1000, true);
	}

	protected void RadiationTick()
	{
		if (m_armst_radiactive_level <= 0 && m_armst_player_stat_radiactive > 0)
			ArmstPlayerStatSetRadio(-m_fRadiationDecayPerTick);

		if (IsRadiationSick())
		{
			ApplySicknessTick();
			ApplyStaminaDrain();
		}

		ApplySpeedOverride();
	}

	protected void ApplySpeedOverride()
	{
		IEntity owner = GetOwner();
		if (!owner)
			return;

		CharacterControllerComponent controller = CharacterControllerComponent.Cast(owner.FindComponent(CharacterControllerComponent));
		if (!controller)
			return;

		if (IsRadiationSick())
			controller.OverrideMaxSpeed(m_fSicknessMoveSpeedMult);
		else
			controller.OverrideMaxSpeed(-1);
	}

	protected void ApplyStaminaDrain()
	{
		IEntity owner = GetOwner();
		if (!owner)
			return;

		CharacterControllerComponent controller = CharacterControllerComponent.Cast(owner.FindComponent(CharacterControllerComponent));
		if (!controller)
			return;

		CharacterStaminaComponent stamina = controller.GetStaminaComponent();
		if (!stamina)
			return;

		stamina.AddStamina(-m_fSicknessStaminaDrainPerTick);
	}

	protected void ApplySicknessTick()
	{
		IEntity owner = GetOwner();
		if (!owner)
			return;

		DamageManagerComponent damageManager = DamageManagerComponent.Cast(owner.FindComponent(DamageManagerComponent));
		if (!damageManager)
			return;

		float maxHealth = damageManager.GetMaxHealth();
		if (maxHealth <= 0)
			return;

		float newHealth = damageManager.GetHealth() - m_fSicknessHealthDrainPerTick;
		if (newHealth < 0)
			newHealth = 0;

		damageManager.SetHealthScaled(newHealth / maxHealth);
	}

	bool IsRadiationSick()
	{
		return m_armst_player_stat_radiactive >= m_fRadiationSicknessThreshold;
	}
}