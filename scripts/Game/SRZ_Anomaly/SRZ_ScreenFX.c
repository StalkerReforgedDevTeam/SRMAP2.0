class SRZ_ScreenFXBinding
{
	void Apply(float level)
	{
	}
}

class SRZ_ScreenFXSource
{
	float m_fStrength;
	float m_fUntil;
}

class SRZ_ScreenFXChannel
{
	int m_iPriority;
	PostProcessEffectType m_eType;
	ResourceName m_Material;
	float m_fRiseRate;
	float m_fFallRate;
	ref SRZ_ScreenFXBinding m_Binding;
	ref map<string, ref SRZ_ScreenFXSource> m_mSources = new map<string, ref SRZ_ScreenFXSource>();
	float m_fLevel;
	int m_iCamera = -1;
}

class SRZ_ScreenFX
{
	protected static const int TICK_MS = 33;
	protected static const float HOLD_S = 0.5;

	protected static ref map<string, ref SRZ_ScreenFXChannel> s_mChannels = new map<string, ref SRZ_ScreenFXChannel>();
	protected static float s_fLastTick = -1;
	protected static bool s_bRunning;

	static void Define(string channel, int priority, PostProcessEffectType type, ResourceName material, float riseRate, float fallRate, SRZ_ScreenFXBinding binding = null)
	{
		if (SRZ_FXRange.IsHeadless())
			return;

		SRZ_ScreenFXChannel c = s_mChannels.Get(channel);
		if (!c)
		{
			c = new SRZ_ScreenFXChannel();
			s_mChannels.Set(channel, c);
		}

		c.m_iPriority = priority;
		c.m_eType = type;
		c.m_Material = material;
		c.m_fRiseRate = Math.Max(riseRate, 0.01);
		c.m_fFallRate = Math.Max(fallRate, 0.01);
		c.m_Binding = binding;
		EnsureRunning();
	}

	static bool IsDefined(string channel)
	{
		return s_mChannels.Contains(channel);
	}

	static void Want(string channel, string source, float strength, float holdS = HOLD_S)
	{
		SRZ_ScreenFXChannel c = s_mChannels.Get(channel);
		if (!c)
			return;

		SRZ_ScreenFXSource s = c.m_mSources.Get(source);
		if (!s)
		{
			s = new SRZ_ScreenFXSource();
			c.m_mSources.Set(source, s);
		}

		s.m_fStrength = Math.Clamp(strength, 0, 1);
		s.m_fUntil = Now() + Math.Max(holdS, 0.05);
		EnsureRunning();
	}

	static void Drop(string channel, string source)
	{
		SRZ_ScreenFXChannel c = s_mChannels.Get(channel);
		if (c)
			c.m_mSources.Remove(source);
	}

	static float Level(string channel)
	{
		SRZ_ScreenFXChannel c = s_mChannels.Get(channel);
		if (!c)
			return 0;

		return c.m_fLevel;
	}

	protected static float Now()
	{
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return 0;

		return world.GetWorldTime() * 0.001;
	}

	protected static void EnsureRunning()
	{
		if (s_bRunning)
			return;

		s_bRunning = true;
		s_fLastTick = -1;
		GetGame().GetCallqueue().CallLater(Tick, TICK_MS, true);
	}

	protected static bool LocalAlive()
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		if (!character)
			return false;

		CharacterControllerComponent controller = character.GetCharacterController();
		return controller && controller.GetLifeState() != ECharacterLifeState.DEAD;
	}

	protected static void Tick()
	{
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return;

		float now = Now();
		float dt = 0;
		if (s_fLastTick >= 0)
			dt = Math.Clamp(now - s_fLastTick, 0, 0.25);

		s_fLastTick = now;
		bool alive = LocalAlive();
		bool anyActive = false;

		foreach (string name, SRZ_ScreenFXChannel c : s_mChannels)
		{
			float target = 0;
			for (int i = c.m_mSources.Count() - 1; i >= 0; i--)
			{
				SRZ_ScreenFXSource s = c.m_mSources.GetElement(i);
				if (!s || s.m_fUntil < now)
				{
					c.m_mSources.RemoveElement(i);
					continue;
				}

				target = Math.Max(target, s.m_fStrength);
			}

			if (!alive)
				target = 0;

			if (target > c.m_fLevel)
				c.m_fLevel = Math.Min(target, c.m_fLevel + c.m_fRiseRate * dt);
			else
				c.m_fLevel = Math.Max(target, c.m_fLevel - c.m_fFallRate * dt);

			if (c.m_Binding)
				c.m_Binding.Apply(c.m_fLevel);

			Show(world, c);
			if (c.m_fLevel > 0.001 || !c.m_mSources.IsEmpty())
				anyActive = true;
		}

		if (anyActive)
			return;

		GetGame().GetCallqueue().Remove(Tick);
		s_bRunning = false;
	}

	protected static void Show(BaseWorld world, SRZ_ScreenFXChannel c)
	{
		bool on = c.m_fLevel > 0.001 && !c.m_Material.IsEmpty();
		int camera = world.GetCurrentCameraId();

		if (c.m_iCamera >= 0 && (!on || c.m_iCamera != camera))
		{
			world.SetCameraPostProcessEffect(c.m_iCamera, c.m_iPriority, PostProcessEffectType.None, string.Empty);
			c.m_iCamera = -1;
		}

		if (!on || c.m_iCamera == camera)
			return;

		world.SetCameraPostProcessEffect(camera, c.m_iPriority, c.m_eType, c.m_Material);
		c.m_iCamera = camera;
	}
}
