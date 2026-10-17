// DayZ style condition names instead of percentages, shown on every item in the inventory tooltip (guns included).
// Devices keep their numeric battery line.

class SRZ_ConditionLabel
{
	static string Get(float condition)
	{
		if (condition <= 0)
			return "Ruined";
		if (condition < 30)
			return "Badly Damaged";
		if (condition < 50)
			return "Damaged";
		if (condition < 70)
			return "Worn";
		return "Pristine";
	}

	static bool IsDevice(InventoryItemComponent item)
	{
		ItemAttributeCollection attributes = item.GetAttributes();
		return attributes && attributes.GetCommonType() == ECommonItemType.ARMST_DEVICE_ITEM;
	}
}

class SRZ_ConditionHintUIInfo : SCR_InventoryItemHintUIInfo
{
	override bool CanBeShown(InventoryItemComponent item, SCR_InventorySlotUI focusedSlot)
	{
		if (!item || SRZ_ConditionLabel.IsDevice(item) || item.GetOwner().FindComponent(ARMST_ARTEFACT_PROPERTIES))
			return false;

		return item.GetOwner().FindComponent(ARMST_ITEMS_STATS_COMPONENTS) != null;
	}

	override string GetItemHintName(InventoryItemComponent item)
	{
		ARMST_ITEMS_STATS_COMPONENTS stats = ARMST_ITEMS_STATS_COMPONENTS.Cast(item.GetOwner().FindComponent(ARMST_ITEMS_STATS_COMPONENTS));
		if (!stats)
			return string.Empty;

		return "Condition: " + SRZ_ConditionLabel.Get(stats.GetConditionLevel());
	}
}

modded class SCR_InventoryMenuUI
{
	protected static ref SRZ_ConditionHintUIInfo s_SRZ_ConditionHint;

	override protected void GetGeneralItemHintsInfos(out notnull array<SCR_InventoryItemHintUIInfo> hintsInfo)
	{
		super.GetGeneralItemHintsInfos(hintsInfo);

		if (!s_SRZ_ConditionHint)
			s_SRZ_ConditionHint = new SRZ_ConditionHintUIInfo();

		hintsInfo.Insert(s_SRZ_ConditionHint);
	}
}

// Tooltip cleanup: no numeric condition (the hint above replaces it), no price, no food/med effects
modded class SCR_InventoryUIInfo
{
	override string GetInventoryItemDescription(InventoryItemComponent item)
	{
		string text = super.GetInventoryItemDescription(item);
		if (!item)
			return text;

		text = SRZ_RemoveLine(text, "\n" + Price + ": ");

		if (!SRZ_ConditionLabel.IsDevice(item))
			text = SRZ_RemoveLine(text, "\n \n" + PropertiesConditionLevel + ": ");

		if (item.GetOwner().FindComponent(ARMST_ItemUseComponent) && !item.GetOwner().FindComponent(ARMST_ARTEFACT_PROPERTIES))
			text = SRZ_RemoveBlock(text, "\n" + Effects + ":");

		return text;
	}

	// Removes the marker and the rest of its line
	protected string SRZ_RemoveLine(string text, string marker)
	{
		int start = text.IndexOf(marker);
		if (start == -1)
			return text;

		int end = text.IndexOfFrom(start + marker.Length(), "\n");
		if (end < start)
			end = text.Length();

		return text.Substring(0, start) + text.Substring(end, text.Length() - end);
	}

	// Removes a header line plus the indented lines under it
	protected string SRZ_RemoveBlock(string text, string header)
	{
		int start = text.IndexOf(header);
		if (start == -1)
			return text;

		int length = text.Length();
		int end = start + header.Length();
		while (end + 3 <= length && text.Substring(end, 3) == "\n  ")
		{
			int next = text.IndexOfFrom(end + 1, "\n");
			if (next <= end)
			{
				end = length;
				break;
			}
			end = next;
		}

		return text.Substring(0, start) + text.Substring(end, text.Length() - end);
	}
}

// Weapon inspect line
modded class ARMST_USER_CHECK_COND
{
	override bool GetActionNameScript(out string outName)
	{
		ARMST_ITEMS_STATS_COMPONENTS stats = ARMST_ITEMS_STATS_COMPONENTS.Cast(GetOwner().FindComponent(ARMST_ITEMS_STATS_COMPONENTS));
		if (!stats)
			return false;

		outName = "Condition: " + SRZ_ConditionLabel.Get(stats.GetConditionLevel());
		return true;
	}
}
