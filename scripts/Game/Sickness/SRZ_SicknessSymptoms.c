// The symptoms, DayZ's SymptomManager equivalent. The server picks when a sneeze, cough, fit, vomit
// or shiver happens; sounds are broadcast so everyone nearby hears them, shakes and messages go to the owner.
modded class SCR_ChimeraCharacter
{
	//------------------------------------------------------------------------------------------------
	void SRZ_SendSickSfx(int kind)
	{
		Rpc(RpcDo_SRZSickSfx, kind);
		RpcDo_SRZSickSfx(kind);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_SRZSickSfx(int kind)
	{
		SRZ_SicknessSymptoms.PlaySfx(this, kind);
	}

	//------------------------------------------------------------------------------------------------
	void SRZ_SendSickMessage(string text)
	{
		if (this == SCR_PlayerController.GetLocalControlledEntity())
			RpcDo_SRZSickMessage(text);
		else
			Rpc(RpcDo_SRZSickMessage, text);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RpcDo_SRZSickMessage(string text)
	{
		SRZ_SicknessSymptoms.ShowMessage(text);
	}

	//------------------------------------------------------------------------------------------------
	void SRZ_SendSickHud(int tempBand, int wet, int sick, bool draining)
	{
		if (this == SCR_PlayerController.GetLocalControlledEntity())
			RpcDo_SRZSickHud(tempBand, wet, sick, draining);
		else
			Rpc(RpcDo_SRZSickHud, tempBand, wet, sick, draining);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RpcDo_SRZSickHud(int tempBand, int wet, int sick, bool draining)
	{
		SRZ_StatusHud.SetSickness(tempBand, wet, sick, draining);
	}
}

class SRZ_SicknessSymptoms
{
	static const int SFX_SNEEZE = 0;
	static const int SFX_COUGH = 1;
	static const int SFX_COUGHFIT = 2;
	static const int SFX_VOMIT = 3;

	protected static const ResourceName SNEEZE_SOUND = "{0138423D04EC8FC0}Sounds/SRZ/Sickness/SRZ_Sneeze.acp";
	protected static const ResourceName COUGH_SOUND = "{9A50862FFB25C058}Sounds/SRZ/Sickness/SRZ_Cough.acp";
	protected static const ResourceName COUGHFIT_SOUND = "{05FF33DE3AACBA58}Sounds/SRZ/Sickness/SRZ_CoughFit.acp";
	protected static const ResourceName VOMIT_ASI = "{1EA686B6E197CC1A}Anims/SRZ/SRZ_Vomit.asi";
	protected static const ResourceName VOMIT_AGR = "{D701670260215BD7}Anims/workspaces/CUSTOM/CUSTOM_ANIMATION.agr";

	static const float VOMIT_TIME = 4;
	static const float VOMIT_FOOD = 6;
	static const float VOMIT_WATER = 10;

	// Seconds between symptoms, picked at random inside the range
	static const float SNEEZE_MIN = 40;
	static const float SNEEZE_MAX = 90;
	static const float COUGH_MIN = 30;
	static const float COUGH_MAX = 70;
	static const float FIT_MIN = 60;
	static const float FIT_MAX = 120;
	static const float VOMIT_MIN = 90;
	static const float VOMIT_MAX = 150;
	static const float RAD_VOMIT_MIN = 240;
	static const float RAD_VOMIT_MAX = 480;
	static const float MESSAGE_GAP = 120;

	//------------------------------------------------------------------------------------------------
	static void Update(SRZ_SicknessStats st, int tempBand, float now)
	{
		SCR_ChimeraCharacter chimera = SCR_ChimeraCharacter.Cast(st.m_Char);
		if (!chimera)
			return;

		ARMST_PLAYER_STATS_COMPONENT stats = ARMST_PLAYER_STATS_COMPONENT.Cast(st.m_Char.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
		int coldStage = SRZ_SicknessAgents.ColdStage(st);
		int salmStage = SRZ_SicknessAgents.SalmonellaStage(st);
		bool radStage1 = SRZ_SicknessModifiers.IsRadStage1(stats);
		bool codeine = st.m_fCodeine > 0;
		float gap = 1;
		if (codeine)
			gap = 2;

		// Shivering: light shake when cold, heavy when freezing. Codeine hides it.
		if (!codeine && tempBand == SRZ_Sickness.BAND_COLD)
			chimera.ArmstCameraShake(0.04, 0.12, 0.1, 0.6, 0.4);
		else if (!codeine && tempBand <= SRZ_Sickness.BAND_FREEZING)
			chimera.ArmstCameraShake(0.08, 0.25, 0.1, 1.2, 0.5);

		// Fever throb at influenza and above
		if (coldStage >= 2 && Math.RandomFloat01() < 0.3)
			chimera.ArmstCameraShake(0.03, 0.08, 0.6, 0.8, 0.8);

		// Sneezing: a cold and the flu
		if (coldStage >= 1 && coldStage <= 2 && now >= st.m_fNextSneeze)
		{
			st.m_fNextSneeze = now + Math.RandomFloat(SNEEZE_MIN, SNEEZE_MAX) * gap;
			chimera.SRZ_SendSickSfx(SFX_SNEEZE);
		}

		// Coughing: the flu, radiation stage 1. Fits at pneumonia.
		if ((coldStage == 2 || radStage1) && now >= st.m_fNextCough)
		{
			st.m_fNextCough = now + Math.RandomFloat(COUGH_MIN, COUGH_MAX) * gap;
			chimera.SRZ_SendSickSfx(SFX_COUGH);
		}
		else if (coldStage == 3 && now >= st.m_fNextCough)
		{
			st.m_fNextCough = now + Math.RandomFloat(FIT_MIN, FIT_MAX) * gap;
			chimera.SRZ_SendSickSfx(SFX_COUGHFIT);
			chimera.ArmstCameraShake(0.1, 0.3, 0.2, 2, 0.6);
		}

		// Vomiting: salmonella while there's something in the stomach, the odd one from radiation
		if (stats && now >= st.m_fNextVomit)
		{
			bool vomit = false;
			if (salmStage >= 1 && stats.ArmstPlayerStatGetEat() > 5)
			{
				st.m_fNextVomit = now + Math.RandomFloat(VOMIT_MIN, VOMIT_MAX);
				vomit = true;
			}
			else if (radStage1)
			{
				st.m_fNextVomit = now + Math.RandomFloat(RAD_VOMIT_MIN, RAD_VOMIT_MAX);
				vomit = true;
			}

			if (vomit)
				Vomit(st, stats);
		}

		Messages(st, chimera, tempBand, coldStage, salmStage, now);
	}

	//------------------------------------------------------------------------------------------------
	static void Vomit(SRZ_SicknessStats st, ARMST_PLAYER_STATS_COMPONENT stats)
	{
		SCR_ChimeraCharacter chimera = SCR_ChimeraCharacter.Cast(st.m_Char);
		if (!chimera || chimera.IsInVehicle())
			return;

		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(chimera.GetCharacterController());
		if (!controller || controller.IsDead() || controller.IsUnconscious() || controller.IsLoitering())
			return;

		ARMST_PLAYER_ANIMATION anim = ARMST_PLAYER_ANIMATION.Cast(chimera.FindComponent(ARMST_PLAYER_ANIMATION));
		if (anim)
			anim.StartAnimationsPlayerCustomAGR(VOMIT_AGR, VOMIT_ASI, false, true, VOMIT_TIME);

		chimera.SRZ_SendSickSfx(SFX_VOMIT);
		chimera.ArmstCameraShake(0.15, 0.4, 0.3, 2.5, 0.8);

		if (stats)
		{
			stats.ArmstPlayerStatSetEat(-VOMIT_FOOD);
			stats.ArmstPlayerStatSetWater(-VOMIT_WATER);
		}

		SRZ_SicknessAgents.Vomited(st);
	}

	//------------------------------------------------------------------------------------------------
	// DayZ wording, only when something changes, never more than one every two minutes
	protected static void Messages(SRZ_SicknessStats st, SCR_ChimeraCharacter chimera, int tempBand, int coldStage, int salmStage, float now)
	{
		string text;

		if (tempBand != st.m_iLastTempBand)
		{
			if (tempBand == SRZ_Sickness.BAND_COLD) text = "I feel cold";
			else if (tempBand == SRZ_Sickness.BAND_FREEZING) text = "I'm freezing";
			else if (tempBand == SRZ_Sickness.BAND_HYPOTHERMIA) text = "I can't stop shaking, I need to get warm";
			else if (tempBand == SRZ_Sickness.BAND_HOT) text = "I feel hot";
			else if (tempBand == SRZ_Sickness.BAND_HEATSTROKE) text = "I'm overheating";
			else if (st.m_iLastTempBand != SRZ_Sickness.BAND_COMFORT) text = "I feel comfortable again";
			st.m_iLastTempBand = tempBand;
		}

		if (coldStage != st.m_iLastColdStage)
		{
			if (st.m_iLastColdStage >= 0)
			{
				if (coldStage == 1 && st.m_iLastColdStage < 1) text = "I think I'm catching a cold";
				else if (coldStage == 2 && st.m_iLastColdStage < 2) text = "I'm feeling sick";
				else if (coldStage == 3) text = "I can't breathe properly";
				else if (coldStage < st.m_iLastColdStage) text = "I'm feeling a bit better";
			}
			st.m_iLastColdStage = coldStage;
		}

		if (salmStage != st.m_iLastSalmStage)
		{
			if (st.m_iLastSalmStage >= 0)
			{
				if (salmStage == 1 && st.m_iLastSalmStage < 1) text = "My stomach doesn't feel right";
				else if (salmStage == 2) text = "I feel really sick";
				else if (salmStage == 0) text = "My stomach has settled";
			}
			st.m_iLastSalmStage = salmStage;
		}

		if (text.IsEmpty() || now < st.m_fNextMessage)
			return;

		st.m_fNextMessage = now + MESSAGE_GAP;
		chimera.SRZ_SendSickMessage(text);
	}

	//------------------------------------------------------------------------------------------------
	// Runs on every client near the player
	static void PlaySfx(IEntity character, int kind)
	{
		if (System.IsConsoleApp() || RplSession.Mode() == RplMode.Dedicated || !character)
			return;

		vector mat[4];
		character.GetWorldTransform(mat);

		ResourceName acp;
		string soundEvent;
		switch (kind)
		{
			case SFX_SNEEZE:
				acp = SNEEZE_SOUND;
				soundEvent = "SOUND_SRZ_SNEEZE";
				break;
			case SFX_COUGH:
				acp = COUGH_SOUND;
				soundEvent = "SOUND_SRZ_COUGH";
				break;
			default:
				// Vomit uses the fit until a vomit recording is in
				acp = COUGHFIT_SOUND;
				soundEvent = "SOUND_SRZ_COUGHFIT";
				break;
		}

		AudioSystem.PlayEvent(acp, soundEvent, mat, new array<string>(), new array<float>());
	}

	//------------------------------------------------------------------------------------------------
	static void ShowMessage(string text)
	{
		SCR_HintManagerComponent hints = SCR_HintManagerComponent.GetInstance();
		if (hints)
			hints.ShowCustomHint(text, "", 4);
	}
}
