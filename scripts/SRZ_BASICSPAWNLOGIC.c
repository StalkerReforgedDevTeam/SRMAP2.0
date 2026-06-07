modded class ARMST_BasicSpawnLogic : EPF_BasicSpawnLogic
{
override void LoadStartingLoot(IEntity character, int playerId, ARMST_FACTION_LABEL factionKey)
{
array<ref ResourceName> lootConfigs = GetLootConfigForFaction(factionKey, playerId);

if (!lootConfigs || lootConfigs.IsEmpty())
return;

int randomIndex = Math.RandomInt(0, lootConfigs.Count());
ResourceName selected = lootConfigs[randomIndex];

Print(string.Format("[SRZ_LOOT] Loadout variant %1 of %2 selected for faction %3, player %4.", randomIndex + 1, lootConfigs.Count(), factionKey, playerId), LogLevel.NORMAL);

if (!selected.IsEmpty())
Helpers.GiveLoot(character, selected);
}
}
