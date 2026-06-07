//------------------------------------------------------------------------------------------------
//! Entity Attribute for character factions (appears in player properties)
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class GME_FactionsCharacterEditorAttribute : GME_BaseFactionsEditorAttribute
{
//------------------------------------------------------------------------------------------------
override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
{
SCR_EditableCharacterComponent editableEntity = SCR_EditableCharacterComponent.Cast(item);
if (!editableEntity)
return null;

return super.ReadVariable(item, manager);
}

//------------------------------------------------------------------------------------------------
override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
{
SCR_EditableCharacterComponent editableEntity = SCR_EditableCharacterComponent.Cast(item);
if (!editableEntity)
return;

super.WriteVariable(item, var, manager, playerID);
}
//------------------------------------------------------------------------------------------------
// Optional: Only show this attribute for entities that have the ARMST component
override bool IsSerializable()
{
return true;
}
}