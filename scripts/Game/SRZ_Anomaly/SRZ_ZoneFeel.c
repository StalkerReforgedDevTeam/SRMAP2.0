class SRZ_ZoneFeel
{
	protected static const float PSY_FROM = 85;
	protected static const float PSY_FULL = 30;
	protected static const float TOXIC_FROM = 5;
	protected static const float TOXIC_FULL = 50;
	protected static const float EASE_PER_S = 1.5;

	protected static const float BLUE_OPACITY = 0.22;
	protected static const float GREEN_OPACITY = 0.2;
	protected static const float MUFFLE_AT_FULL = 0.55;
	protected static const float ROLL_DEG_AT_FULL = 7;
	protected static const float FLASH_S = 0.45;

	protected static const int SORT = 880;

	protected static float s_fPsy;
	protected static float s_fToxic;
	protected static float s_fClock;
	protected static float s_fLast = -1;
	protected static float s_fFlashLeft;
	protected static float s_fNextFlicker;
	protected static ImageWidget s_wBlue;
	protected static ImageWidget s_wGreen;
	protected static ImageWidget s_wFlash;
	protected static bool s_bVolumeHeld;
	protected static float s_fBaseSfx = 1;

	static void Flash(IEntity character)
	{
		if (!character || character != SCR_PlayerController.GetLocalControlledEntity())
			return;

		s_fFlashLeft = FLASH_S;
		s_fNextFlicker = 0;
	}

	static float RollDeg()
	{
		if (s_fPsy <= 0.001)
			return 0;

		float drift = Math.Sin(s_fClock * 0.9) * 0.7 + Math.Sin(s_fClock * 2.3 + 1.1) * 0.3;
		return s_fPsy * ROLL_DEG_AT_FULL * drift;
	}

	static void Step()
	{
		if (SRZ_FXRange.IsHeadless())
			return;

		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return;

		float now = world.GetWorldTime() * 0.001;
		if (now == s_fLast)
			return;

		float dt = 0;
		if (s_fLast >= 0)
			dt = Math.Clamp(now - s_fLast, 0, 0.25);

		s_fLast = now;
		s_fClock += dt;

		float wantPsy = 0;
		float wantToxic = 0;
		ChimeraCharacter character = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		if (character && character.GetCharacterController() && character.GetCharacterController().GetLifeState() != ECharacterLifeState.DEAD)
		{
			ARMST_PLAYER_STATS_COMPONENT stats = ARMST_PLAYER_STATS_COMPONENT.Cast(character.FindComponent(ARMST_PLAYER_STATS_COMPONENT));
			if (stats)
			{
				wantPsy = Math.Clamp((PSY_FROM - stats.ArmstPlayerStatGetPsy()) / (PSY_FROM - PSY_FULL), 0, 1);
				wantToxic = Math.Clamp((stats.ArmstPlayerStatGetToxic() - TOXIC_FROM) / (TOXIC_FULL - TOXIC_FROM), 0, 1);
			}
		}
		else
		{
			s_fFlashLeft = 0;
		}

		float k = Math.Clamp(dt * EASE_PER_S, 0, 1);
		s_fPsy += (wantPsy - s_fPsy) * k;
		s_fToxic += (wantToxic - s_fToxic) * k;
		if (s_fPsy < 0.002)
			s_fPsy = 0;

		if (s_fToxic < 0.002)
			s_fToxic = 0;

		UpdateTints();
		UpdateFlash(dt);
		UpdateVolume();
	}

	protected static ImageWidget Layer(ImageWidget existing, int r, int g, int b, int sort)
	{
		if (existing)
			return existing;

		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
			return null;

		Widget w = workspace.CreateWidgetInWorkspace(WidgetType.ImageWidgetTypeID, 0, 0, workspace.GetWidth(), workspace.GetHeight(), WidgetFlags.VISIBLE | WidgetFlags.BLEND | WidgetFlags.IGNORE_CURSOR | WidgetFlags.NOFOCUS, Color.FromRGBA(r, g, b, 255), sort);
		ImageWidget image = ImageWidget.Cast(w);
		if (!image)
			return null;

		FrameSlot.SetAnchorMin(image, 0, 0);
		FrameSlot.SetAnchorMax(image, 1, 1);
		FrameSlot.SetOffsets(image, 0, 0, 0, 0);
		image.SetColor(Color.FromRGBA(r, g, b, 255));
		image.SetOpacity(0);
		image.SetVisible(false);
		return image;
	}

	protected static void Show(ImageWidget image, float opacity)
	{
		if (!image)
			return;

		image.SetOpacity(Math.Clamp(opacity, 0, 1));
		image.SetVisible(opacity > 0.002);
	}

	protected static void UpdateTints()
	{
		if (s_fPsy > 0 || s_wBlue)
		{
			s_wBlue = Layer(s_wBlue, 120, 190, 255, SORT);
			float breathe = 0.8 + 0.2 * Math.Sin(s_fClock * 1.3);
			Show(s_wBlue, BLUE_OPACITY * s_fPsy * breathe);
		}

		if (s_fToxic > 0 || s_wGreen)
		{
			s_wGreen = Layer(s_wGreen, 90, 230, 60, SORT + 1);
			float pulse = 0.65 + 0.35 * Math.Sin(s_fClock * 2.6);
			Show(s_wGreen, GREEN_OPACITY * s_fToxic * pulse);
		}
	}

	protected static void UpdateFlash(float dt)
	{
		if (s_fFlashLeft <= 0)
		{
			if (s_wFlash)
				Show(s_wFlash, 0);

			return;
		}

		s_wFlash = Layer(s_wFlash, 210, 230, 255, SORT + 2);
		s_fFlashLeft -= dt;
		if (s_fClock < s_fNextFlicker)
			return;

		s_fNextFlicker = s_fClock + Math.RandomFloat(0.03, 0.07);
		float opacity = 0;
		if (Math.RandomFloat01() < 0.6)
			opacity = Math.RandomFloat(0.15, 0.45) * Math.Clamp(s_fFlashLeft / FLASH_S + 0.3, 0, 1);

		Show(s_wFlash, opacity);
	}

	protected static void UpdateVolume()
	{
		float keep = 1 - MUFFLE_AT_FULL * s_fPsy;
		if (keep < 0.995)
		{
			if (!s_bVolumeHeld)
			{
				s_bVolumeHeld = true;
				s_fBaseSfx = Math.Clamp(AudioSystem.GetMasterVolume(AudioSystem.SFX), 0, 1);
			}

			AudioSystem.SetMasterVolume(AudioSystem.SFX, s_fBaseSfx * keep);
			return;
		}

		if (!s_bVolumeHeld)
			return;

		s_bVolumeHeld = false;
		AudioSystem.SetMasterVolume(AudioSystem.SFX, s_fBaseSfx);
	}
}

modded class SCR_CharacterCameraHandlerComponent
{
	override void AddShakeToToTransform(inout vector transform[4], inout float fieldOfView)
	{
		super.AddShakeToToTransform(transform, fieldOfView);
		SRZ_ZoneFeel.Step();

		float roll = SRZ_ZoneFeel.RollDeg();
		if (roll == 0)
			return;

		float r = roll * Math.DEG2RAD;
		float c = Math.Cos(r);
		float s = Math.Sin(r);
		vector right = transform[0];
		vector up = transform[1];
		transform[0] = right * c + up * s;
		transform[1] = up * c - right * s;
	}
}
