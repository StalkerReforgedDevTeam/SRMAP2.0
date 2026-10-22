// Shared storage only one faction can open. Contents save through EPF like the ARMST safe.
[ComponentEditorProps(category: "SRZ", description: "Storage only listed factions can open")]
class SRZ_FactionStorageComponentClass : ScriptComponentClass
{
}

class SRZ_FactionStorageComponent : ScriptComponent
{
	[Attribute("0", UIWidgets.ComboBox, "ARMST faction that can open this", "", ParamEnumArray.FromEnum(ARMST_FACTION_LABEL))]
	protected ARMST_FACTION_LABEL m_eFaction;

	[Attribute("", UIWidgets.EditBox, "Name shown to players who can't open it")]
	protected string m_sFactionName;

	//------------------------------------------------------------------------------------------------
	static SRZ_FactionStorageComponent Get(IEntity ent)
	{
		if (!ent)
			return null;

		return SRZ_FactionStorageComponent.Cast(ent.FindComponent(SRZ_FactionStorageComponent));
	}

	//------------------------------------------------------------------------------------------------
	// Uses the ARMST faction, the one players are actually set to on this server
	bool CanUse(IEntity user)
	{
		if (m_eFaction == ARMST_FACTION_LABEL.FACTION_ALL)
			return true;

		if (!user)
			return false;

		ARMST_PLAYER_STATS_COMPONENT stats = ARMST_PLAYER_STATS_COMPONENT.Cast(user.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
		if (!stats)
			return false;

		return stats.GetFactionKey() == m_eFaction;
	}

	//------------------------------------------------------------------------------------------------
	string GetFactionName()
	{
		return m_sFactionName;
	}
}

modded class SCR_OpenStorageAction
{
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		SRZ_FactionStorageComponent factionStorage = SRZ_FactionStorageComponent.Get(GetOwner());
		if (factionStorage && !factionStorage.CanUse(user))
		{
			SetCannotPerformReason(factionStorage.GetFactionName() + " only");
			return false;
		}

		return super.CanBePerformedScript(user);
	}
}
