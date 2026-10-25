// Whisper, speak and shout. Each mode is its own VoN component on the character with its own sound range.
// Range method based on VoN immersion tweaks (APL).
class SRZ_VoNWhisperComponentClass : SCR_VoNComponentClass
{
}

class SRZ_VoNWhisperComponent : SCR_VoNComponent
{
}

class SRZ_VoNSpeakComponentClass : SCR_VoNComponentClass
{
}

class SRZ_VoNSpeakComponent : SCR_VoNComponent
{
}

class SRZ_VoNShoutComponentClass : SCR_VoNComponentClass
{
}

class SRZ_VoNShoutComponent : SCR_VoNComponent
{
}

modded class SCR_VONController
{
	protected static const string SRZ_ACTION_UP = "SRZ_VoiceRangeUp";
	protected static const string SRZ_ACTION_DOWN = "SRZ_VoiceRangeDown";
	protected static const string SRZ_VOICE_FILE = "$profile:SRZ_VoiceMode.txt";
	protected static const ResourceName SRZ_VOICE_LAYOUT = "{5A3E7C21D4B98F06}UI/layouts/HUD/SRZ_VoiceRange.layout";
	protected static const int SRZ_MODE_WHISPER = 0;
	protected static const int SRZ_MODE_SPEAK = 1;
	protected static const int SRZ_MODE_SHOUT = 2;
	protected static const float SRZ_IDLE_OPACITY = 0.45;

	protected int m_iSRZVoiceMode = SRZ_MODE_SPEAK;
	protected bool m_bSRZVoiceListening;
	protected bool m_bSRZWarned;
	protected int m_iSRZSetupTries;
	protected Widget m_wSRZVoiceRoot;
	protected RichTextWidget m_wSRZVoiceText;
	protected ImageWidget m_wSRZVoiceIcon;

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		GetGame().GetCallqueue().Remove(SRZ_TrySetup);
		GetGame().GetCallqueue().CallLater(SRZ_TrySetup, 2000, false);
	}

	//------------------------------------------------------------------------------------------------
	override protected void Init(IEntity owner)
	{
		super.Init(owner);

		m_iSRZSetupTries = 0;
		GetGame().GetCallqueue().Remove(SRZ_TrySetup);
		GetGame().GetCallqueue().CallLater(SRZ_TrySetup, 1000, false);
	}

	//------------------------------------------------------------------------------------------------
	// The local player controller isn't known yet during Init, so wait for it
	protected void SRZ_TrySetup()
	{
		PlayerController local = GetGame().GetPlayerController();
		if (!local)
		{
			m_iSRZSetupTries++;
			if (m_iSRZSetupTries < 60)
				GetGame().GetCallqueue().CallLater(SRZ_TrySetup, 1000, false);

			return;
		}

		if (local != GetOwner())
			return;

		SRZ_LoadVoiceMode();

		if (!m_bSRZVoiceListening)
		{
			InputManager input = GetGame().GetInputManager();
			input.AddActionListener(SRZ_ACTION_UP, EActionTrigger.DOWN, SRZ_OnVoiceUp);
			input.AddActionListener(SRZ_ACTION_DOWN, EActionTrigger.DOWN, SRZ_OnVoiceDown);
			m_bSRZVoiceListening = true;
		}

		SRZ_CreateVoiceWidget();
		SRZ_ApplyVoiceMode();
		GetGame().GetCallqueue().Remove(SRZ_UpdateVoiceWidget);
		GetGame().GetCallqueue().CallLater(SRZ_UpdateVoiceWidget, 1000, true);
		Print("[SRZ Voice] Ready, mode " + m_iSRZVoiceMode);
	}

	//------------------------------------------------------------------------------------------------
	override protected void Cleanup()
	{
		super.Cleanup();

		if (m_bSRZVoiceListening)
		{
			InputManager input = GetGame().GetInputManager();
			input.RemoveActionListener(SRZ_ACTION_UP, EActionTrigger.DOWN, SRZ_OnVoiceUp);
			input.RemoveActionListener(SRZ_ACTION_DOWN, EActionTrigger.DOWN, SRZ_OnVoiceDown);
			m_bSRZVoiceListening = false;
		}

		GetGame().GetCallqueue().Remove(SRZ_TrySetup);
		GetGame().GetCallqueue().Remove(SRZ_ApplyVoiceMode);
		GetGame().GetCallqueue().Remove(SRZ_UpdateVoiceWidget);
		GetGame().GetCallqueue().Remove(SRZ_DimVoiceWidget);

		if (m_wSRZVoiceRoot)
			m_wSRZVoiceRoot.RemoveFromHierarchy();

		m_wSRZVoiceRoot = null;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnControlledEntityChanged(IEntity from, IEntity to)
	{
		super.OnControlledEntityChanged(from, to);

		if (m_bSRZVoiceListening)
			GetGame().GetCallqueue().CallLater(SRZ_ApplyVoiceMode, 500, false);
	}

	//------------------------------------------------------------------------------------------------
	protected void SRZ_OnVoiceUp(float value, EActionTrigger reason)
	{
		SRZ_ChangeVoiceMode(1);
	}

	//------------------------------------------------------------------------------------------------
	protected void SRZ_OnVoiceDown(float value, EActionTrigger reason)
	{
		SRZ_ChangeVoiceMode(-1);
	}

	//------------------------------------------------------------------------------------------------
	protected void SRZ_ChangeVoiceMode(int step)
	{
		if (SRZ_IsTransmitting())
			return;

		int mode = Math.ClampInt(m_iSRZVoiceMode + step, SRZ_MODE_WHISPER, SRZ_MODE_SHOUT);
		if (mode != m_iSRZVoiceMode)
		{
			m_iSRZVoiceMode = mode;
			SRZ_SaveVoiceMode();
			SRZ_ApplyVoiceMode();
		}

		SRZ_ShowVoiceWidget();
	}

	//------------------------------------------------------------------------------------------------
	protected bool SRZ_IsTransmitting()
	{
		SCR_VoNComponent comp = GetVONComponent();
		return comp && comp.IsTransmiting();
	}

	//------------------------------------------------------------------------------------------------
	protected typename SRZ_GetModeComponent()
	{
		if (m_iSRZVoiceMode == SRZ_MODE_WHISPER)
			return SRZ_VoNWhisperComponent;

		if (m_iSRZVoiceMode == SRZ_MODE_SHOUT)
			return SRZ_VoNShoutComponent;

		return SRZ_VoNSpeakComponent;
	}

	//------------------------------------------------------------------------------------------------
	protected void SRZ_ApplyVoiceMode()
	{
		PlayerController pc = GetGame().GetPlayerController();
		if (!pc)
			return;

		IEntity character = pc.GetControlledEntity();
		if (!character)
			return;

		SCR_VoNComponent comp = SCR_VoNComponent.Cast(character.FindComponent(SRZ_GetModeComponent()));
		if (!comp)
		{
			if (!m_bSRZWarned)
				Print("[SRZ Voice] Character has no " + SRZ_GetModeComponent().ToString() + ", using the default voice range", LogLevel.WARNING);

			m_bSRZWarned = true;
			return;
		}

		if (GetVONComponent() == comp)
			return;

		if (GetVONComponent())
			ResetVON();

		SetVONComponent(comp);
		Print("[SRZ Voice] Talking through " + comp.Type().ToString());
	}

	//------------------------------------------------------------------------------------------------
	// UI: bottom left, shows the mode and three bars. Bright for a few seconds after a change, then dims.
	protected void SRZ_CreateVoiceWidget()
	{
		if (m_wSRZVoiceRoot)
			return;

		m_wSRZVoiceRoot = GetGame().GetWorkspace().CreateWidgets(SRZ_VOICE_LAYOUT);
		if (!m_wSRZVoiceRoot)
		{
			Print("[SRZ Voice] Could not create HUD layout", LogLevel.WARNING);
			return;
		}

		m_wSRZVoiceText = RichTextWidget.Cast(m_wSRZVoiceRoot.FindAnyWidget("SRZ_VoiceText"));
		m_wSRZVoiceIcon = ImageWidget.Cast(m_wSRZVoiceRoot.FindAnyWidget("SRZ_VoiceIcon"));
		m_wSRZVoiceRoot.SetOpacity(SRZ_IDLE_OPACITY);
		SRZ_UpdateVoiceWidget();
	}

	//------------------------------------------------------------------------------------------------
	protected void SRZ_UpdateVoiceWidget()
	{
		if (!m_wSRZVoiceRoot)
			return;

		PlayerController pc = GetGame().GetPlayerController();
		bool alive = pc && pc.GetControlledEntity();
		m_wSRZVoiceRoot.SetVisible(alive);
		if (!alive)
			return;

		// The game can swap the voice back to its default after a respawn, radio use or menu, so keep putting ours back
		if (!SRZ_IsTransmitting())
			SRZ_ApplyVoiceMode();

		if (!m_wSRZVoiceText)
			return;

		string on = "<color rgba='120,200,90,255'>|</color>";
		string off = "<color rgba='110,110,110,255'>|</color>";
		string bars;
		for (int i = SRZ_MODE_WHISPER; i <= SRZ_MODE_SHOUT; i++)
		{
			if (i <= m_iSRZVoiceMode)
				bars += on;
			else
				bars += off;
		}

		m_wSRZVoiceText.SetText(bars + "  " + SRZ_GetModeText());
	}

	//------------------------------------------------------------------------------------------------
	protected void SRZ_ShowVoiceWidget()
	{
		SRZ_UpdateVoiceWidget();
		if (!m_wSRZVoiceRoot)
			return;

		m_wSRZVoiceRoot.SetOpacity(1);
		GetGame().GetCallqueue().Remove(SRZ_DimVoiceWidget);
		GetGame().GetCallqueue().CallLater(SRZ_DimVoiceWidget, 3000, false);
	}

	//------------------------------------------------------------------------------------------------
	protected void SRZ_DimVoiceWidget()
	{
		if (m_wSRZVoiceRoot)
			m_wSRZVoiceRoot.SetOpacity(SRZ_IDLE_OPACITY);
	}

	//------------------------------------------------------------------------------------------------
	protected string SRZ_GetModeText()
	{
		if (m_iSRZVoiceMode == SRZ_MODE_WHISPER)
			return "Whisper 6m";

		if (m_iSRZVoiceMode == SRZ_MODE_SHOUT)
			return "Shout 100m";

		return "Speak 25m";
	}

	//------------------------------------------------------------------------------------------------
	protected void SRZ_LoadVoiceMode()
	{
		if (!FileIO.FileExists(SRZ_VOICE_FILE))
			return;

		FileHandle file = FileIO.OpenFile(SRZ_VOICE_FILE, FileMode.READ);
		if (!file)
			return;

		string line;
		file.ReadLine(line);
		file.Close();

		int mode = line.ToInt();
		if (mode >= SRZ_MODE_WHISPER && mode <= SRZ_MODE_SHOUT)
			m_iSRZVoiceMode = mode;
	}

	//------------------------------------------------------------------------------------------------
	protected void SRZ_SaveVoiceMode()
	{
		FileHandle file = FileIO.OpenFile(SRZ_VOICE_FILE, FileMode.WRITE);
		if (!file)
			return;

		file.WriteLine(m_iSRZVoiceMode.ToString());
		file.Close();
	}
}
