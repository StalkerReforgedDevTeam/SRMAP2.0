[BaseContainerProps(category: "Respawn")]
modded class ARMST_BasicSpawnLogic: EPF_BasicSpawnLogic
{
// Trying to fix new players joining and getting teleported to random faction bunkers, they should spawn at a STALKER faction bunker by default.
// This overrides the CreateCharacter function, lets the persistence load up first and then sets player to a default position.
override protected void CreateCharacter(int playerId, string characterPersistenceId)
{ 
super.CreateCharacter(playerId, characterPersistenceId);
IEntity character = m_mLoadingCharacters.Get(playerId);
if (!character)
{
Print("Tried to create Character, but could not find playerId in m_mLoadingCharacters map. " + playerId);
return;
}
vector position, yawPitchRoll;
GetCreationPosition(playerId, characterPersistenceId, position, yawPitchRoll);
character.SetOrigin(position);
character.SetYawPitchRoll(yawPitchRoll); 
} 
}