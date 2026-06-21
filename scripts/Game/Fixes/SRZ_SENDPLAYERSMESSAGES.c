// ============================================================================
// SRZ_RuleWarningContextAction
//
// New GM right-click context menu action: "Warn" submenu with rule-violation
// reasons (OOC, NLR, etc.). Replaces the old SRZ_SendPlayersMessageContextAction
// entirely. This is SEPARATE from the .warn chat command (SRZ_RPNet / .warn
// command system) - that one is untouched.
//
// Each reason is its own context action instance (one per submenu entry),
// distinguished by the m_reasonLabel and m_reasonMessage attributes set per
// instance in the editor config/prefab. This avoids any runtime menu-building
// logic, which is the safest way to guarantee entries actually render on a
// dedicated server - the entries exist statically, not conditionally.
//
// NOTE ON BASE CLASS: SCR_EditableEntityContextAction does not exist in this
// codebase (confirmed by compile error: "Unknown type"). The only confirmed-
// real, confirmed-compiling base class available is SCR_HealEntitiesContextAction,
// from the original SRZ_SendPlayersMessageContextAction.c. This file uses that
// base again so it compiles. This means the heal-specific base class is NOT
// the proven cause of the old menu-missing bug - we don't yet know the real
// cause, since we never got to test the IsHosted/SRZ_SendWarning theory in
// isolation. If this entry is still missing after testing live, the next
// thing to check is whatever prefab/config file registers context actions,
// since the class itself now mirrors the original's structure exactly except
// for the RPC routing and submenu attributes.
//
// No IsHosted() branching anywhere - always routes via RPC through SRZ_RPNet,
// exactly like the proven-working .warn command path. This guarantees identical
// behavior in Workbench/hosted testing and on the live dedicated server.
// ============================================================================

[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SRZ_RuleWarningContextAction : SCR_HealEntitiesContextAction
{
	// Label shown in the "Warn" submenu, e.g. "OOC", "NLR"
	[Attribute("OOC", UIWidgets.EditBox, "Short label for this warning reason (shown in submenu)")]
	string m_reasonLabel;

	// Full message sent to the target player and shown on their screen
	[Attribute("You have been warned for an OOC violation. Please review the server rules.", UIWidgets.EditBox, "Full warning message sent to the target player")]
	string m_reasonMessage;

	//------------------------------------------------------------------------------------------------
	override bool CanBeShown(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		if (!selectedEntity)
			return false;

		return selectedEntity.GetEntityType() == EEditableEntityType.CHARACTER;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformed(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		if (!selectedEntity)
			return false;

		return selectedEntity.GetEntityType() == EEditableEntityType.CHARACTER;
	}

	//------------------------------------------------------------------------------------------------
	override void Perform(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition)
	{
		if (!selectedEntity)
			return;

		IEntity owner = selectedEntity.GetOwner();
		if (!owner)
			return;

		PlayerManager pm = GetGame().GetPlayerManager();
		if (!pm)
			return;

		int playerId = pm.GetPlayerIdFromControlledEntity(owner);
		if (playerId <= 0)
			return;

		// Always route to the target via the proven RPC path - identical
		// behavior whether this is run hosted (Workbench) or on a real
		// dedicated server. No environment branching.
		string formattedMessage = string.Format("[RULE WARNING - %1] %2", m_reasonLabel, m_reasonMessage);
		SRZ_RPNet.SendToPlayer(playerId, formattedMessage);

		Print(string.Format("[SRZ_RP] GM issued rule warning (%1) to player ID %2", m_reasonLabel, playerId), LogLevel.NORMAL);
	}
}