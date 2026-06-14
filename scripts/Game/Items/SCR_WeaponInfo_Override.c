// ============================================================
// FILE: SCR_WeaponInfo_Override.c
//
// Overrides the locked mod's OnAmmoCountChanged to REMOVE
// the client-side jam logic. All jam/degradation is now
// handled server-side via SCR_CharacterControllerComponent.
// ============================================================

modded class SCR_WeaponInfo : SCR_InfoDisplayExtended
{
	override void OnAmmoCountChanged(
		BaseWeaponComponent weapon,
		BaseMuzzleComponent muzzle,
		BaseMagazineComponent magazine,
		int ammoCount,
		bool isBarrelChambered)
	{
		if (!m_WeaponState)
			return;

		#ifdef WEAPON_INFO_DEBUG
		_print("OnAmmoCountChanged");
		_print(string.Format("    weapon:            %1", weapon));
		_print(string.Format("    muzzle:            %1", muzzle));
		_print(string.Format("    magazine:          %1", magazine));
		_print(string.Format("    ammoCount:         %1", ammoCount));
		_print(string.Format("    isBarrelChambered: %1", isBarrelChambered));
		#endif

		// ---- JAM LOGIC REMOVED ----
		// Previously this had a client-side jam roll here.
		// That is now handled server-side in
		// SCR_CharacterControllerComponent.RpcAsk_TAO_WeaponDegradation()

		// Set weapon state change flag
		m_eWeaponStateEvent |= EWeaponFeature.AMMOCOUNT;

		m_WeaponState.m_bBarrelChambered = isBarrelChambered;
		m_WeaponState.m_iMagAmmoCount = ammoCount;
		m_WeaponState.m_iMagMaxAmmoCount = 1;

		if (magazine)
			m_WeaponState.m_iMagMaxAmmoCount = magazine.GetMaxAmmoCount();

		if (m_WeaponState.m_iMagMaxAmmoCount == 0)
			m_WeaponState.m_iMagMaxAmmoCount = 1;

		m_WeaponState.m_fMagAmmoPerc = m_WeaponState.m_iMagAmmoCount / m_WeaponState.m_iMagMaxAmmoCount;

		if (magazine)
			UpdateMagazineIndicator_Progress(m_WeaponState);

		UpdateBulletInChamberIndicator(m_WeaponState);
	}
}