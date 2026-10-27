// LM Suppression touch ups: your own shots never suppress you, and the vignette and flinch
// overlays are forced clear once suppression has worn off so they can't get stuck on screen.

modded class SCR_MuzzleEffectComponent
{
	override void OnFired(IEntity effectEntity, BaseMuzzleComponent muzzle, IEntity projectileEntity)
	{
		super.OnFired(effectEntity, muzzle, projectileEntity);

		if (!muzzle || !LM_SuppressionUtils.IsClient())
			return;

		IEntity local = SCR_PlayerController.GetLocalControlledEntity();
		if (!local)
			return;

		IEntity shooter = SCR_EntityHelper.GetMainParent(muzzle.GetOwner());
		if (shooter != local)
			return;

		LM_SuppressionProjectileComponent projectile = LM_SuppressionProjectileComponent.Cast(projectileEntity.FindComponent(LM_SuppressionProjectileComponent));
		if (projectile)
		{
			projectile.SetImpactWhenDead(false);
			projectile.SetCanFlinch(false);
		}

		LM_Suppression.GetInstance().RemoveAllProjectiles();
	}
}

modded class LM_SuppressionScreenEffect
{
	protected static const float SRZ_CLEAR_AFTER = 1.5;
	protected float m_fSRZ_QuietFor;

	override void UpdateEffect(float timeSlice)
	{
		super.UpdateEffect(timeSlice);

		float amount = 0;
		if (m_pPlayerController)
			amount = m_pPlayerController.GetSuppressionAmount();

		if (amount > 0.001 || m_fSuppression > 0.01)
		{
			m_fSRZ_QuietFor = 0;
			return;
		}

		m_fSRZ_QuietFor += timeSlice;
		if (m_fSRZ_QuietFor < SRZ_CLEAR_AFTER)
			return;

		if (m_wSuppressionVignette)
		{
			m_wSuppressionVignette.SetMaskProgress(0);
			m_wSuppressionVignette.SetOpacity(0);
		}

		if (m_wSuppressionFlinch)
		{
			AnimateWidget.StopAllAnimations(m_wSuppressionFlinch);
			m_wSuppressionFlinch.SetMaskProgress(0);
			m_wSuppressionFlinch.SetOpacity(0);
		}
	}

	override void DisplayOnSuspended()
	{
		super.DisplayOnSuspended();

		if (m_wSuppressionVignette)
		{
			m_wSuppressionVignette.SetMaskProgress(0);
			m_wSuppressionVignette.SetOpacity(0);
		}

		if (m_wSuppressionFlinch)
		{
			AnimateWidget.StopAllAnimations(m_wSuppressionFlinch);
			m_wSuppressionFlinch.SetOpacity(0);
		}
	}

}
