[BaseContainerProps()]
class SR_RankLootEntry
{
    [Attribute(uiwidget: UIWidgets.ComboBox, desc: "Select the Stalker Rank", enums: ParamEnumArray.FromEnum(SR_STALKER_RANK))]
    SR_STALKER_RANK m_eRank;

    [Attribute(uiwidget: UIWidgets.ResourceNamePicker, desc: "List of starting loadout configs for this rank", params: "conf")]
    ref array<ref ResourceName> m_aLootConfigs;
}


[BaseContainerProps(configRoot: true)]
class SR_MasterRankLootConfig
{
    [Attribute(desc: "List of ranks and their corresponding loadouts")]
    ref array<ref SR_RankLootEntry> m_aRankList;
}
 
