// Sickness test commands for GMs and admins, chat ".sick help" for the list.
modded class SCR_PlayerController
{
	void SRZ_SendSickCommand(string command)
	{
		if (Replication.IsServer())
			SRZ_ProcessSickCommand(command);
		else
			Rpc(SRZ_RpcAsk_ProcessSickCommand, command);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void SRZ_RpcAsk_ProcessSickCommand(string command)
	{
		SRZ_ProcessSickCommand(command);
	}

	void SRZ_ProcessSickCommand(string text)
	{
		if (!Replication.IsServer())
			return;

		int playerId = GetPlayerId();
		PlayerManager pm = GetGame().GetPlayerManager();
		if (!pm || playerId <= 0)
			return;

		if (!pm.HasPlayerRole(playerId, EPlayerRole.GAME_MASTER) && !pm.HasPlayerRole(playerId, EPlayerRole.ADMINISTRATOR))
		{
			SRZ_RPNet.SendToPlayer(playerId, "GM or Admin access required.");
			return;
		}

		IEntity character = GetControlledEntity();
		if (!character)
			return;

		SRZ_SicknessStats st = SRZ_Sickness.GetState(character);
		if (!st)
		{
			SRZ_RPNet.SendToPlayer(playerId, "No sickness state yet, wait a tick.");
			return;
		}

		array<string> parts = {};
		text.Split(" ", parts, true);
		string cmd = "help";
		if (parts.Count() > 1)
		{
			cmd = parts[1];
			cmd.ToLower();
		}

		float value = 0;
		if (parts.Count() > 2)
			value = parts[2].ToFloat();

		ARMST_PLAYER_STATS_COMPONENT stats = ARMST_PLAYER_STATS_COMPONENT.Cast(character.FindComponent(ARMST_PLAYER_STATS_COMPONENT));

		if (cmd == "info")
		{
			int band = SRZ_Sickness.Band(st.m_fTemperature);
			string info = "temp " + st.m_fTemperature.ToString() + " target " + st.m_fTarget.ToString() + " band " + band.ToString();
			info += " | insul " + st.m_fInsulation.ToString() + " wet " + st.m_fWetness.ToString() + " heat " + st.m_fHeatBuff.ToString();
			SRZ_RPNet.SendToPlayer(playerId, info);
			string info2 = "cold " + st.m_fColdChain.ToString() + " (stage " + SRZ_SicknessAgents.ColdStage(st).ToString() + ")";
			info2 += " salm " + st.m_fSalmonella.ToString() + " incub " + st.m_fIncubation.ToString();
			info2 += " immunity " + SRZ_SicknessAgents.Immunity(st).ToString() + " resist " + st.m_fResistance.ToString();
			SRZ_RPNet.SendToPlayer(playerId, info2);
			string info3 = "fire " + SRZ_SicknessEnvironment.NearLitFire(character).ToString() + " indoors " + Helpers.IsPlayerIndoors(character).ToString();
			info3 += " codeine " + st.m_fCodeine.ToString() + " tetra " + st.m_fTetraLeft.ToString() + " charcoal " + st.m_fCharcoalLeft.ToString();
			SRZ_RPNet.SendToPlayer(playerId, info3);
			return;
		}

		if (cmd == "temp") { st.m_fTemperature = Math.Clamp(value, -100, 100); st.m_fHeatBuff = 0; }
		else if (cmd == "wet") st.m_fWetness = Math.Clamp(value, 0, 100);
		else if (cmd == "cold") st.m_fColdChain = Math.Clamp(value, 0, 300);
		else if (cmd == "salm") { st.m_fSalmonella = Math.Clamp(value, 0, 100); st.m_fIncubation = 0; }
		else if (cmd == "eatbad") SRZ_SicknessAgents.Ingest(st);
		else if (cmd == "rad" && stats) stats.ArmstPlayerStatSetRadioDirect(Math.Clamp(value, 0, 100));
		else if (cmd == "vomit") SRZ_SicknessSymptoms.Vomit(st, stats);
		else if (cmd == "sneeze") SCR_ChimeraCharacter.Cast(character).SRZ_SendSickSfx(SRZ_SicknessSymptoms.SFX_SNEEZE);
		else if (cmd == "cough") SCR_ChimeraCharacter.Cast(character).SRZ_SendSickSfx(SRZ_SicknessSymptoms.SFX_COUGH);
		else if (cmd == "fit") SCR_ChimeraCharacter.Cast(character).SRZ_SendSickSfx(SRZ_SicknessSymptoms.SFX_COUGHFIT);
		else if (cmd == "clear") st.Reset();
		else
		{
			SRZ_RPNet.SendToPlayer(playerId, ".sick info | temp <-100..100> | wet <0..100> | cold <0..300> | salm <0..100> | eatbad | rad <0..100> | vomit | sneeze | cough | fit | clear");
			return;
		}

		SRZ_RPNet.SendToPlayer(playerId, "sick " + cmd + " done");
	}
}
