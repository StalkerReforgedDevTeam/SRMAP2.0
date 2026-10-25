// Saved gadget back in hand on login without the EPF null crash, and the load step always finishes
// [EDF_DbName()] must stay: it gives saves the short "CharacterControllerComponent" name. Without it no older save can be read.
// No new member variables here, they would be written into every save file.
[EDF_DbName()]
modded class EPF_CharacterControllerComponentSaveData
{
	protected static ref set<EPF_CharacterControllerComponentSaveData> s_SRZ_GadgetDone = new set<EPF_CharacterControllerComponentSaveData>();

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
		if (s_SRZ_GadgetDone.Contains(this))
			return;

		s_SRZ_GadgetDone.Insert(this);
		GetGame().GetCallqueue().Remove(SRZ_FinishGadget);
		if (m_pCharacterController && m_pCharacterController.m_OnGadgetStateChangedInvoker)
			m_pCharacterController.m_OnGadgetStateChangedInvoker.Remove(OnGadgetStateChanged);

		EPF_DeferredApplyResult.SetFinished(this, "CharacterControllerComponentSaveData::GadgetEquipped");
	}
}
