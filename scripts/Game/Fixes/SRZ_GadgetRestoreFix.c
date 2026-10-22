// Saved gadget back in hand on login without the EPF null crash, and the load step always finishes
modded class EPF_CharacterControllerComponentSaveData
{
	protected bool m_bSRZ_GadgetDone;

	override protected void OnGadgetInitDone(IEntity owner, SCR_GadgetManagerComponent gadgetMananger)
	{
		IEntity leftHandEntity = EPF_PersistenceManager.GetInstance().FindEntityByPersistentId(m_sLeftHandItemId);
		if (!leftHandEntity || !gadgetMananger || !m_pCharacterController)
		{
			SRZ_FinishGadget();
			return;
		}

		GetGame().GetCallqueue().CallLater(SRZ_FinishGadget, 5000);
		if (m_pCharacterController.m_OnGadgetStateChangedInvoker)
		{
			super.OnGadgetInitDone(owner, gadgetMananger);
			return;
		}

		gadgetMananger.HandleInput(leftHandEntity, 1);
		GetGame().GetCallqueue().Remove(SRZ_FinishGadget);
		GetGame().GetCallqueue().CallLater(SRZ_FinishGadget, 1500);
	}

	override protected void OnGadgetStateChangedCompleted()
	{
		SRZ_FinishGadget();
	}

	protected void SRZ_FinishGadget()
	{
		if (m_bSRZ_GadgetDone)
			return;

		m_bSRZ_GadgetDone = true;
		GetGame().GetCallqueue().Remove(SRZ_FinishGadget);
		if (m_pCharacterController && m_pCharacterController.m_OnGadgetStateChangedInvoker)
			m_pCharacterController.m_OnGadgetStateChangedInvoker.Remove(OnGadgetStateChanged);

		EPF_DeferredApplyResult.SetFinished(this, "CharacterControllerComponentSaveData::GadgetEquipped");
	}
}
