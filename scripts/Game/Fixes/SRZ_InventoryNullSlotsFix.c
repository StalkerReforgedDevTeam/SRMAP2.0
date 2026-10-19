// Saved storage with no items loads as m_aSlots = null and EPF crashes on it (endless loading). Treat it as empty.
modded class EPF_BaseInventoryStorageComponentSaveData
{
	override EPF_EApplyResult ApplyTo(IEntity owner, GenericComponent component, EPF_ComponentSaveDataClass attributes)
	{
		if (!m_aSlots)
		{
			m_aSlots = {};
			Print(string.Format("[SRZ Save] Empty storage record on %1 (%2), loaded as empty", owner, component), LogLevel.WARNING);
		}

		return super.ApplyTo(owner, component, attributes);
	}
}
