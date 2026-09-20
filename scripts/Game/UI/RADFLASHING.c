[BaseContainerProps(category: "HUDUpdate")]
modded class ARMST_TestHudUpdate : ARMST_HUD_Update
{
	protected const int RADIATION_COLOR_WHITE = ARGB(255, 255, 255, 255);
	protected const int RADIATION_COLOR_YELLOW = ARGB(255, 255, 255, 0);
	protected const int RADIATION_COLOR_RED = ARGB(255, 255, 0, 0);

	protected const float RADIATION_FLASH_HALF_PERIOD = 500;
	protected const float RADIATION_FLASH_MIN_OPACITY = 0.2;

	override void UpdateRadiationIndicator(float value)
	{
		if (!m_Slider_Radiation || !m_Stat_Radiation)
			return;

		if (value <= 0)
		{
			m_Stat_Radiation.SetOpacity(0);
			m_Stat_Radiation.SetVisible(false);
			SetColor(m_Slider_Radiation, RADIATION_COLOR_WHITE);
			SetColor(m_Stat_Radiation, RADIATION_COLOR_WHITE);
			return;
		}

		m_Stat_Radiation.SetVisible(true);

		if (value < 25)
		{
			m_Stat_Radiation.SetOpacity(1);
			SetColor(m_Slider_Radiation, RADIATION_COLOR_WHITE);
			SetColor(m_Stat_Radiation, RADIATION_COLOR_WHITE);
		}
		else if (value < 50)
		{
			m_Stat_Radiation.SetOpacity(1);
			SetColor(m_Slider_Radiation, RADIATION_COLOR_YELLOW);
			SetColor(m_Stat_Radiation, RADIATION_COLOR_YELLOW);
		}
		else if (value < 75)
		{
			m_Stat_Radiation.SetOpacity(1);
			SetColor(m_Slider_Radiation, RADIATION_COLOR_RED);
			SetColor(m_Stat_Radiation, RADIATION_COLOR_RED);
		}
		else
		{
			SetColor(m_Slider_Radiation, RADIATION_COLOR_RED);
			SetColor(m_Stat_Radiation, RADIATION_COLOR_RED);

			int flashIndex = (int)(GetGame().GetWorld().GetWorldTime() / RADIATION_FLASH_HALF_PERIOD) % 2;
			if (flashIndex == 0)
				m_Stat_Radiation.SetOpacity(1);
			else
				m_Stat_Radiation.SetOpacity(RADIATION_FLASH_MIN_OPACITY);
		}
	}
}