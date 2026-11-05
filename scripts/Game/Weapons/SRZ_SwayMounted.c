// No sway while the weapon is mounted on cover, deployed on a bipod, or while focus is held.
modded class SRZ_SwayAimModifier
{
	override protected void OnCalculate(IEntity owner, WeaponAimModifierContext context, float timeSlice, out vector translation, out vector rotation, out vector turnOffset)
	{
		bool steady = m_Controller && (m_Controller.GetIsWeaponDeployed() || m_Controller.GetIsWeaponDeployedBipod());

		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(m_Character);
		if (character && character.SRZ_IsFocusing())
			steady = true;

		if (steady)
		{
			translation = vector.Zero;
			rotation = vector.Zero;
			turnOffset = vector.Zero;
			return;
		}

		super.OnCalculate(owner, context, timeSlice, translation, rotation, turnOffset);
	}
}
