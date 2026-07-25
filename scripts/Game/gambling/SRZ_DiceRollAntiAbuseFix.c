// scripts/Game/Fixes/SRZ_DiceRollAntiAbuseFix.c

modded class SCR_PlayerController
{
    // Prevents spamming .diceroll to fish for a favorable number before committing
    // to .gamble. Once a roll is active, it's locked permanently - the player must
    // .gamble that exact roll before they're allowed to roll again. No timeout.
    //------------------------------------------------------------------------------------------------
    override void SRZ_ProcessDiceRollCommand(string msg)
    {
        if (!Replication.IsServer())
            return;

        int playerId = GetPlayerId();
        if (playerId <= 0)
            return;

        // Already have a live, unspent roll - refuse to reroll until it's used.
        if (m_iSRZ_CurrentDiceRoll != -1)
        {
            SRZ_RPNet.SendToPlayer(playerId, string.Format("You already rolled %1. Type .gamble with an amount and higher or lower to use it.", m_iSRZ_CurrentDiceRoll));
            return;
        }

        int total = Math.RandomInt(1, 101);
        m_iSRZ_CurrentDiceRoll = total;

        SRZ_RPNet.SendToPlayer(playerId, string.Format("You rolled %1. Now type .gamble with a amount and higher or lower", total));
    }
};