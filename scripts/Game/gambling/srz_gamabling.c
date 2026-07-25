// ============================================================================
// SRZ_GambleDiceAction.c
//
// Standalone custom "higher or lower" dice gambling action. Attach to any
// object's ActionsManagerComponent. Add two instances - one HIGHER, one
// LOWER - same object.
//
// Mechanic: rolls a "current" number and a "next" number (1 to m_iDiceSides,
// default 6). HIGHER wins if next > current. LOWER wins if next < current.
// A tie always loses (house edge).
//
// Money: ARMST_PLAYER_STATS_COMPONENT.GetValue() / .SetValue(int)
// Result: private popup via SRZ_RPNet.SendToPlayer(playerId, message)
// ============================================================================

enum SRZ_GambleDicePrediction
{
	HIGHER,
	LOWER
}

[BaseContainerProps(configRoot: true)]
class SRZ_GambleDiceAction : ScriptedUserAction
{
	[Attribute("1000", UIWidgets.EditBox, "Bet amount", category: "Gambling")]
	protected int m_iBetAmount;

	[Attribute("0", UIWidgets.ComboBox, "Prediction", "", ParamEnumArray.FromEnum(SRZ_GambleDicePrediction), category: "Gambling")]
	protected SRZ_GambleDicePrediction m_ePrediction;

	[Attribute("6", UIWidgets.Slider, "Dice sides (1 to this number)", params: "2 100 1", category: "Gambling")]
	protected int m_iDiceSides;

	[Attribute("30", UIWidgets.Slider, "Cooldown seconds", params: "0 120 1", category: "Gambling")]
	protected int m_iCooldownSeconds;

	protected bool m_bOnCooldown = false;

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		string predictionName;
		if (m_ePrediction == SRZ_GambleDicePrediction.HIGHER)
			predictionName = "Higher";
		else
			predictionName = "Lower";

		outName = string.Format("Gamble %1 - Dice %2", m_iBetAmount, predictionName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!Replication.IsServer())
			return;

		if (!pUserEntity)
			return;

		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!playerManager)
			return;

		int playerId = playerManager.GetPlayerIdFromControlledEntity(pUserEntity);
		if (playerId <= 0)
			return;

		if (m_bOnCooldown)
		{
			SRZ_RPNet.SendToPlayer(playerId, "Slow down - wait before gambling again.");
			return;
		}

		ARMST_PLAYER_STATS_COMPONENT statsComp = ARMST_PLAYER_STATS_COMPONENT.Cast(pUserEntity.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
		if (!statsComp)
			return;

		int currentMoney = statsComp.GetValue();
		if (currentMoney < m_iBetAmount)
		{
			SRZ_RPNet.SendToPlayer(playerId, string.Format("You need %1 roubles. You have %2.", m_iBetAmount, currentMoney));
			return;
		}

		m_bOnCooldown = true;
		GetGame().GetCallqueue().CallLater(ClearCooldown, m_iCooldownSeconds * 1000, false);

		int firstRoll = Math.RandomInt(1, m_iDiceSides + 1);
		int secondRoll = Math.RandomInt(1, m_iDiceSides + 1);

		bool won;
		if (m_ePrediction == SRZ_GambleDicePrediction.HIGHER)
			won = secondRoll > firstRoll;
		else
			won = secondRoll < firstRoll;

		string predictionName;
		if (m_ePrediction == SRZ_GambleDicePrediction.HIGHER)
			predictionName = "Higher";
		else
			predictionName = "Lower";

		int newMoney;
		string message;

		if (won)
		{
			newMoney = currentMoney + m_iBetAmount;
			statsComp.SetValue(newMoney);
			message = string.Format("Rolled %1, then %2 - bet %3, won! +%4. Balance: %5", firstRoll, secondRoll, predictionName, m_iBetAmount, newMoney);
		}
		else
		{
			newMoney = currentMoney - m_iBetAmount;
			statsComp.SetValue(newMoney);
			message = string.Format("Rolled %1, then %2 - bet %3, lost. -%4. Balance: %5", firstRoll, secondRoll, predictionName, m_iBetAmount, newMoney);
		}

		SRZ_RPNet.SendToPlayer(playerId, message);
	}

	//------------------------------------------------------------------------------------------------
	protected void ClearCooldown()
	{
		m_bOnCooldown = false;
	}
};