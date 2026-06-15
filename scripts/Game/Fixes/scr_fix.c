modded class SCR_RespawnSystemComponent : ScriptComponent
{
    override void OnPlayerKilled_S(int playerId, IEntity playerEntity, IEntity killerEntity, notnull Instigator killer)
    {
        if (!playerEntity)
            return;

        if (!GetGame().GetGameMode())
            return;

        super.OnPlayerKilled_S(playerId, playerEntity, killerEntity, killer);
    }
}