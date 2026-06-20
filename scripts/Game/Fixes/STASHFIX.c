modded class SCR_InventoryStoragesListUI
{
	protected override int CreateSlots()
	{
		if (!m_InventoryStorage)
			return -1;

		m_Items.Clear();
		m_InventoryStorage.GetAll(m_Items);
		m_aSlots.Clear();

		SCR_InventorySlotUI uiSlot;
		foreach (IEntity item : m_Items)
		{
			if (!item)
				continue;

			InventoryItemComponent comp = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
			if (!comp)
			{
				Print("[ARMST] Skipping item with no InventoryItemComponent in storages list: " + item, LogLevel.WARNING);
				continue;
			}

			uiSlot = CreateStorageSlotUI(comp);
			if (!uiSlot)
				continue;

			m_aSlots.Insert(uiSlot);
		}

		return m_aSlots.Count() - 1;
	}
}