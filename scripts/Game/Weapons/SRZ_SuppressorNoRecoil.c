// Muzzle devices are cosmetic. They only change the shot sound and muzzle flash.
// No recoil, muzzle speed, dispersion or extra length changes, so handling is identical to a bare gun.
modded class SCR_WeaponAttachmentSuppressorAttributes
{
	//------------------------------------------------------------------------------------------------
	override bool ApplyModifiers(BaseWeaponStatsManagerComponent statsManager, int muzzleIndex, IEntity attachedEntity)
	{
		if (muzzleIndex == -1)
			return false;

		if (m_bOverrideShot)
		{
			if (!statsManager.SetShotSoundOverride(attachedEntity, muzzleIndex, true))
				return false;
		}

		if (m_bOverrideMuzzleEffects)
		{
			if (!statsManager.SetMuzzleEffectOverride(attachedEntity, muzzleIndex, true))
				return false;

			array<Managed> comArray = {};
			attachedEntity.FindComponents(MuzzleEffectComponent, comArray);
			foreach (Managed component : comArray)
			{
				MuzzleEffectComponent muzzleEffect = MuzzleEffectComponent.Cast(component);
				if (muzzleEffect)
					statsManager.AddMuzzleEffectOverride(attachedEntity, muzzleIndex, muzzleEffect);
			}
		}

		return true;
	}
}
