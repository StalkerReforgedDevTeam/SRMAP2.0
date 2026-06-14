class SRZ_USER_SET_FACTION : ScriptedUserAction
{
// Dropdown in the action’s properties
[Attribute("1", UIWidgets.ComboBox, "Faction to set", "", ParamEnumArray.FromEnum(ARMST_FACTION_LABEL), category: "Faction")]
protected ARMST_FACTION_LABEL m_FactionKey;

// Optional: delete the object after someone uses it (like your example)
[Attribute("0", UIWidgets.CheckBox, "Delete entity after use", "If enabled, the entity this action is on will be deleted after use.", category: "Behavior")]
protected bool m_DeleteOwnerAfterUse = false;

//------------------------------------------------------------------------------------------------
override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
{
if (!pUserEntity)
return;

ARMST_PLAYER_STATS_COMPONENT playerStats =
ARMST_PLAYER_STATS_COMPONENT.Cast(pUserEntity.FindComponent(ARMST_PLAYER_STATS_COMPONENT));

if (!playerStats)
return;

playerStats.SetFactionKey(m_FactionKey);

if (m_DeleteOwnerAfterUse && pOwnerEntity)
SCR_EntityHelper.DeleteEntityAndChildren(pOwnerEntity);
}

//------------------------------------------------------------------------------------------------
override bool GetActionNameScript(out string outName)
{
outName = "Set Faction";
return true;
}

//------------------------------------------------------------------------------------------------
override bool CanBePerformedScript(IEntity user)
{
return true;
}

//------------------------------------------------------------------------------------------------
void ~SRZ_USER_SET_FACTION() {}
};