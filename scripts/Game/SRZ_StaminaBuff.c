// ============================================================================
// SRZ_StaminaBuff.c
// Two-part stamina buff system:
//   SRZ_StaminaBuffManager  → attach to PLAYER prefab (once)
//   SRZ_StaminaBuff         → attach to each consumable ITEM prefab
//
// Runs on the machine where ActivateAction fires.
// AddStamina is client-authoritative in Reforger.
// ============================================================================

// ---------------------------------------------------------------------------
// Player-side manager — processes the active buff.
// Lives on the player entity so it survives after the item is consumed.
// ---------------------------------------------------------------------------
class SRZ_StaminaBuffManagerClass : ScriptComponentClass {}

class SRZ_StaminaBuffManager : ScriptComponent
{
	protected float m_fRemainingTime;
	protected float m_fTickInterval;
	protected float m_fStaminaPerTick;
	protected bool  m_bBuffActive;
	protected bool  m_bTickScheduled;

	//! Start or refresh a stamina buff.
	void StartBuff(float totalStamina, float duration, float tickInterval)
	{
		totalStamina = Math.Clamp(totalStamina, 0.0, 5.0);
		duration     = Math.Clamp(duration, 1.0, 600.0);
		tickInterval = Math.Clamp(tickInterval, 0.25, duration);

		int tickCount      = Math.Ceil(duration / tickInterval);
		m_fStaminaPerTick  = totalStamina / tickCount;
		m_fTickInterval    = tickInterval;
		m_fRemainingTime   = tickCount * tickInterval;
		m_bBuffActive      = true;

		if (!m_bTickScheduled)
		{
			m_bTickScheduled = true;
			GetGame().GetCallqueue().CallLater(BuffTick, m_fTickInterval * 1000, false);
		}
	}

	//! Returns true while a buff is active.
	bool IsBuffActive()
	{
		return m_bBuffActive;
	}

	protected void BuffTick()
	{
		m_bTickScheduled = false;

		if (!GetOwner())
		{
			StopBuff();
			return;
		}

		CharacterControllerComponent ctrl = CharacterControllerComponent.Cast(
			GetOwner().FindComponent(CharacterControllerComponent));
		if (!ctrl || ctrl.GetLifeState() == ECharacterLifeState.DEAD)
		{
			StopBuff();
			return;
		}

		CharacterStaminaComponent stamina = CharacterStaminaComponent.Cast(
			GetOwner().FindComponent(CharacterStaminaComponent));
		if (stamina)
			stamina.AddStamina(m_fStaminaPerTick);

		m_fRemainingTime -= m_fTickInterval;
		if (m_fRemainingTime <= 0)
		{
			StopBuff();
			return;
		}

		m_bTickScheduled = true;
		GetGame().GetCallqueue().CallLater(BuffTick, m_fTickInterval * 1000, false);
	}

	protected void StopBuff()
	{
		m_bBuffActive      = false;
		m_bTickScheduled   = false;
		m_fRemainingTime   = 0;
		m_fStaminaPerTick  = 0;
	}

	override void OnDelete(IEntity owner)
	{
		if (m_bTickScheduled)
			GetGame().GetCallqueue().Remove(BuffTick);
		super.OnDelete(owner);
	}
}


// ---------------------------------------------------------------------------
// Item-side component — attach to every consumable prefab that should
// grant a stamina buff (energy drink, coffee, tea …).
// Configure the values per prefab in the World Editor.
// ---------------------------------------------------------------------------
class SRZ_StaminaBuffClass : ScriptComponentClass {}

class SRZ_StaminaBuff : ScriptComponent
{
	[Attribute("0.80", UIWidgets.Slider,
		"Total stamina to restore over the buff duration (0.0–1.0 = 0–100%)",
		"0.0 1.0 0.05",
		category: "Buff Settings")]
	protected float m_fTotalStaminaRestore;

	[Attribute("30", UIWidgets.Slider,
		"Buff duration in seconds",
		"5 300 1",
		category: "Buff Settings")]
	protected float m_fBuffDuration;

	[Attribute("2.0", UIWidgets.Slider,
		"Seconds between each stamina tick",
		"0.25 10.0 0.25",
		category: "Buff Settings")]
	protected float m_fTickInterval;

	float GetTotalStaminaRestore() { return m_fTotalStaminaRestore; }
	float GetBuffDuration()        { return m_fBuffDuration; }
	float GetTickInterval()        { return m_fTickInterval; }

	//! Call this when the consumable is used.
	void ActivateBuff(IEntity character)
	{
		if (!character)
			return;

		SRZ_StaminaBuffManager mgr = SRZ_StaminaBuffManager.Cast(
			character.FindComponent(SRZ_StaminaBuffManager));

		if (!mgr)
			return;

		mgr.StartBuff(m_fTotalStaminaRestore, m_fBuffDuration, m_fTickInterval);
	}
}