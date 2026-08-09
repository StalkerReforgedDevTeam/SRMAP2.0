// Scripts/Game/Fixes/SRZ_MutantImmunity_SurgeGamemode.c
// Makes FACTION_MUTANTS immune to surge damage coming from ARMST_SURGE_GamemodeComponent.
// Indoor check corrected to use the real static helper: Helpers.IsPlayerIndoors(player)

modded class ARMST_SURGE_GamemodeComponent
{
	[Attribute("0", UIWidgets.CheckBox, "Mutants: ignore surge FX/sounds", category: "SRZ | MUTANT IMMUNITY")]
	protected bool m_bMutantsIgnoreSurgeFX;

	protected bool IsMutantPlayer(IEntity player)
	{
		if (!player)
			return false;

		ARMST_PLAYER_STATS_COMPONENT stats = ARMST_PLAYER_STATS_COMPONENT.Cast(player.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
		if (!stats)
			return false;

		return stats.GetFactionKey() == ARMST_FACTION_LABEL.FACTION_MUTANTS;
	}

	override protected void ApplyDamageToPlayers()
	{
		if (!m_SurgeActiveDamage)
			return;

		array<int> playerIds = new array<int>();
		GetGame().GetPlayerManager().GetPlayers(playerIds);

		SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		IEntity localControlled;
		if (pc)
			localControlled = pc.GetControlledEntity();
		else
			localControlled = null;

		foreach (int playerId : playerIds)
		{
			IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
			if (!player)
				continue;

			// --- MUTANT IMMUNITY (NO DAMAGE) ---
			if (IsMutantPlayer(player))
				continue;
			// -----------------------------------

			ARMST_PLAYER_STATS_COMPONENT stats = ARMST_PLAYER_STATS_COMPONENT.Cast(player.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
			if (!stats)
				continue;

			bool isIndoors = Helpers.IsPlayerIndoors(player);
			if (!isIndoors)
			{
				stats.ArmstPlayerStatSetPsy(-m_SurgeDamage);

				DamageManagerComponent dmg = DamageManagerComponent.Cast(player.FindComponent(DamageManagerComponent));
				if (dmg)
				{
					BaseDamageContext ctx = new BaseDamageContext();
					ctx.damageType = EDamageType.PROCESSED_FRAGMENTATION;
					ctx.damageValue = m_SurgeDamage;
					dmg.HandleDamage(ctx);
				}

				if (localControlled && localControlled == player)
					pc.ArmstCameraShake(1.5, 2.6, 1, 2, 4);
			}
		}

		GetGame().GetCallqueue().CallLater(ApplyDamageToPlayers, 1000, false);
	}

	override protected void StartSurge()
	{
		float schanse = Math.RandomFloat(0, 99);
		if (schanse < m_ToxicRainChance)
		{
			super.StartSurge();
			return;
		}

		if (!IsSurgeEnabled())
			return;

		m_LightningSpawned = 0;
		m_SurgeActiveStart = true;
		Print("[ARMST_SURGE] START SURGE!!!", LogLevel.NORMAL);

		GetGame().GetCallqueue().CallLater(SurgeNotify, 15 * 1000, false);

		if (timeAndWeatherManager)
			timeAndWeatherManager.ForceWeatherTo(false, "Storm");

		array<int> playerIds = new array<int>();
		GetGame().GetPlayerManager().GetPlayers(playerIds);

		foreach (int playerId : playerIds)
		{
			IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
			if (!player)
				continue;

			if (m_bMutantsIgnoreSurgeFX && IsMutantPlayer(player))
				continue;

			SpawnSurgeSoundStartForPlayer(player);
		}

		GetGame().GetCallqueue().CallLater(ChangeSurgeWeather, m_SurgeTimerAmbient * 1000, false);
	}

	override protected void ChangeSurgeWeather()
	{
		if (timeAndWeatherManager)
		{
			timeAndWeatherManager.ForceWeatherTo(false, "Surge");
			timeAndWeatherManager.SetFogAmountOverride(true, 1);
		}

		array<int> playerIds = new array<int>();
		GetGame().GetPlayerManager().GetPlayers(playerIds);

		foreach (int playerId : playerIds)
		{
			IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
			if (!player)
				continue;

			if (m_bMutantsIgnoreSurgeFX && IsMutantPlayer(player))
				continue;

			SpawnSurgeSoundAmbientForPlayer(player);
			SpawnSurgeEffectsForPlayer(player);
		}

		m_SurgeActiveDamage = true;
		m_SurgeActiveStart = false;

		GetGame().GetCallqueue().CallLater(ApplyDamageToPlayers, 1000, false);
		ScheduleNextLightning();
	}
};