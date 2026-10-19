class SRZ_SavedItemGuard
{
	static bool Usable(EPF_EntitySaveData saveData)
	{
		if (!saveData)
			return true;

		if (saveData.m_rPrefab.IsEmpty())
			return false;

		Resource resource = Resource.Load(saveData.m_rPrefab);
		return resource && resource.IsValid();
	}

	static void Skip(EPF_EntitySaveData saveData, IEntity owner)
	{
		string where = "unknown";
		if (owner)
			where = EPF_Utils.GetPrefabName(owner);

		Print(string.Format("[SRZ Save] saved item '%1' no longer exists, dropped it from '%2' so loading can carry on", saveData.m_rPrefab, where), LogLevel.WARNING);
	}

	static EPF_EApplyResult Soften(EPF_EApplyResult result, IEntity owner, string what)
	{
		if (result != EPF_EApplyResult.ERROR)
			return result;

		string where = "unknown";
		if (owner)
			where = EPF_Utils.GetPrefabName(owner);

		Print(string.Format("[SRZ Save] %1 on '%2' failed to restore, skipped it so loading can carry on", what, where), LogLevel.WARNING);
		return EPF_EApplyResult.OK;
	}
}

modded class EPF_PersistenceComponent
{
	override bool Load(notnull EPF_EntitySaveData saveData, bool isRoot = true)
	{
		if (super.Load(saveData, isRoot))
			return true;

		if (isRoot || ChimeraCharacter.Cast(GetOwner()))
			return false;

		Print(string.Format("[SRZ Save] item '%1' only partly restored, kept it so its owner can still load", saveData.m_rPrefab), LogLevel.WARNING);
		return true;
	}
}

modded class EPF_BaseInventoryStorageComponentSaveData
{
	override EPF_EApplyResult ApplyTo(IEntity owner, GenericComponent component, EPF_ComponentSaveDataClass attributes)
	{
		if (m_aSlots)
		{
			for (int i = m_aSlots.Count() - 1; i >= 0; i--)
			{
				EPF_PersistentInventoryStorageSlot slot = m_aSlots[i];
				if (!slot || SRZ_SavedItemGuard.Usable(slot.m_pEntity))
					continue;

				SRZ_SavedItemGuard.Skip(slot.m_pEntity, owner);
				m_aSlots.Remove(i);
			}
		}

		return SRZ_SavedItemGuard.Soften(super.ApplyTo(owner, component, attributes), owner, "storage");
	}
}

modded class EPF_SlotManagerComponentSaveData
{
	override EPF_EApplyResult ApplyTo(IEntity owner, GenericComponent component, EPF_ComponentSaveDataClass attributes)
	{
		if (m_aSlots)
		{
			for (int i = m_aSlots.Count() - 1; i >= 0; i--)
			{
				EPF_PersistentEntitySlot slot = m_aSlots[i];
				if (!slot || SRZ_SavedItemGuard.Usable(slot.m_pEntity))
					continue;

				SRZ_SavedItemGuard.Skip(slot.m_pEntity, owner);
				m_aSlots.Remove(i);
			}
		}

		return SRZ_SavedItemGuard.Soften(super.ApplyTo(owner, component, attributes), owner, "slot");
	}
}

modded class EPF_WeaponSlotComponentSaveData
{
	override EPF_EApplyResult ApplyTo(IEntity owner, GenericComponent component, EPF_ComponentSaveDataClass attributes)
	{
		if (!SRZ_SavedItemGuard.Usable(m_pEntity))
		{
			SRZ_SavedItemGuard.Skip(m_pEntity, owner);
			return EPF_EApplyResult.OK;
		}

		return SRZ_SavedItemGuard.Soften(super.ApplyTo(owner, component, attributes), owner, "weapon slot");
	}
}

modded class EPF_BaseSlotComponentSaveData
{
	override EPF_EApplyResult ApplyTo(IEntity owner, GenericComponent component, EPF_ComponentSaveDataClass attributes)
	{
		if (!SRZ_SavedItemGuard.Usable(m_pEntity))
		{
			SRZ_SavedItemGuard.Skip(m_pEntity, owner);
			return EPF_EApplyResult.OK;
		}

		return SRZ_SavedItemGuard.Soften(super.ApplyTo(owner, component, attributes), owner, "slot");
	}
}

modded class EPF_CharacterControllerComponentSaveData
{
	protected bool m_bSRZ_GadgetDelayed;

	protected override void OnGadgetInitDone(IEntity owner, SCR_GadgetManagerComponent gadgetMananger)
	{
		if (!m_bSRZ_GadgetDelayed)
		{
			m_bSRZ_GadgetDelayed = true;
			GetGame().GetCallqueue().CallLater(OnGadgetInitDone, 1500, false, owner, gadgetMananger);
			GetGame().GetCallqueue().CallLater(SRZ_GadgetGiveUp, 6000);
			return;
		}

		if (!owner || !gadgetMananger || !m_pCharacterController)
		{
			SRZ_GadgetGiveUp();
			return;
		}

		super.OnGadgetInitDone(owner, gadgetMananger);
	}

	protected void SRZ_GadgetGiveUp()
	{
		EPF_DeferredApplyResult.SetFinished(this, "CharacterControllerComponentSaveData::GadgetEquipped");
	}
}
