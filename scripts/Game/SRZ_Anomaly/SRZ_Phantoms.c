[BaseContainerProps()]
class SRZ_PhantomSound
{
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Sound project", params: "acp")]
	ResourceName m_Project;

	[Attribute("", UIWidgets.EditBox, "Sound event name")]
	string m_sEvent;

	[Attribute("1", UIWidgets.Slider, "How often it is picked compared to the others", "0 10 0.1")]
	float m_fWeight;

	[Attribute("5", UIWidgets.Slider, "Closest it plays, metres", "1 40 0.5")]
	float m_fMinDistance;

	[Attribute("10", UIWidgets.Slider, "Farthest it plays, metres", "1 40 0.5")]
	float m_fMaxDistance;

	[Attribute("1", UIWidgets.Slider, "Times it repeats, each one closer (4 for footsteps creeping up)", "1 10 1")]
	int m_iRepeats;

	[Attribute("0.55", UIWidgets.Slider, "Seconds between repeats", "0.1 5 0.05")]
	float m_fRepeatGapS;

	[Attribute("1.4", UIWidgets.Slider, "Height above the ground, metres", "0 3 0.1")]
	float m_fHeight;
}

[BaseContainerProps(configRoot: true)]
class SRZ_PhantomSoundsConfig
{
	[Attribute("60", UIWidgets.Slider, "Phantom sounds start once psy health drops below this", "0 100 1")]
	float m_fPsyFrom;

	[Attribute("20", UIWidgets.Slider, "At or below this psy health they come as often as they can", "0 100 1")]
	float m_fPsyFull;

	[Attribute("90", UIWidgets.Slider, "Average seconds between sounds when they first start", "5 600 1")]
	float m_fGapStartS;

	[Attribute("20", UIWidgets.Slider, "Average seconds between sounds at the worst", "2 300 1")]
	float m_fGapFullS;

	[Attribute()]
	ref array<ref SRZ_PhantomSound> m_aSounds;
}

class SRZ_Phantoms
{
	protected static const ResourceName CONFIG = "{5E3719EA266C6E70}Configs/SRZ/SRZ_PhantomSounds.conf";

	protected static ref SRZ_PhantomSoundsConfig s_Config;
	protected static bool s_bLoaded;
	protected static float s_fWait = -1;
	protected static ref array<float> s_aAt = {};
	protected static ref array<vector> s_aPos = {};
	protected static ref array<ResourceName> s_aProject = {};
	protected static ref array<string> s_aEvent = {};
	protected static float s_fClock;
	protected static float s_fNextLog;
	static const bool DEBUG = false;

	protected static SRZ_PhantomSoundsConfig Config()
	{
		if (s_bLoaded)
			return s_Config;

		s_bLoaded = true;
		Resource resource = BaseContainerTools.LoadContainer(CONFIG);
		if (!resource || !resource.IsValid())
		{
			if (DEBUG)
				Print("[SRZ Phantom] config could not be loaded: " + CONFIG, LogLevel.WARNING);

			return null;
		}

		s_Config = SRZ_PhantomSoundsConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(resource.GetResource().ToBaseContainer()));
		if (DEBUG)
		{
			int count = -1;
			if (s_Config && s_Config.m_aSounds)
				count = s_Config.m_aSounds.Count();

			Print(string.Format("[SRZ Phantom] config loaded %1, sounds %2", s_Config != null, count), LogLevel.NORMAL);
		}

		return s_Config;
	}

	static void Step(float dt, IEntity character, float psy)
	{
		s_fClock += dt;
		PlayDue();

		SRZ_PhantomSoundsConfig config = Config();
		if (DEBUG && s_fClock >= s_fNextLog)
		{
			s_fNextLog = s_fClock + 5;
			float startsBelow = -1;
			if (config)
				startsBelow = config.m_fPsyFrom;

			Print(string.Format("[SRZ Phantom] psy %1, starts below %2, next sound in %3 s, character %4", psy, startsBelow, s_fWait, character != null), LogLevel.NORMAL);
		}
		if (!config || !config.m_aSounds || config.m_aSounds.IsEmpty() || !character || psy >= config.m_fPsyFrom)
		{
			s_fWait = -1;
			return;
		}

		float depth = Math.Clamp((config.m_fPsyFrom - psy) / Math.Max(config.m_fPsyFrom - config.m_fPsyFull, 1), 0, 1);
		if (s_fWait < 0)
			s_fWait = Math.Lerp(config.m_fGapStartS, config.m_fGapFullS, depth) * Math.RandomFloat(0.6, 1.4);

		s_fWait -= dt;
		if (s_fWait > 0)
			return;

		s_fWait = -1;
		SRZ_PhantomSound sound = Pick(config.m_aSounds);
		if (sound)
			Queue(sound, character);
	}

	protected static SRZ_PhantomSound Pick(array<ref SRZ_PhantomSound> sounds)
	{
		float total = 0;
		foreach (SRZ_PhantomSound s : sounds)
		{
			if (s && !s.m_Project.IsEmpty() && !s.m_sEvent.IsEmpty())
				total += Math.Max(s.m_fWeight, 0);
		}

		if (total <= 0)
			return null;

		float roll = Math.RandomFloat(0, total);
		foreach (SRZ_PhantomSound candidate : sounds)
		{
			if (!candidate || candidate.m_Project.IsEmpty() || candidate.m_sEvent.IsEmpty())
				continue;

			roll -= Math.Max(candidate.m_fWeight, 0);
			if (roll <= 0)
				return candidate;
		}

		return null;
	}

	protected static void Queue(SRZ_PhantomSound sound, IEntity character)
	{
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return;

		vector cam[4];
		world.GetCurrentCamera(cam);
		vector forward = cam[2];
		forward[1] = 0;
		if (forward.LengthSq() < 0.0001)
			forward = vector.Forward;

		forward.Normalize();
		vector right = Vector(forward[2], 0, -forward[0]);
		vector back = right * Math.RandomFloat(-0.6, 0.6) - forward;
		back.Normalize();

		vector feet = character.GetOrigin();
		int repeats = Math.Max(sound.m_iRepeats, 1);
		float near = Math.Min(sound.m_fMinDistance, sound.m_fMaxDistance);
		float far = Math.Max(sound.m_fMinDistance, sound.m_fMaxDistance);
		float start = Math.RandomFloat(near, far);
		for (int i = 0; i < repeats; i++)
		{
			float dist = start;
			if (repeats > 1)
				dist = Math.Lerp(far, near, i / (repeats - 1.0));

			s_aAt.Insert(s_fClock + i * sound.m_fRepeatGapS);
			s_aPos.Insert(feet + back * dist + Vector(0, sound.m_fHeight, 0));
			s_aProject.Insert(sound.m_Project);
			s_aEvent.Insert(sound.m_sEvent);
		}
	}

	protected static void PlayDue()
	{
		for (int i = s_aAt.Count() - 1; i >= 0; i--)
		{
			if (s_fClock < s_aAt[i])
				continue;

			AudioHandle handle = SRZ_FXRange.Sound(s_aProject[i], s_aEvent[i], s_aPos[i]);
			if (DEBUG)
				Print(string.Format("[SRZ Phantom] played %1 %2, handle valid %3", s_aProject[i], s_aEvent[i], handle != AudioHandle.Invalid), LogLevel.NORMAL);

			s_aAt.Remove(i);
			s_aPos.Remove(i);
			s_aProject.Remove(i);
			s_aEvent.Remove(i);
		}
	}
}
