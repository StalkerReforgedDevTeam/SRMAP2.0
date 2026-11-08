// Smokeable Smokes 3.2.0 replaces the emote start with an old copy that never listens for the emote's end event.
// Emotes and repairs that play through a graph attachment (all ARMST emotes) never finish, so players get stuck
// in the pose while moving freely. This is the game's 1.9 start code, with Smokeable's cigarette prop call kept.
modded class SCR_CharacterCommandLoiter
{
	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		if (!m_pCharacter)
			return;

		m_bWasTag = false;
		if (!m_rplComponent)
			m_rplComponent = m_pCharacter.GetRplComponent();

		if (m_pScrInputCtx.m_iLoiteringType < 0)
		{
			SetFlagFinished(true);
			return;
		}

		if (m_customAnimData)
		{
			if (m_customAnimData.m_sGraphBindingName != string.Empty && m_customAnimData.m_sGraphName != string.Empty)
			{
				SCR_CharacterControllerComponent charController = SCR_CharacterControllerComponent.Cast(m_pCharacter.FindComponent(SCR_CharacterControllerComponent));
				if (charController)
				{
					charController.GetOnAnimationEvent().Remove(OnAnimationEvent);
					charController.GetOnAnimationEvent().Insert(OnAnimationEvent);
					charController.m_OnLifeStateChanged.Remove(OnLifeStateChanged);
					charController.m_OnLifeStateChanged.Insert(OnLifeStateChanged);
				}

				PreAnim_SetAttachment(m_customAnimData.m_sGraphBindingName, m_customAnimData.m_sGraphName, m_customAnimData.m_sGraphInstanceName, "MasterControl");
				m_bUsingGraphAttachment = true;
			}

			if (m_customAnimData.GetLoiterItemID() != SCR_EAnimationItemID.NONE)
				m_customAnimData.SpawnItem(m_pCharacter, m_customAnimData.GetLoiterItemID());
		}

		SwitchState(ELoiterCommandState.LOITERING);
	}
}
