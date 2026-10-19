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

		return super.ApplyTo(owner, component, attributes);
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

		return super.ApplyTo(owner, component, attributes);
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

		return super.ApplyTo(owner, component, attributes);
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

		return super.ApplyTo(owner, component, attributes);
	}
}
