// Artifacts hide their properties until a vendor examines them (free, any trader, via dialog).

modded enum ARMST_DIALOGS_SCRIPTS
{
	SRZ_dialog_ExamineArtifacts
}

modded class ARMST_ARTEFACT_PROPERTIES
{
	[RplProp()]
	bool m_bSRZ_Examined;

	void SRZ_SetExamined(bool examined)
	{
		if (!Replication.IsServer() || m_bSRZ_Examined == examined)
			return;

		m_bSRZ_Examined = examined;
		Replication.BumpMe();
	}
}

modded class EPF_ArmstArtefactPropertiesComponentSaveData
{
	bool m_bSRZ_Examined;

	override EPF_EReadResult ReadFrom(IEntity owner, GenericComponent component, EPF_ComponentSaveDataClass attributes)
	{
		EPF_EReadResult result = super.ReadFrom(owner, component, attributes);

		ARMST_ARTEFACT_PROPERTIES props = ARMST_ARTEFACT_PROPERTIES.Cast(component);
		if (props)
			m_bSRZ_Examined = props.m_bSRZ_Examined;

		return result;
	}

	override EPF_EApplyResult ApplyTo(IEntity owner, GenericComponent component, EPF_ComponentSaveDataClass attributes)
	{
		EPF_EApplyResult result = super.ApplyTo(owner, component, attributes);

		ARMST_ARTEFACT_PROPERTIES props = ARMST_ARTEFACT_PROPERTIES.Cast(component);
		if (props)
			props.SRZ_SetExamined(m_bSRZ_Examined);

		return result;
	}

	override bool Equals(notnull EPF_ComponentSaveData other)
	{
		EPF_ArmstArtefactPropertiesComponentSaveData otherData = EPF_ArmstArtefactPropertiesComponentSaveData.Cast(other);
		if (!otherData || m_bSRZ_Examined != otherData.m_bSRZ_Examined)
			return false;

		return super.Equals(other);
	}
}

modded class ARMST_DIALOGS_COMPONENT
{
	protected static const string SRZ_EXAMINE_BUTTON = "Can you take a look at my artifacts?";

	override void LoadDialogConfig()
	{
		super.LoadDialogConfig();

		if (!SRZ_IsTrader())
			return;

		if (m_sDialog_1_Scripts == ARMST_DIALOGS_SCRIPTS.Armst_dialog_none) { m_sDialog_1_Scripts = ARMST_DIALOGS_SCRIPTS.SRZ_dialog_ExamineArtifacts; m_sDialog_1_Text = SRZ_EXAMINE_BUTTON; return; }
		if (m_sDialog_2_Scripts == ARMST_DIALOGS_SCRIPTS.Armst_dialog_none) { m_sDialog_2_Scripts = ARMST_DIALOGS_SCRIPTS.SRZ_dialog_ExamineArtifacts; m_sDialog_2_Text = SRZ_EXAMINE_BUTTON; return; }
		if (m_sDialog_3_Scripts == ARMST_DIALOGS_SCRIPTS.Armst_dialog_none) { m_sDialog_3_Scripts = ARMST_DIALOGS_SCRIPTS.SRZ_dialog_ExamineArtifacts; m_sDialog_3_Text = SRZ_EXAMINE_BUTTON; return; }
		if (m_sDialog_4_Scripts == ARMST_DIALOGS_SCRIPTS.Armst_dialog_none) { m_sDialog_4_Scripts = ARMST_DIALOGS_SCRIPTS.SRZ_dialog_ExamineArtifacts; m_sDialog_4_Text = SRZ_EXAMINE_BUTTON; return; }
		if (m_sDialog_5_Scripts == ARMST_DIALOGS_SCRIPTS.Armst_dialog_none) { m_sDialog_5_Scripts = ARMST_DIALOGS_SCRIPTS.SRZ_dialog_ExamineArtifacts; m_sDialog_5_Text = SRZ_EXAMINE_BUTTON; return; }

		Print("[SRZ_ArtifactExamine] No free dialog slot on trader " + m_Actor, LogLevel.WARNING);
	}

	protected bool SRZ_IsTrader()
	{
		return m_sDialog_1_Scripts == ARMST_DIALOGS_SCRIPTS.Armst_dialog_trader
			|| m_sDialog_2_Scripts == ARMST_DIALOGS_SCRIPTS.Armst_dialog_trader
			|| m_sDialog_3_Scripts == ARMST_DIALOGS_SCRIPTS.Armst_dialog_trader
			|| m_sDialog_4_Scripts == ARMST_DIALOGS_SCRIPTS.Armst_dialog_trader
			|| m_sDialog_5_Scripts == ARMST_DIALOGS_SCRIPTS.Armst_dialog_trader;
	}

	override string GetDefaultButtonText(ARMST_DIALOGS_SCRIPTS script)
	{
		if (script == ARMST_DIALOGS_SCRIPTS.SRZ_dialog_ExamineArtifacts)
			return SRZ_EXAMINE_BUTTON;

		return super.GetDefaultButtonText(script);
	}

	override void Armst_dialog_start_scripts(ARMST_DIALOGS_SCRIPTS scriptName, out string playerText, out string botText)
	{
		if (scriptName != ARMST_DIALOGS_SCRIPTS.SRZ_dialog_ExamineArtifacts)
		{
			super.Armst_dialog_start_scripts(scriptName, playerText, botText);
			return;
		}

		playerText = SRZ_EXAMINE_BUTTON;

		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(SCR_PlayerController.GetLocalPlayerId());
		int count = SRZ_ArtifactExamine.CountUnexamined(player);
		if (count == 0)
		{
			botText = "You've got nothing new for me to look at.";
			return;
		}

		SCR_PlayerController controller = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (controller)
			controller.SRZ_RequestExamineArtifacts();

		if (count == 1)
			botText = "Alright, hand it over... There. Now you know what it does.";
		else
			botText = string.Format("Alright, hand them over... There, all %1 of them. Now you know what they do.", count);
	}
}

class SRZ_ArtifactExamine
{
	static int CountUnexamined(IEntity player)
	{
		array<IEntity> items = {};
		if (!GetItems(player, items))
			return 0;

		int count;
		foreach (IEntity item : items)
		{
			ARMST_ARTEFACT_PROPERTIES props = ARMST_ARTEFACT_PROPERTIES.Cast(item.FindComponent(ARMST_ARTEFACT_PROPERTIES));
			if (props && !props.m_bSRZ_Examined)
				count++;
		}
		return count;
	}

	static void ExamineAll(IEntity player)
	{
		array<IEntity> items = {};
		if (!GetItems(player, items))
			return;

		foreach (IEntity item : items)
		{
			ARMST_ARTEFACT_PROPERTIES props = ARMST_ARTEFACT_PROPERTIES.Cast(item.FindComponent(ARMST_ARTEFACT_PROPERTIES));
			if (props)
				props.SRZ_SetExamined(true);
		}
	}

	protected static bool GetItems(IEntity player, out array<IEntity> items)
	{
		if (!player)
			return false;

		SCR_InventoryStorageManagerComponent inventory = SCR_InventoryStorageManagerComponent.Cast(player.FindComponent(SCR_InventoryStorageManagerComponent));
		return inventory && inventory.GetItems(items);
	}
}

modded class SCR_PlayerController
{
	void SRZ_RequestExamineArtifacts()
	{
		if (Replication.IsServer())
			SRZ_ArtifactExamine.ExamineAll(GetMainEntity());
		else
			Rpc(RpcAsk_SRZ_ExamineArtifacts);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_SRZ_ExamineArtifacts()
	{
		SRZ_ArtifactExamine.ExamineAll(GetMainEntity());
	}
}

// Tooltip: unexamined artifacts show no properties
modded class SCR_InventoryUIInfo
{
	override string GetInventoryItemDescription(InventoryItemComponent item)
	{
		string text = super.GetInventoryItemDescription(item);
		if (!item)
			return text;

		ARMST_ARTEFACT_PROPERTIES props = ARMST_ARTEFACT_PROPERTIES.Cast(item.GetOwner().FindComponent(ARMST_ARTEFACT_PROPERTIES));
		if (!props || props.m_bSRZ_Examined)
			return text;

		text = SRZ_MaskBlockValues(text, "\n" + Effects + ": ");
		return text + "\n\nNot examined. Have a vendor look at it.";
	}

	// Keeps each stat name under the header but replaces its value with ???
	protected string SRZ_MaskBlockValues(string text, string header)
	{
		int start = text.IndexOf(header);
		if (start == -1)
			return text;

		string result = text.Substring(0, start + header.Length());
		int pos = start + header.Length();
		int length = text.Length();

		while (pos + 3 <= length && text.Substring(pos, 3) == "\n  ")
		{
			int lineEnd = text.IndexOfFrom(pos + 1, "\n");
			if (lineEnd <= pos)
				lineEnd = length;

			string line = text.Substring(pos, lineEnd - pos);
			int colon = line.IndexOf(": ");
			if (colon != -1)
				line = line.Substring(0, colon + 2) + "???";

			result += line;
			pos = lineEnd;
		}

		return result + text.Substring(pos, length - pos);
	}
}
