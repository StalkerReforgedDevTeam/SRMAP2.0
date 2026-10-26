// Saved storage with no items loads as m_aSlots = null and EPF crashes on it (endless loading). Treat it as empty.
// [EDF_DbName()] must stay: it gives saves the short "InventoryStorageComponent" name. Without it no older save can be read.
[EDF_DbName()]
modded class EPF_BaseInventoryStorageComponentSaveData
{
	override EPF_EApplyResult ApplyTo(IEntity owner, GenericComponent component, EPF_ComponentSaveDataClass attributes)
	{
		if (!m_aSlots)
		{
			m_aSlots = {};
			Print(string.Format("[SRZ Save] Empty storage record on %1 (%2), loaded as empty", owner, component), LogLevel.WARNING);
		}

		// An item whose prefab no longer exists (renamed or removed in an update) made the whole character fail to load.
		// Drop just that item and keep the rest.
		for (int i = m_aSlots.Count() - 1; i >= 0; i--)
		{
			EPF_PersistentInventoryStorageSlot slot = m_aSlots[i];
			if (!slot || !slot.m_pEntity || slot.m_pEntity.m_rPrefab.IsEmpty())
				continue;

			Resource res = Resource.Load(slot.m_pEntity.m_rPrefab);
			if (res && res.IsValid())
				continue;

			Print(string.Format("[SRZ Save] Dropping unknown item %1 from %2, prefab no longer exists", slot.m_pEntity.m_rPrefab, owner), LogLevel.WARNING);
			m_aSlots.Remove(i);
		}

		SRZ_StorageLoadGuard.s_iLoading++;
		EPF_EApplyResult result = super.ApplyTo(owner, component, attributes);
		SRZ_StorageLoadGuard.s_iLoading--;
		return result;
	}
}
