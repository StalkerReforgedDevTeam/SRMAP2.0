// ============================================================================
// SRZ_ItemUseComponentOverride.c
// Modded class — injects SRZ_StaminaBuff hook into ARMST_ItemUseComponent
// without modifying the original script file.
// ============================================================================

modded class ARMST_ItemUseComponent : SCR_ConsumableItemComponent
{
	override void ActivateAction()
	{
		super.ActivateAction();

		SRZ_StaminaBuff buff = SRZ_StaminaBuff.Cast(GetOwner().FindComponent(SRZ_StaminaBuff));
		if (buff)
			buff.ActivateBuff(m_CharacterOwner);
	}
}