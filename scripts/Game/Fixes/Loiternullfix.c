modded class SCR_CharacterControllerComponent
{
	override void StartLoitering(IEntity loiterEntity, int loiteringType, bool holsterWeapon, bool allowRootMotion, bool alignToPosition, vector targetPosition[4] = { "1 0 0", "0 1 0", "0 0 1", "0 0 0" }, bool disableInput = false,
		SCR_LoiterCustomAnimData customAnimData = SCR_LoiterCustomAnimData.Default)
	{
		if (loiteringType == ELoiteringType.NONE)
			return;

		ChimeraCharacter character = GetCharacter();
		RplComponent rplComp = character.GetRplComponent();
		if (rplComp && !rplComp.IsOwner())
			return;

		SCR_CharacterCommandHandlerComponent scrCmdHandler = SCR_CharacterCommandHandlerComponent.Cast(character.GetAnimationComponent().GetCommandHandler());
		if (!scrCmdHandler)
			return;

		if (!scrCmdHandler.GetCommandMove())
			return;

		if (!customAnimData)
			customAnimData = SCR_LoiterCustomAnimData.Default;
		else
			customAnimData.RandomizeSeed();

		m_pScrInputContext.SetLoiteringEntity(loiterEntity);
		m_pScrInputContext.m_iLoiteringType = loiteringType;
		m_pScrInputContext.m_bLoiteringShouldHolsterWeapon = holsterWeapon;
		m_pScrInputContext.m_bLoiteringShouldAlignCharacter = alignToPosition;
		m_pScrInputContext.m_bLoiteringDisablePlayerInput = disableInput;
		m_pScrInputContext.m_mLoiteringPosition = targetPosition;
		m_pScrInputContext.m_bLoiteringRootMotion = allowRootMotion;
		m_pScrInputContext.m_CustomAnimData = customAnimData;

		if (IsGadgetInHands())
			RemoveGadgetFromHand(true);

		if (alignToPosition)
			AlignToPositionFromCurrentPosition(targetPosition);

		TryStartLoiteringInternal();
	}
}