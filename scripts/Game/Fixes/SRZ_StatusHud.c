// DayZ style status HUD, bottom right. Health, water and food drain and change colour, radiation changes colour with your rad level.
// Pop ups on the left for psy damage, bleeding, broken legs and stims. Stamina bar and server name underneath.
class SRZ_StatusMain
{
	ImageWidget m_wBg;
	ImageWidget m_wFill;
	ref array<ImageWidget> m_aArrows = {};
	float m_fSample = -1;
	float m_fRate;
}

class SRZ_StatusHud
{
	// Hides the old ARMST bars and status icons
	static const bool HIDE_ARMST_HUD = true;

	protected static const ResourceName LAYOUT = "{B51FEBD2BDB8B5A5}UI/layouts/HUD/SRZ_StatusHud.layout";

	protected static const float RIGHT = 22;
	protected static const float BOTTOM = 18;
	protected static const float ICON = 34;
	protected static const float POPUP = 30;
	protected static const float SLOT = 46;
	protected static const float SLOT_GAP = 16;

	protected static const float RAD_YELLOW = 40;
	protected static const float RAD_RED = 70;
	protected static const float RAD_FLASH = 90;
	protected static const float PSY_SHOW_TIME = 6;
	protected static const float PSY_HEAVY_DROP = 5;
	protected static const float TREND_SAMPLE = 2;

	protected static const int WHITE = ARGB(255, 242, 241, 236);
	protected static const int YELLOW = ARGB(255, 233, 197, 72);
	protected static const int RED = ARGB(255, 217, 65, 47);
	protected static const int SHADOW = ARGB(120, 0, 0, 0);
	protected static const int STAM_BG = ARGB(140, 0, 0, 0);
	protected static const int SEP = ARGB(64, 242, 241, 236);
	protected static const int BRAND = ARGB(153, 242, 241, 236);

	protected static Widget s_wRoot;
	protected static ref array<ref SRZ_StatusMain> s_aMain = {};
	protected static ImageWidget s_wSep;
	protected static ImageWidget s_wBrain;
	protected static ImageWidget s_wBleed;
	protected static ImageWidget s_wBone;
	protected static ImageWidget s_wStim;
	protected static ImageWidget s_wStamBg;
	protected static ImageWidget s_wStamFill;
	protected static TextWidget s_wStimText;
	protected static TextWidget s_wBrand;

	protected static float s_fLastTick;
	protected static float s_fLastUpdate;
	protected static float s_fLastSample;
	protected static float s_fLastPsy = -1;
	protected static float s_fLastPsyDrop = -100;
	protected static ref array<float> s_aPsyDropTimes = {};
	protected static ref array<float> s_aPsyDrops = {};
	protected static int s_iLegState;
	protected static bool s_bWatching;

	//------------------------------------------------------------------------------------------------
	protected static ResourceName Tex(string name)
	{
		switch (name)
		{
			case "ArrowDown": return "{911B49857AE4E925}UI/Textures/SRZ/HUD/SRZ_Hud_ArrowDown.edds";
			case "ArrowUp": return "{DF1E78EECBBA8096}UI/Textures/SRZ/HUD/SRZ_Hud_ArrowUp.edds";
			case "Bleed": return "{4F67C8CF0BDCDA38}UI/Textures/SRZ/HUD/SRZ_Hud_Bleed.edds";
			case "Bone": return "{4C46457E2F1ED0AE}UI/Textures/SRZ/HUD/SRZ_Hud_Bone.edds";
			case "Brain": return "{76782DC51363EB42}UI/Textures/SRZ/HUD/SRZ_Hud_Brain.edds";
			case "FillMask": return "{BA63BBE4CFEEE254}UI/Textures/SRZ/HUD/SRZ_Hud_FillMask.edds";
			case "Food": return "{68559D647ADE110E}UI/Textures/SRZ/HUD/SRZ_Hud_Food.edds";
			case "Health": return "{8E9C5C1C4430C935}UI/Textures/SRZ/HUD/SRZ_Hud_Health.edds";
			case "Rad": return "{8D24770D757CCD40}UI/Textures/SRZ/HUD/SRZ_Hud_Rad.edds";
			case "Stim": return "{C5A720677798A4A4}UI/Textures/SRZ/HUD/SRZ_Hud_Stim.edds";
			case "Water": return "{E8E7284604754140}UI/Textures/SRZ/HUD/SRZ_Hud_Water.edds";
			case "White": return "{BF04947D531991A2}UI/Textures/SRZ/HUD/SRZ_Hud_White.edds";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	static void SetLegState(int state)
	{
		s_iLegState = state;
	}

	//------------------------------------------------------------------------------------------------
	// Called from the ARMST HUD every frame on the player's own machine
	static void Tick(ARMST_PLAYER_STATS_COMPONENT stats, SCR_CharacterDamageManagerComponent damage)
	{
		if (!stats || !GetGame().GetWorld())
			return;

		IEntity owner = stats.GetOwner();
		if (!owner || owner != SCR_PlayerController.GetLocalControlledEntity())
			return;

		float now = GetGame().GetWorld().GetWorldTime() / 1000;
		s_fLastTick = now;
		if (now - s_fLastUpdate < 0.1)
			return;

		s_fLastUpdate = now;
		if (!Create())
			return;

		bool show = stats.m_hud_check && !IsBlocked();
		s_wRoot.SetVisible(show);
		if (!show)
			return;

		bool blinkOn = ((int)(now * 2)) % 2 == 0;

		float health = stats.ArmstPlayerStatGetHealth();
		float water = stats.ArmstPlayerStatGetWater();
		float food = stats.ArmstPlayerStatGetEat();

		bool sample = now - s_fLastSample >= TREND_SAMPLE;
		if (sample)
			s_fLastSample = now;

		UpdateMain(s_aMain[0], health, now, sample, blinkOn);
		UpdateMain(s_aMain[1], water, now, sample, blinkOn);
		UpdateMain(s_aMain[2], food, now, sample, blinkOn);
		UpdateRad(stats, blinkOn);

		float stamina = Math.Clamp(stats.ArmstPlayerStatGetStamina(), 0, 1);
		float barWidth = SLOT * 4 + SLOT_GAP * 3;
		FrameSlot.SetSize(s_wStamFill, barWidth * stamina, 4);

		UpdatePopups(stats, damage, now, blinkOn);
	}

	//------------------------------------------------------------------------------------------------
	// Inventory, map, pause and Game Master all hide the HUD
	protected static bool IsBlocked()
	{
		MenuManager menus = GetGame().GetMenuManager();
		if (menus && menus.IsAnyMenuOpen())
			return true;

		SCR_EditorManagerEntity editor = SCR_EditorManagerEntity.GetInstance();
		return editor && editor.IsOpened();
	}

	//------------------------------------------------------------------------------------------------
	protected static void UpdateMain(SRZ_StatusMain icon, float value, float now, bool sample, bool blinkOn)
	{
		if (!icon)
			return;

		value = Math.Clamp(value, 0, 100);
		if (sample)
		{
			if (icon.m_fSample >= 0)
				icon.m_fRate = (value - icon.m_fSample) / TREND_SAMPLE;

			icon.m_fSample = value;
		}

		int color = WHITE;
		float opacity = 1;
		if (value < 10)
		{
			color = RED;
			if (!blinkOn)
				opacity = 0.25;
		}
		else if (value < 30)
		{
			color = RED;
		}
		else if (value < 60)
		{
			color = YELLOW;
		}

		icon.m_wFill.SetColorInt(color);
		icon.m_wFill.SetOpacity(opacity);
		icon.m_wFill.SetMaskProgress(value / 100);
		SetArrows(icon, color, opacity);
	}

	//------------------------------------------------------------------------------------------------
	protected static void SetArrows(SRZ_StatusMain icon, int color, float opacity)
	{
		float rate = Math.AbsFloat(icon.m_fRate);
		int count = 0;
		if (rate >= 2)
			count = 3;
		else if (rate >= 0.5)
			count = 2;
		else if (rate >= 0.05)
			count = 1;

		ResourceName tex = Tex("ArrowDown");
		if (icon.m_fRate > 0)
			tex = Tex("ArrowUp");

		for (int i = 0; i < 3; i++)
		{
			ImageWidget arrow = icon.m_aArrows[i];
			arrow.SetVisible(i < count);
			if (i >= count)
				continue;

			arrow.LoadImageTexture(0, tex);
			arrow.SetColorInt(color);
			arrow.SetOpacity(opacity);
		}
	}

	//------------------------------------------------------------------------------------------------
	// Colour follows your own radiation level: white under 40, yellow from 40, red from 70, flashing red from 90
	protected static void UpdateRad(ARMST_PLAYER_STATS_COMPONENT stats, bool blinkOn)
	{
		SRZ_StatusMain icon = s_aMain[3];
		float rad = stats.ArmstPlayerStatGetRadio();

		int color = WHITE;
		float opacity = 1;
		if (rad >= RAD_FLASH)
		{
			color = RED;
			if (!blinkOn)
				opacity = 0.25;
		}
		else if (rad >= RAD_RED)
		{
			color = RED;
		}
		else if (rad >= RAD_YELLOW)
		{
			color = YELLOW;
		}

		icon.m_wFill.SetColorInt(color);
		icon.m_wFill.SetOpacity(opacity);
	}

	//------------------------------------------------------------------------------------------------
	protected static void UpdatePopups(ARMST_PLAYER_STATS_COMPONENT stats, SCR_CharacterDamageManagerComponent damage, float now, bool blinkOn)
	{
		// Psy damage: brain shows while psy keeps dropping, flashing red when it drops fast
		float psy = stats.ArmstPlayerStatGetPsy();
		if (s_fLastPsy >= 0 && psy < s_fLastPsy - 0.01)
		{
			s_fLastPsyDrop = now;
			s_aPsyDropTimes.Insert(now);
			s_aPsyDrops.Insert(s_fLastPsy - psy);
		}

		s_fLastPsy = psy;
		float recentDrop = 0;
		for (int i = s_aPsyDropTimes.Count() - 1; i >= 0; i--)
		{
			if (now - s_aPsyDropTimes[i] > PSY_SHOW_TIME)
			{
				s_aPsyDropTimes.Remove(i);
				s_aPsyDrops.Remove(i);
				continue;
			}

			recentDrop += s_aPsyDrops[i];
		}

		bool psyHit = now - s_fLastPsyDrop < PSY_SHOW_TIME;
		if (recentDrop >= PSY_HEAVY_DROP)
			SetPopup(s_wBrain, psyHit, RED, !blinkOn);
		else
			SetPopup(s_wBrain, psyHit, YELLOW, false);

		bool bleeding = damage && damage.IsBleeding();
		SetPopup(s_wBleed, bleeding, RED, !blinkOn);

		if (s_iLegState == SRZ_Legs.SPLINTED)
			SetPopup(s_wBone, true, YELLOW, false);
		else
			SetPopup(s_wBone, s_iLegState == SRZ_Legs.BROKEN, RED, false);

		float stimLeft;
		bool sidePhase;
		bool stim = SRZ_StimHud.SRZ_GetActive(stimLeft, sidePhase);
		int stimColor = WHITE;
		if (sidePhase)
			stimColor = YELLOW;

		SetPopup(s_wStim, stim, stimColor, false);
		s_wStimText.SetVisible(stim);
		if (stim)
		{
			int secs = (int)stimLeft;
			string sec = (secs % 60).ToString();
			if (secs % 60 < 10)
				sec = "0" + sec;

			s_wStimText.SetText((secs / 60).ToString() + ":" + sec);
			s_wStimText.SetColorInt(stimColor);
		}

		LayoutPopups(stim);
	}

	//------------------------------------------------------------------------------------------------
	protected static void SetPopup(ImageWidget w, bool show, int color, bool dim)
	{
		w.SetVisible(show);
		if (!show)
			return;

		w.SetColorInt(color);
		if (dim)
			w.SetOpacity(0.25);
		else
			w.SetOpacity(1);
	}

	//------------------------------------------------------------------------------------------------
	// Packs visible pop ups right to left, starting just left of the health icon
	protected static void LayoutPopups(bool stim)
	{
		float top = -BOTTOM - 42 - ICON + (ICON - POPUP) / 2;
		float x = -RIGHT - SLOT * 4 - SLOT_GAP * 3 - SLOT_GAP;
		bool any = stim || s_wBone.IsVisible() || s_wBleed.IsVisible() || s_wBrain.IsVisible();

		s_wSep.SetVisible(any);
		if (!any)
			return;

		x -= 2;
		FrameSlot.SetPos(s_wSep, x, -BOTTOM - 42 - ICON + 4);
		x -= SLOT_GAP;

		if (stim)
		{
			x -= 52;
			FrameSlot.SetPos(s_wStimText, x + 4, top + 3);
			x -= POPUP;
			FrameSlot.SetPos(s_wStim, x, top);
			x -= 14;
		}

		array<ImageWidget> order = {s_wBone, s_wBleed, s_wBrain};
		foreach (ImageWidget w : order)
		{
			if (!w.IsVisible())
				continue;

			x -= POPUP;
			FrameSlot.SetPos(w, x, top);
			x -= 14;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static bool Create()
	{
		if (s_wRoot)
			return true;

		s_wRoot = GetGame().GetWorkspace().CreateWidgets(LAYOUT);
		if (!s_wRoot)
		{
			Print("[SRZ StatusHud] Could not create layout", LogLevel.WARNING);
			return false;
		}

		s_wRoot.SetZOrder(9000);
		s_aMain.Clear();

		array<string> names = {"Health", "Water", "Food", "Rad"};
		float iconTop = -BOTTOM - 42 - ICON;
		for (int i = 0; i < 4; i++)
		{
			string n = names[i];
			float left = -RIGHT - SLOT * (4 - i) - SLOT_GAP * (3 - i);

			SRZ_StatusMain icon = new SRZ_StatusMain();
			icon.m_wBg = ImageWidget.Cast(s_wRoot.FindAnyWidget("SRZ_SH_" + n + "Bg"));
			icon.m_wFill = ImageWidget.Cast(s_wRoot.FindAnyWidget("SRZ_SH_" + n + "Fill"));
			Place(icon.m_wBg, Tex(n), left, iconTop, ICON, ICON, SHADOW);
			Place(icon.m_wFill, Tex(n), left, iconTop, ICON, ICON, WHITE);
			if (n != "Rad")
			{
				if (!icon.m_wFill.LoadMaskTexture(Tex("FillMask")))
					Print("[SRZ StatusHud] Fill mask did not load", LogLevel.WARNING);

				icon.m_wFill.SetMaskTransitionWidth(0);
			}
			else
			{
				icon.m_wBg.SetVisible(false);
			}

			for (int a = 0; a < 3; a++)
			{
				ImageWidget arrow = ImageWidget.Cast(s_wRoot.FindAnyWidget("SRZ_SH_" + n + "A" + a));
				Place(arrow, Tex("ArrowDown"), left + ICON + 3, iconTop + 5 + a * 9, 9, 6, WHITE);
				arrow.SetVisible(false);
				icon.m_aArrows.Insert(arrow);
			}

			s_aMain.Insert(icon);
		}

		float barWidth = SLOT * 4 + SLOT_GAP * 3;
		float barLeft = -RIGHT - barWidth;

		s_wSep = ImageWidget.Cast(s_wRoot.FindAnyWidget("SRZ_SH_Sep"));
		Place(s_wSep, Tex("White"), 0, 0, 2, ICON - 8, SEP);

		s_wBrain = SetupPopup("Brain");
		s_wBleed = SetupPopup("Bleed");
		s_wBone = SetupPopup("Bone");
		s_wStim = SetupPopup("Stim");
		s_wStimText = TextWidget.Cast(s_wRoot.FindAnyWidget("SRZ_SH_StimText"));
		FrameSlot.SetSize(s_wStimText, 52, 24);
		s_wStimText.SetVisible(false);

		s_wStamBg = ImageWidget.Cast(s_wRoot.FindAnyWidget("SRZ_SH_StamBg"));
		s_wStamFill = ImageWidget.Cast(s_wRoot.FindAnyWidget("SRZ_SH_StamFill"));
		Place(s_wStamBg, Tex("White"), barLeft, -BOTTOM - 32, barWidth, 4, STAM_BG);
		Place(s_wStamFill, Tex("White"), barLeft, -BOTTOM - 32, barWidth, 4, WHITE);

		s_wBrand = TextWidget.Cast(s_wRoot.FindAnyWidget("SRZ_SH_Brand"));
		FrameSlot.SetPos(s_wBrand, barLeft, -BOTTOM - 22);
		FrameSlot.SetSize(s_wBrand, barWidth + 20, 20);
		s_wBrand.SetColorInt(BRAND);

		if (!s_bWatching)
		{
			s_bWatching = true;
			GetGame().GetCallqueue().CallLater(Watch, 500, true);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected static ImageWidget SetupPopup(string name)
	{
		ImageWidget w = ImageWidget.Cast(s_wRoot.FindAnyWidget("SRZ_SH_" + name));
		Place(w, Tex(name), 0, 0, POPUP, POPUP, WHITE);
		w.SetVisible(false);
		return w;
	}

	//------------------------------------------------------------------------------------------------
	protected static void Place(ImageWidget w, ResourceName tex, float x, float y, float sizeX, float sizeY, int color)
	{
		w.LoadImageTexture(0, tex);
		FrameSlot.SetPos(w, x, y);
		FrameSlot.SetSize(w, sizeX, sizeY);
		w.SetColorInt(color);
	}

	//------------------------------------------------------------------------------------------------
	// Hides the HUD when the ARMST HUD stops ticking, like on death or in menus without a character
	protected static void Watch()
	{
		if (!s_wRoot || !GetGame().GetWorld())
			return;

		float now = GetGame().GetWorld().GetWorldTime() / 1000;
		if (now - s_fLastTick > 1)
		{
			s_wRoot.SetVisible(false);
			s_fLastPsy = -1;
		}
	}
}

modded class ARMST_TestHudUpdate
{
	protected Widget m_wSRZ_CleanedRoot;

	//------------------------------------------------------------------------------------------------
	override protected void UpdateIndicators()
	{
		super.UpdateIndicators();
		SRZ_StatusHud.Tick(m_PlayerStats, m_DamageManager);

		if (!SRZ_StatusHud.HIDE_ARMST_HUD)
			return;

		SRZ_Hide(m_Stat_Health);
		SRZ_Hide(m_Stat_Stamina);
		SRZ_Hide(m_Stat_Water);
		SRZ_Hide(m_Stat_Eat);
		SRZ_Hide(m_Stat_Psy);
		SRZ_Hide(m_Stat_Radiation);
		SRZ_Hide(m_Stat_Scheemese);
		SRZ_Hide(m_Stat_Bleed);

		SRZ_HideOldFrame();
	}

	//------------------------------------------------------------------------------------------------
	protected void SRZ_Hide(Widget w)
	{
		if (w)
			w.SetVisible(false);
	}

	//------------------------------------------------------------------------------------------------
	// Hides the frame art behind the old bars and the old server name text, once per HUD
	protected void SRZ_HideOldFrame()
	{
		if (!m_Stat_Health)
			return;

		Widget top = m_Stat_Health;
		while (top.GetParent())
		{
			top = top.GetParent();
		}

		if (top == m_wSRZ_CleanedRoot)
			return;

		m_wSRZ_CleanedRoot = top;

		Widget parent = m_Stat_Health.GetParent();
		if (parent)
		{
			Widget child = parent.GetChildren();
			while (child)
			{
				if (ImageWidget.Cast(child) && child != m_Stat_Weight && child != m_Stat_Surrender && child != m_Stat_Safe)
					child.SetVisible(false);

				child = child.GetSibling();
			}
		}

		Widget rootChild = top.GetChildren();
		while (rootChild)
		{
			TextWidget text = TextWidget.Cast(rootChild);
			if (text && text.GetText().Contains("Stalker Reforged"))
				text.SetVisible(false);

			rootChild = rootChild.GetSibling();
		}
	}
}
