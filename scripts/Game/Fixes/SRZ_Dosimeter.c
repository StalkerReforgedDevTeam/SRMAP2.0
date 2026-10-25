// Dosimeter: while it's switched on, a number bottom left shows the radiation where you stand,
// and the clicking speeds up the more radiation you're in.
class SRZ_DosimeterHud
{
	protected static const ResourceName LAYOUT = "{3B7E5D9A1C2F4806}UI/layouts/HUD/SRZ_DosimeterHud.layout";

	protected static Widget s_wRoot;
	protected static TextWidget s_wText;
	protected static Managed s_Owner;

	//------------------------------------------------------------------------------------------------
	static void Show(Managed owner, float level)
	{
		if (!s_wRoot)
		{
			s_wRoot = GetGame().GetWorkspace().CreateWidgets(LAYOUT);
			if (!s_wRoot)
				return;

			s_wText = TextWidget.Cast(s_wRoot.FindAnyWidget("SRZ_DosiText"));
		}

		s_Owner = owner;
		if (s_wText)
			s_wText.SetText(((int)Math.Round(level)).ToString());
	}

	//------------------------------------------------------------------------------------------------
	static void Hide(Managed owner)
	{
		if (owner && s_Owner && owner != s_Owner)
			return;

		if (s_wRoot)
			s_wRoot.RemoveFromHierarchy();

		s_wRoot = null;
		s_wText = null;
		s_Owner = null;
	}
}

modded class ARMST_DETECTOR_COMPONENTS
{
	protected static const float SRZ_CLICK_SLOWEST = 1200;
	protected static const float SRZ_CLICK_FASTEST = 60;

	//------------------------------------------------------------------------------------------------
	// About 0.5s between clicks at 20, 0.33s at 60, 0.15s at 300 and as fast as it goes near 1000
	protected float SRZ_ClickInterval(float level)
	{
		float interval = SRZ_CLICK_SLOWEST / Math.Sqrt(1 + level / 5);
		interval *= Math.RandomFloat(0.7, 1.3);
		return Math.Clamp(interval, SRZ_CLICK_FASTEST, SRZ_CLICK_SLOWEST);
	}

	//------------------------------------------------------------------------------------------------
	override protected void ArmstDetectorCicle()
	{
		super.ArmstDetectorCicle();
		SRZ_UpdateReadout();
	}

	//------------------------------------------------------------------------------------------------
	override void OnToggleActive(bool state)
	{
		super.OnToggleActive(state);

		if (!state)
			SRZ_DosimeterHud.Hide(this);
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		SRZ_DosimeterHud.Hide(this);
		super.OnDelete(owner);
	}

	//------------------------------------------------------------------------------------------------
	protected void SRZ_UpdateReadout()
	{
		if (!m_bLastLightState)
		{
			SRZ_DosimeterHud.Hide(this);
			return;
		}

		IEntity player = SCR_PlayerController.GetLocalControlledEntity();
		if (!player)
			return;

		ARMST_PLAYER_STATS_COMPONENT stats = ARMST_PLAYER_STATS_COMPONENT.Cast(player.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
		if (stats)
			SRZ_DosimeterHud.Show(this, stats.ArmstRadiactiveLevelGet());
	}

	//------------------------------------------------------------------------------------------------
	// Same lights as ARMST, with the new click speed. The remap text is rebuilt each time instead of growing forever.
	override protected void UpdateMaterialBasedFeedback(float levelRad, float protectionSumm, SCR_SoundManagerModule soundManager)
	{
		if (!m_Mesh || !m_Owner)
		{
			GetGame().GetCallqueue().CallLater(ArmstDetectorCicle, SRZ_CLICK_SLOWEST, false);
			return;
		}

		ResourceName red = m_MatRedOn;
		if (protectionSumm > levelRad)
			red = m_MatRedOff;

		ResourceName yellow = m_MatYellowOff;
		if (levelRad > 1)
			yellow = m_MatYellowOn;

		m_RemappingString = string.Format("$remap '%1' '%2';$remap '%3' '%4';", m_Materials[2], yellow, m_Materials[3], red);
		m_Owner.SetObject(m_Mesh, m_RemappingString);

		if (levelRad > 1)
		{
			soundManager.CreateAndPlayAudioSource(GetOwner(), SCR_SoundEvent.DETECTOR_GEIGER_ACTIVE);
			m_Timer = SRZ_ClickInterval(levelRad);
		}
		else
		{
			m_Timer = 2000;
		}

		GetGame().GetCallqueue().CallLater(ArmstDetectorCicle, m_Timer, false);
	}

	//------------------------------------------------------------------------------------------------
	override protected void UpdateUIBasedFeedback(float levelRad, float protectionSumm, SCR_SoundManagerModule soundManager)
	{
		if (m_RadIcon)
		{
			if (levelRad <= 1)
				m_RadIcon.SetColorInt(ARGB(255, 255, 255, 255));
			else if (protectionSumm > levelRad)
				m_RadIcon.SetColorInt(ARGB(255, 255, 255, 0));
			else
				m_RadIcon.SetColorInt(ARGB(255, 255, 0, 0));
		}

		if (m_TextWidget)
			m_TextWidget.SetText(((int)Math.Round(levelRad)).ToString());

		if (levelRad > 1)
		{
			soundManager.CreateAndPlayAudioSource(GetOwner(), SCR_SoundEvent.DETECTOR_GEIGER_ACTIVE);
			m_Timer = SRZ_ClickInterval(levelRad);
		}
		else
		{
			m_Timer = 2000;
		}

		GetGame().GetCallqueue().CallLater(ArmstDetectorCicle, m_Timer, false);
	}
}
