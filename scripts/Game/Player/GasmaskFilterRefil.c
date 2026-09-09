modded class ARMST_ItemUseComponent
{
	[Attribute("0", UIWidgets.CheckBox, "Using this item repairs the equipped gas mask's condition", category: "FILTER")]
	bool m_bRepairsGasmaskFilter;

	[Attribute("50", UIWidgets.EditBox, "Percent of gas mask condition restored on use (0-100)", category: "FILTER")]
	float m_fFilterRepairPercent;

	override void ActivateAction()
	{
		super.ActivateAction();

		Print("ARMST FILTER DEBUG: ActivateAction reached, m_bRepairsGasmaskFilter=" + m_bRepairsGasmaskFilter);

		if (m_bRepairsGasmaskFilter)
			ApplyFilterRepair();
	}

	protected void ApplyFilterRepair()
	{
		Print("ARMST FILTER DEBUG: ApplyFilterRepair called");

		if (!m_CharacterOwner)
		{
			Print("ARMST FILTER DEBUG: stopped - m_CharacterOwner is null");
			return;
		}

		EquipedLoadoutStorageComponent loadout = EquipedLoadoutStorageComponent.Cast(m_CharacterOwner.FindComponent(EquipedLoadoutStorageComponent));
		if (!loadout)
		{
			Print("ARMST FILTER DEBUG: stopped - no EquipedLoadoutStorageComponent on m_CharacterOwner");
			return;
		}

		IEntity mask = loadout.GetClothFromArea(ARMST_FaceArea);
		if (!mask)
		{
			Print("ARMST FILTER DEBUG: stopped - nothing equipped in ARMST_FaceArea");
			return;
		}

		ARMST_ITEMS_STATS_COMPONENTS maskStats = ARMST_ITEMS_STATS_COMPONENTS.Cast(mask.FindComponent(ARMST_ITEMS_STATS_COMPONENTS));
		if (!maskStats)
		{
			Print("ARMST FILTER DEBUG: stopped - equipped face item has no ARMST_ITEMS_STATS_COMPONENTS");
			return;
		}

		Print("ARMST FILTER DEBUG: condition before repair = " + maskStats.GetConditionLevel());
		maskStats.RepairCondition(m_fFilterRepairPercent);
		Print("ARMST FILTER DEBUG: condition after repair = " + maskStats.GetConditionLevel());
	}
}