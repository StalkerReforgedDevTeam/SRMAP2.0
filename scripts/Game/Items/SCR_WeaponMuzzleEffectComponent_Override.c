// ============================================================
// FILE: SCR_MuzzleEffectComponent_Override.c
//
// Overrides the locked mod's OnFired to remove the old
// client-side degradation code. All degradation is now
// handled via OnProjectileShot in
// SCR_CharacterControllerComponent.
// ============================================================

modded class SCR_MuzzleEffectComponent : MuzzleEffectComponent
{
	override void OnFired(IEntity effectEntity, BaseMuzzleComponent muzzle, IEntity projectileEntity)
	{
		if (m_OnWeaponFired)
			m_OnWeaponFired.Invoke(effectEntity, muzzle, projectileEntity);
	}
}