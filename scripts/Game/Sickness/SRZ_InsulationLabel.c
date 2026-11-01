// DayZ style insulation label on clothing tooltips: Bad, Low, Medium, High, Best, plus Waterproof.
class SRZ_InsulationLabel
{
	static string Get(float insulation)
	{
		if (insulation > 80) return "Best";
		if (insulation > 60) return "High";
		if (insulation > 40) return "Medium";
		if (insulation > 20) return "Low";
		return "Bad";
	}

	// Nominal value from SRZ_Insulation.conf, before condition and wetness. False when the item isn't listed.
	static bool Nominal(IEntity item, out float value, out bool waterproof)
	{
		value = 0;
		waterproof = false;
		if (!item || !item.GetPrefabData())
			return false;

		SRZ_InsulationEntry entry = SRZ_InsulationConfig.Get(item.GetPrefabData().GetPrefabName());
		if (!entry)
			return false;

		value = entry.m_fInsulation;
		waterproof = entry.m_bWaterproof;
		return true;
	}
}

class SRZ_InsulationHintUIInfo : SCR_InventoryItemHintUIInfo
{
	override bool CanBeShown(InventoryItemComponent item, SCR_InventorySlotUI focusedSlot)
	{
		if (!item || !item.GetOwner())
			return false;

		float value;
		bool waterproof;
		return SRZ_InsulationLabel.Nominal(item.GetOwner(), value, waterproof);
	}

	override string GetItemHintName(InventoryItemComponent item)
	{
		float value;
		bool waterproof;
		SRZ_InsulationLabel.Nominal(item.GetOwner(), value, waterproof);
		string text = "Insulation: " + SRZ_InsulationLabel.Get(value);
		if (waterproof)
			text += ", Waterproof";

		return text;
	}
}

modded class SCR_InventoryMenuUI
{
	protected static ref SRZ_InsulationHintUIInfo s_SRZ_InsulationHint;

	override protected void GetGeneralItemHintsInfos(out notnull array<SCR_InventoryItemHintUIInfo> hintsInfo)
	{
		super.GetGeneralItemHintsInfos(hintsInfo);

		if (!s_SRZ_InsulationHint)
			s_SRZ_InsulationHint = new SRZ_InsulationHintUIInfo();

		hintsInfo.Insert(s_SRZ_InsulationHint);
	}
}
