// Players could fire out of an emote and get stuck in the pose.
// Player emotes now put the weapon away first and start once it is holstered.
// If a weapon is drawn again while the emote is playing, the emote ends.
// Sitting on chairs or beds and NPC animations are left alone.
modded class ARMST_PLAYER_ANIMATION
{
	protected static const int SRZ_HOLSTER_DELAY_MS = 900;
	protected static const int SRZ_WATCH_MS = 250;
	protected bool m_bSRZHolstered;

	//------------------------------------------------------------------------------------------------
	override void StartAnimationsPlayerByName(string animName)
	{
		if (!m_bSRZHolstered && SRZ_HolsterFirst(animName))
			return;

		m_bSRZHolstered = false;
		super.StartAnimationsPlayerByName(animName);

		if (SRZ_GetPlayerController())
		{
			GetGame().GetCallqueue().Remove(SRZ_WatchWeapon);
			GetGame().GetCallqueue().CallLater(SRZ_WatchWeapon, SRZ_HOLSTER_DELAY_MS, false);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_CharacterControllerComponent SRZ_GetPlayerController()
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());
		if (!character || !EntityUtils.IsPlayer(character))
			return null;

		return SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
	}

	//------------------------------------------------------------------------------------------------
	protected bool SRZ_HasWeaponInHands(SCR_CharacterControllerComponent charCtrl)
	{
		BaseWeaponManagerComponent weapons = charCtrl.GetWeaponManagerComponent();
		return weapons && weapons.GetCurrentWeapon();
	}

	//------------------------------------------------------------------------------------------------
	protected bool SRZ_HolsterFirst(string animName)
	{
		SCR_CharacterControllerComponent charCtrl = SRZ_GetPlayerController();
		if (!charCtrl || !SRZ_HasWeaponInHands(charCtrl))
			return false;

		charCtrl.SelectWeapon(null);
		m_bSRZHolstered = true;
		GetGame().GetCallqueue().CallLater(StartAnimationsPlayerByName, SRZ_HOLSTER_DELAY_MS, false, animName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void SRZ_WatchWeapon()
	{
		SCR_CharacterControllerComponent charCtrl = SRZ_GetPlayerController();
		if (!charCtrl || charCtrl.IsDead() || !charCtrl.IsLoitering())
			return;

		if (SRZ_HasWeaponInHands(charCtrl))
		{
			charCtrl.StopLoitering(false);
			return;
		}

		GetGame().GetCallqueue().CallLater(SRZ_WatchWeapon, SRZ_WATCH_MS, false);
	}
}