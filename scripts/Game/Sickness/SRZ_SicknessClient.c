// What being cold or sick feels like on the player's own machine: rhythmic shivering that gets faster and
// harder the colder you are, a washed out screen in the deeper bands, and fever throbs when properly ill.
// Fed by the owner RPC every 10 seconds, runs its own 100 ms tick.
class SRZ_SicknessClient
{
	protected static const ResourceName COLOR_MAT = "{7C202A913EB8B1A9}UI/Materials/ScreenEffects_ColorPP.emat";

	protected static int s_iBand;
	protected static int s_iSick;
	protected static bool s_bDraining;
	protected static bool s_bCodeine;
	protected static bool s_bTicking;
	protected static float s_fNextShiver;
	protected static float s_fNextThrob;
	protected static float s_fDesat;
	protected static Material s_Color;
	protected static bool s_bForcedThird;
	protected static CameraHandlerComponent s_Camera;

	//------------------------------------------------------------------------------------------------
	static void Set(int band, int sick, bool draining, bool codeine)
	{
		s_iBand = band;
		s_iSick = sick;
		s_bDraining = draining;
		s_bCodeine = codeine;

		if (!s_bTicking)
		{
			s_bTicking = true;
			GetGame().GetCallqueue().CallLater(Tick, 100, true);
		}
	}

	//------------------------------------------------------------------------------------------------
	// Switch to third person for a full body animation and switch back after, unless the player was already there
	static void ForceThirdPerson(IEntity character, float seconds)
	{
		if (!character)
			return;

		CameraHandlerComponent cam = CameraHandlerComponent.Cast(character.FindComponent(CameraHandlerComponent));
		if (!cam)
			return;

		GetGame().GetCallqueue().Remove(RestoreCamera);
		if (!s_bForcedThird && cam.IsInThirdPerson())
			return;

		s_Camera = cam;
		s_bForcedThird = true;
		cam.SetThirdPerson(true);
		GetGame().GetCallqueue().CallLater(RestoreCamera, seconds * 1000 + 300, false);
	}

	//------------------------------------------------------------------------------------------------
	protected static void RestoreCamera()
	{
		if (s_bForcedThird && s_Camera)
			s_Camera.SetThirdPerson(false);

		s_bForcedThird = false;
		s_Camera = null;
	}

	//------------------------------------------------------------------------------------------------
	protected static void Tick()
	{
		if (!GetGame().GetWorld())
			return;

		float now = GetGame().GetWorld().GetWorldTime() / 1000;
		bool cold = s_iBand <= SRZ_Sickness.BAND_COLD;

		// Shiver cadence and strength per band
		float gapMin = 0;
		float gapMax = 0;
		float linear = 0;
		float angular = 0;
		float hold = 0.5;
		if (s_iBand == SRZ_Sickness.BAND_COLD)
		{
			gapMin = 3; gapMax = 5; linear = 0.025; angular = 0.09;
		}
		else if (s_iBand == SRZ_Sickness.BAND_FREEZING)
		{
			gapMin = 1.5; gapMax = 2.5; linear = 0.05; angular = 0.18; hold = 0.8;
		}
		else if (s_iBand == SRZ_Sickness.BAND_HYPOTHERMIA)
		{
			gapMin = 0.8; gapMax = 1.3; linear = 0.08; angular = 0.28; hold = 1.0;
		}

		if (cold && !s_bCodeine && now >= s_fNextShiver)
		{
			s_fNextShiver = now + Math.RandomFloat(gapMin, gapMax);
			SCR_CameraShakeManagerComponent.AddCameraShake(linear, angular, 0.08, hold, 0.3);
		}

		// Fever throb: slow heavy sway now and then when properly ill
		if (s_iSick >= SRZ_Sickness.SICK_BAD && now >= s_fNextThrob)
		{
			s_fNextThrob = now + Math.RandomFloat(6, 12);
			SCR_CameraShakeManagerComponent.AddCameraShake(0.03, 0.08, 0.8, 1.2, 1.0);
		}

		// Screen washes out as you get colder, more when health is draining
		float target = 0;
		if (s_iBand == SRZ_Sickness.BAND_FREEZING) target = 0.3;
		else if (s_iBand == SRZ_Sickness.BAND_HYPOTHERMIA) target = 0.55;
		else if (s_iBand == SRZ_Sickness.BAND_COLD) target = 0.1;
		if (s_bDraining && s_iSick >= SRZ_Sickness.SICK_BAD)
			target = Math.Max(target, 0.35);

		if (target > s_fDesat)
			s_fDesat = Math.Min(target, s_fDesat + 0.02);
		else
			s_fDesat = Math.Max(target, s_fDesat - 0.02);

		ApplyColor(s_fDesat);
	}

	//------------------------------------------------------------------------------------------------
	protected static void ApplyColor(float amount)
	{
		if (!SRZ_ScreenFX.IsDefined("srz_cold"))
			SRZ_ScreenFX.Define("srz_cold", 60, PostProcessEffectType.Colors, COLOR_MAT, 2, 2);

		if (!s_Color)
			s_Color = Material.GetMaterial(COLOR_MAT);

		if (s_Color)
			s_Color.SetParam("Saturation", 1 - amount * 0.8);

		if (amount > 0.01)
			SRZ_ScreenFX.Want("srz_cold", "sickness", 1, 0.5);
		else
			SRZ_ScreenFX.Drop("srz_cold", "sickness");
	}
}
