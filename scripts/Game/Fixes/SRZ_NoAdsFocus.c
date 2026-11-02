// No focus zoom at all, except optics while aiming and raised gadgets such as binoculars.
modded class SCR_CharacterCameraHandlerComponent
{
	//------------------------------------------------------------------------------------------------
	override void OnBeforeCameraUpdate(float pDt, bool pIsKeyframe)
	{
		super.OnBeforeCameraUpdate(pDt, pIsKeyframe);

		if (m_fFocusValue <= 0 || SRZ_FocusAllowed())
			return;

		m_fFocusTargetValue = 0;
		m_fFocusValue = 0;
		m_bDoInterpolateFocus = false;
		SetFocusMode(0);
	}

	//------------------------------------------------------------------------------------------------
	protected bool SRZ_FocusAllowed()
	{
		BaseWeaponManagerComponent weapons = BaseWeaponManagerComponent.Cast(GetOwner().FindComponent(BaseWeaponManagerComponent));
		BaseWeaponComponent weapon;
		if (weapons)
			weapon = weapons.GetCurrentWeapon();

		if (!weapon)
		{
			SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
			return playerController && playerController.GetGadgetFocus();
		}

		if (m_fADSProgress <= 0)
			return false;

		BaseSightsComponent sights = weapon.GetSights();
		if (!sights)
			return false;

		return SCR_2DOpticsComponent.Cast(sights) != null;
	}
}
