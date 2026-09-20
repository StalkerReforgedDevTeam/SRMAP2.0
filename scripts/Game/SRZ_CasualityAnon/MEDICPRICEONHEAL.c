class RTTZ_MEDIC_HEALING
{
	static const int MIN_PRICE = 2500;
	static const int MAX_PRICE = 10000;

	static int GetPrice(IEntity player)
	{
		if (!player)
			return MIN_PRICE;

		SCR_CharacterDamageManagerComponent damageManager = SCR_CharacterDamageManagerComponent.Cast(player.FindComponent(SCR_CharacterDamageManagerComponent));
		if (!damageManager)
			return MIN_PRICE;

		float health = Math.Clamp(damageManager.GetHealthScaled(), 0, 1);
		float damageFraction = 1 - health;
		int price = Math.Round(damageFraction * MAX_PRICE);

		return Math.Clamp(price, MIN_PRICE, MAX_PRICE);
	}
}

modded class ARMST_DIALOGS_COMPONENT
{
	override void Armst_dialog_Heal(out string PlayerText, out string BotText)
	{
		int localPlayerId = SCR_PlayerController.GetLocalPlayerId();
		IEntity localPlayer = GetGame().GetPlayerManager().GetPlayerControlledEntity(localPlayerId);
		healingPrice = RTTZ_MEDIC_HEALING.GetPrice(localPlayer);

		super.Armst_dialog_Heal(PlayerText, BotText);
	}

	override void HealPlayer(out string PlayerText, out string BotText)
	{
		int localPlayerId = SCR_PlayerController.GetLocalPlayerId();
		IEntity localPlayer = GetGame().GetPlayerManager().GetPlayerControlledEntity(localPlayerId);
		if (!localPlayer)
		{
			PlayerText = "#armst_dialogs_heal_confirm_button";
			BotText = "#armst_dialogs_heal_dont";
			Print("[RTTZ MEDIC] Could not find the local player.", LogLevel.ERROR);
			return;
		}

		healingPrice = RTTZ_MEDIC_HEALING.GetPrice(localPlayer);

		ARMST_PLAYER_STATS_COMPONENT currency = ARMST_PLAYER_STATS_COMPONENT.Cast(localPlayer.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
		if (!currency)
		{
			PlayerText = "#armst_dialogs_heal_confirm_button";
			BotText = "#armst_dialogs_heal_dont";
			Print("[RTTZ MEDIC] Could not find the player currency component.", LogLevel.ERROR);
			return;
		}

		PlayerText = "#armst_dialogs_heal_confirm_button";
		if (healingPrice > currency.GetValue())
		{
			BotText = "#armst_dialogs_heal_dont";
			Print("[RTTZ MEDIC] Not enough money for healing. Required: " + healingPrice + ", available: " + currency.GetValue(), LogLevel.WARNING);
			return;
		}

		BotText = "#armst_dialogs_heal_done";

		SCR_PlayerController controller = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (controller)
			controller.RequestRTTZMedicHeal();
	}
}

modded class SCR_PlayerController
{
	void RequestRTTZMedicHeal()
	{
		if (Replication.IsServer())
		{
			ProcessRTTZMedicHeal(GetMainEntity());
			return;
		}

		Rpc(RpcAsk_RTTZMedicHeal, GetPlayerId());
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_RTTZMedicHeal(int playerId)
	{
		SCR_PlayerController controller = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
		if (!controller)
			return;

		IEntity player = controller.GetMainEntity();
		if (player)
			ProcessRTTZMedicHeal(player);
	}

	protected void ProcessRTTZMedicHeal(IEntity player)
	{
		if (!Replication.IsServer() || !player)
			return;

		SCR_CharacterDamageManagerComponent damageManager = SCR_CharacterDamageManagerComponent.Cast(player.FindComponent(SCR_CharacterDamageManagerComponent));
		ARMST_PLAYER_STATS_COMPONENT currency = ARMST_PLAYER_STATS_COMPONENT.Cast(player.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
		if (!damageManager || !currency)
			return;

		int price = RTTZ_MEDIC_HEALING.GetPrice(player);
		if (price > currency.GetValue())
			return;

		currency.ModifyValue(price, false);
		damageManager.HealPlayer();
	}
}