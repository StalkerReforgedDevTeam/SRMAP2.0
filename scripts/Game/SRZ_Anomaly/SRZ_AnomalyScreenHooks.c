modded class ARMST_DamagingTriggerEntity
{
	override void ArmstDamageStart(IEntity ent)
	{
		super.ArmstDamageStart(ent);
		if (m_damageTypeVanilla != EDamageType.Electro)
			return;

		ChimeraCharacter character = ChimeraCharacter.Cast(ent);
		if (!character)
			return;

		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
		if (controller)
			controller.SRZ_AskScreenFlash();
	}
}

modded class SCR_CharacterControllerComponent
{
	void SRZ_AskScreenFlash()
	{
		ChimeraCharacter character = GetCharacter();
		if (!character)
			return;

		RplComponent rpl = character.GetRplComponent();
		if (!rpl || rpl.IsOwner())
		{
			SRZ_ZoneFeel.Flash(character);
			return;
		}

		Rpc(RpcDo_SRZ_ScreenFlash);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_SRZ_ScreenFlash()
	{
		SRZ_ZoneFeel.Flash(GetCharacter());
	}
}
