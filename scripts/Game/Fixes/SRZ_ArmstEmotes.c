// The surrender emote only played the pose. ARMST's surrender state, which lets others search you, was only set by Game Masters.
// Now the emote turns it on, and it turns off again once you move out of the pose.
modded class SCR_PlayerController
{
	protected bool m_bSRZ_Surrendered;

	//------------------------------------------------------------------------------------------------
	void SRZ_SetSurrender(bool state)
	{
		if (!state && !m_bSRZ_Surrendered)
			return;

		m_bSRZ_Surrendered = state;
		Rpc(RpcAsk_SRZSurrender, state);

		GetGame().GetCallqueue().Remove(SRZ_CheckSurrender);
		if (state)
			GetGame().GetCallqueue().CallLater(SRZ_CheckSurrender, 2000, false);
	}

	//------------------------------------------------------------------------------------------------
	protected void SRZ_CheckSurrender()
	{
		if (!m_bSRZ_Surrendered)
			return;

		ChimeraCharacter character = ChimeraCharacter.Cast(GetControlledEntity());
		SCR_CharacterControllerComponent controller;
		if (character)
			controller = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());

		if (!controller || controller.IsDead() || !controller.IsLoitering())
		{
			SRZ_SetSurrender(false);
			return;
		}

		GetGame().GetCallqueue().CallLater(SRZ_CheckSurrender, 500, false);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_SRZSurrender(bool state)
	{
		IEntity character = GetControlledEntity();
		if (!character)
			return;

		ARMST_PLAYER_STATS_COMPONENT stats = ARMST_PLAYER_STATS_COMPONENT.Cast(character.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
		if (stats)
			stats.ArmstPlayerSetSurrender(state);
	}
}

// Plays a picked ARMST emote straight away, since the base game only runs menu commands for squads.
modded class SCR_PlayerControllerCommandingComponent
{
	protected static const string SRZ_SURRENDER_EMOTE = "armst_anim_Surrender";

	//------------------------------------------------------------------------------------------------
	override protected void OnRadialMenuPerformed(SCR_SelectionMenu menu, SCR_SelectionMenuEntry entry)
	{
		SCR_CommandingManagerComponent manager = SCR_CommandingManagerComponent.GetInstance();
		if (entry && manager)
		{
			Print("[SRZ Emotes] Picked " + entry.GetId());
			SCR_ARMST_ANIM_BaseCommand emote = SCR_ARMST_ANIM_BaseCommand.Cast(manager.FindCommand(entry.GetId()));
			if (emote)
			{
				Print("[SRZ Emotes] Playing " + entry.GetId());
				emote.Execute(null, null, vector.Zero, SCR_PlayerController.GetLocalPlayerId(), true);

				SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerController());
				if (pc)
					pc.SRZ_SetSurrender(entry.GetId() == SRZ_SURRENDER_EMOTE);

				return;
			}
		}

		super.OnRadialMenuPerformed(menu, entry);
	}
}

// ARMST builds custom animations with the graph stored on the component, not the one it was given.
// That stored graph is only filled in when the prefab names a start animation, which player prefabs don't,
// so the character got locked in an empty animation. Fill it in before ARMST starts one.
modded class ARMST_PLAYER_ANIMATION
{
	protected static const ResourceName SRZ_DEFAULT_GRAPH = "{D701670260215BD7}Anims/workspaces/CUSTOM/CUSTOM_ANIMATION.agr";

	//------------------------------------------------------------------------------------------------
	protected void SRZ_EnsureGraph(ResourceName graph)
	{
		if (!graph.IsEmpty())
			m_sCustomAnimAGR = graph;

		if (m_sCustomAnimAGR.IsEmpty())
			m_sCustomAnimAGR = SRZ_DEFAULT_GRAPH;
	}

	//------------------------------------------------------------------------------------------------
	override void StartAnimationsPlayerCustomAGR(ResourceName m_sCustomAnimAGRs, ResourceName m_sCustomAnimASIs, bool m_bHoldWeapons, bool m_bDisableControls, float timer = 0)
	{
		SRZ_EnsureGraph(m_sCustomAnimAGRs);
		super.StartAnimationsPlayerCustomAGR(m_sCustomAnimAGRs, m_sCustomAnimASIs, m_bHoldWeapons, m_bDisableControls, timer);
	}

	//------------------------------------------------------------------------------------------------
	override void StartAnimationsPlayer(ResourceName m_sCustomAnimASIs, bool m_bHoldWeapons, bool m_bDisableControls, float timer = 0)
	{
		SRZ_EnsureGraph(ResourceName.Empty);
		super.StartAnimationsPlayer(m_sCustomAnimASIs, m_bHoldWeapons, m_bDisableControls, timer);
	}
}

