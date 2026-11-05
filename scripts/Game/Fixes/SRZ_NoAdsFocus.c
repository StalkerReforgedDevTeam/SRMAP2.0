// Focus zoom rules: free on magnified optics and gadgets, costs stamina on irons and red dots, blocked from the hip.
// Holding focus on irons drains stamina on the server. At empty stamina focus is blocked until it recovers.
// While focus is active weapon sway is switched off.
class SRZ_FocusStamina
{
	static const float DRAIN_PER_SECOND = 0.07;
	static const float HELD_ABOVE = 0.1;
	static const int TICK_MS = 250;
	static const float BLOCK_BELOW = 0.02;
	static const float RECOVER_ABOVE = 0.15;
}

modded class SCR_CharacterCameraHandlerComponent
{
	protected bool m_bSRZFocusTired;
	protected bool m_bSRZFocusDraining;
	protected bool m_bSRZFocusActive;

	//------------------------------------------------------------------------------------------------
	override void OnBeforeCameraUpdate(float pDt, bool pIsKeyframe)
	{
		super.OnBeforeCameraUpdate(pDt, pIsKeyframe);

		int rule = SRZ_FocusRule();
		bool focusing = m_fFocusValue > 0 || m_fFocusTargetValue > 0;
		bool held = SRZ_FocusHeld();

		if (rule == 2)
			SRZ_UpdateTired();

		bool drain = held && rule == 2 && !m_bSRZFocusTired;
		bool active = (held && rule == 1) || drain;
		SRZ_SetFocusState(active, drain);

		if (!focusing)
			return;

		if (rule == 1 || drain)
			return;

		m_fFocusTargetValue = 0;
		m_fFocusValue = 0;
		m_bDoInterpolateFocus = false;
		SetFocusMode(0);
	}

	//------------------------------------------------------------------------------------------------
	// Only the player's own focus input counts. Vanilla auto focus while aiming is ignored.
	protected bool SRZ_FocusHeld()
	{
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!playerController || playerController.GetControlledEntity() != GetOwner())
			return false;

		return playerController.GetFocusValue(0, -1) > SRZ_FocusStamina.HELD_ABOVE;
	}

	//------------------------------------------------------------------------------------------------
	// 0 = blocked, 1 = free, 2 = costs stamina
	protected int SRZ_FocusRule()
	{
		BaseWeaponManagerComponent weapons = BaseWeaponManagerComponent.Cast(GetOwner().FindComponent(BaseWeaponManagerComponent));
		BaseWeaponComponent weapon;
		if (weapons)
			weapon = weapons.GetCurrentWeapon();

		if (!weapon)
		{
			SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
			if (playerController && playerController.GetGadgetFocus())
				return 1;
			return 0;
		}

		if (m_fADSProgress <= 0)
			return 0;

		BaseSightsComponent sights = weapon.GetSights();
		if (sights && SCR_2DOpticsComponent.Cast(sights))
			return 1;

		return 2;
	}

	//------------------------------------------------------------------------------------------------
	protected void SRZ_UpdateTired()
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());
		if (!character)
			return;

		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return;

		float stamina = controller.GetStamina();
		if (stamina <= SRZ_FocusStamina.BLOCK_BELOW)
			m_bSRZFocusTired = true;
		else if (stamina >= SRZ_FocusStamina.RECOVER_ABOVE)
			m_bSRZFocusTired = false;
	}

	//------------------------------------------------------------------------------------------------
	protected void SRZ_SetFocusState(bool active, bool drain)
	{
		if (active == m_bSRZFocusActive && drain == m_bSRZFocusDraining)
			return;

		m_bSRZFocusActive = active;
		m_bSRZFocusDraining = drain;
		Print("[SRZ_Focus] active " + active.ToString() + " drain " + drain.ToString(), LogLevel.NORMAL);

		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(GetOwner());
		if (character)
			character.SRZ_SetFocusState(active, drain);
	}
}

modded class SCR_ChimeraCharacter
{
	protected bool m_bSRZFocusDrain;
	protected bool m_bSRZFocusActive;

	//------------------------------------------------------------------------------------------------
	bool SRZ_IsFocusing()
	{
		return m_bSRZFocusActive;
	}

	//------------------------------------------------------------------------------------------------
	void SRZ_SetFocusState(bool active, bool drain)
	{
		m_bSRZFocusActive = active;

		if (Replication.IsServer())
		{
			SRZ_ApplyFocusDrain(drain);
			return;
		}

		Rpc(RpcAsk_SRZFocusState, active, drain);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_SRZFocusState(bool active, bool drain)
	{
		m_bSRZFocusActive = active;
		SRZ_ApplyFocusDrain(drain);
	}

	//------------------------------------------------------------------------------------------------
	protected void SRZ_ApplyFocusDrain(bool drain)
	{
		if (drain == m_bSRZFocusDrain)
			return;

		m_bSRZFocusDrain = drain;
		GetGame().GetCallqueue().Remove(SRZ_FocusDrainTick);

		if (drain)
			GetGame().GetCallqueue().CallLater(SRZ_FocusDrainTick, SRZ_FocusStamina.TICK_MS, true);
	}

	//------------------------------------------------------------------------------------------------
	protected void SRZ_FocusDrainTick()
	{
		CharacterControllerComponent controller = GetCharacterController();
		if (!controller || controller.IsDead())
		{
			SRZ_ApplyFocusDrain(false);
			return;
		}

		CharacterStaminaComponent stamina = controller.GetStaminaComponent();
		if (stamina)
			stamina.AddStamina(-SRZ_FocusStamina.DRAIN_PER_SECOND * SRZ_FocusStamina.TICK_MS / 1000.0);
	}
}
