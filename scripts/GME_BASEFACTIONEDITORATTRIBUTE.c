//------------------------------------------------------------------------------------------------
//! Base Entity Attribute for ARMST factions
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class GME_BaseFactionsEditorAttribute : SCR_BasePresetsEditorAttribute
{
//------------------------------------------------------------------------------------------------
override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
{
SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
if (!editableEntity)
return null;

IEntity owner = editableEntity.GetOwner();
if (!owner)
return SCR_BaseEditorAttributeVar.CreateInt(0);

// Get current faction from ARMST component
ARMST_PLAYER_STATS_COMPONENT statsComp = ARMST_PLAYER_STATS_COMPONENT.Cast(owner.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
if (statsComp)
{
ARMST_FACTION_LABEL currentFaction = statsComp.GetFactionKey();
// Convert enum to dropdown index (FACTION_STALKER=1 becomes index 0, etc.)
if (currentFaction >= ARMST_FACTION_LABEL.FACTION_STALKER)
return SCR_BaseEditorAttributeVar.CreateInt(currentFaction - 1);
}

return SCR_BaseEditorAttributeVar.CreateInt(0);
}

//------------------------------------------------------------------------------------------------
override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
{
if (!var)
return;

SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
if (!editableEntity)
return;

IEntity owner = editableEntity.GetOwner();
if (!owner)
return;

int factionIndex = var.GetInt();
// Convert dropdown index to ARMST_FACTION_LABEL
// Index 0 = FACTION_STALKER (1), Index 1 = FACTION_BANDIT (2), etc.
ARMST_FACTION_LABEL newFaction = factionIndex + 1;
ARMST_PLAYER_STATS_COMPONENT statsComp = ARMST_PLAYER_STATS_COMPONENT.Cast(owner.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
if (!statsComp)
{
Print("[GME_Factions] ERROR: ARMST_PLAYER_STATS_COMPONENT not found!", LogLevel.ERROR);
return;
}

// GM runs on server, so we can set directly (same as your SRZ_USER_SET_FACTION)
statsComp.SetFactionKey(newFaction);
Print(string.Format("[GME_Factions] Set faction to %1 for player %2", newFaction, playerID), LogLevel.NORMAL);
}

//------------------------------------------------------------------------------------------------
override int GetEntries(notnull array<ref SCR_BaseEditorAttributeEntry> outEntries)
{
// Order matches ARMST_FACTION_LABEL enum (starting from FACTION_STALKER)
outEntries.Insert(new SCR_BaseEditorAttributeEntryText("Stalker")); // 0 -> FACTION_STALKER
outEntries.Insert(new SCR_BaseEditorAttributeEntryText("Bandit")); // 1 -> FACTION_BANDIT
outEntries.Insert(new SCR_BaseEditorAttributeEntryText("Army")); // 2 -> FACTION_ARMY
outEntries.Insert(new SCR_BaseEditorAttributeEntryText("Monolith")); // 3 -> FACTION_MUTANTS
outEntries.Insert(new SCR_BaseEditorAttributeEntryText("Renegade")); // 4 -> FACTION_RENEGADE
outEntries.Insert(new SCR_BaseEditorAttributeEntryText("Ecologist")); // 5 -> FACTION_SCIENCES
outEntries.Insert(new SCR_BaseEditorAttributeEntryText("Mercenaries")); // 6 -> FACTION_MERCENARIES
outEntries.Insert(new SCR_BaseEditorAttributeEntryText("Freedom")); // 7 -> FACTION_KB
outEntries.Insert(new SCR_BaseEditorAttributeEntryText("Duty")); // 8 -> FACTION_STORM

return outEntries.Count();
}
}