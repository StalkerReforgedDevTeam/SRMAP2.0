// Small server rules that used to come from separate workshop mods:
// no vote kicks or vote GMs, no photo mode, no name tags, a wait before Exit, shorter nights and an AFK kick.

class SRZ_ServerTweaks
{
	// Exit button wait in seconds
	static const int EXIT_WAIT = 20;

	// Real seconds for one full day while it is light
	static const float DAY_LENGTH = 14400;
	// Share of the real day cycle spent in deep night
	static const float NIGHT_SHARE = 0.25;
	// Game hours around sunset and sunrise that run at normal speed so the light change looks right
	static const float TWILIGHT_HOURS = 1.5;

	// AFK kick: idle seconds before kick, warning shown this many seconds before, only with at least this many players online
	static const int AFK_KICK_AFTER = 900;
	static const int AFK_WARN_BEFORE = 60;
	static const int AFK_MIN_PLAYERS = 40;
}

// ---------------- Voting off ----------------
modded class SCR_VotingManagerComponent
{
	override bool StartVoting(EVotingType type, int startingPlayerID, int value = SCR_VotingBase.DEFAULT_VALUE)
	{
		return false;
	}

	override void Vote(int playerID, EVotingType type, int value)
	{
	}

	override bool IsVotingAvailable(EVotingType type, int value = SCR_VotingBase.DEFAULT_VALUE)
	{
		return false;
	}
}

// ---------------- Photo mode (Arma Vision) off ----------------
modded class SCR_EditorManagerEntity
{
	void SCR_EditorManagerEntity(IEntitySource src, IEntity parent)
	{
		m_bIsLimited = true;
	}
}

[BaseContainerProps(configRoot: true)]
modded class SCR_EditorManagerCore
{
	void SCR_EditorManagerCore()
	{
		for (int i = m_ModePrefabs.Count() - 1; i >= 0; i--)
		{
			if (m_ModePrefabs[i].GetMode() == EEditorMode.PHOTO)
				m_ModePrefabs.Remove(i);
		}

		m_ModePrefabsSorted.Clear();
		foreach (SCR_EditorModePrefab mode : m_ModePrefabs)
		{
			m_ModePrefabsSorted.Insert(mode.GetInfo().GetOrder(), mode);
		}
	}
}

// ---------------- Name tags off ----------------
modded class SCR_NameTagDisplay
{
	override protected void Update(float timeSlice)
	{
	}
}

// ---------------- Exit button wait ----------------
modded class PauseMenuUI
{
	protected SCR_ButtonTextComponent m_SRZ_ExitButton;
	protected int m_iSRZ_ExitLeft;

	override void OnMenuOpen()
	{
		super.OnMenuOpen();

		m_SRZ_ExitButton = SCR_ButtonTextComponent.GetButtonText("Exit", m_wRoot);
		if (!m_SRZ_ExitButton)
			return;

		m_SRZ_ExitButton.m_OnClicked.Remove(OnExit);
		m_iSRZ_ExitLeft = SRZ_ServerTweaks.EXIT_WAIT;
		SRZ_ShowExitWait();
		GetGame().GetCallqueue().Remove(SRZ_ExitTick);
		GetGame().GetCallqueue().CallLater(SRZ_ExitTick, 1000, true);
	}

	override void OnMenuClose()
	{
		GetGame().GetCallqueue().Remove(SRZ_ExitTick);
		super.OnMenuClose();
	}

	protected void SRZ_ExitTick()
	{
		if (!m_SRZ_ExitButton)
		{
			GetGame().GetCallqueue().Remove(SRZ_ExitTick);
			return;
		}

		m_iSRZ_ExitLeft--;
		if (m_iSRZ_ExitLeft > 0)
		{
			SRZ_ShowExitWait();
			return;
		}

		GetGame().GetCallqueue().Remove(SRZ_ExitTick);
		m_SRZ_ExitButton.m_OnClicked.Remove(OnExit);
		m_SRZ_ExitButton.m_OnClicked.Insert(OnExit);
		m_SRZ_ExitButton.SetText("Exit");
	}

	protected void SRZ_ShowExitWait()
	{
		m_SRZ_ExitButton.SetText(string.Format("Exit in %1", m_iSRZ_ExitLeft));
	}
}

// ---------------- Shorter nights ----------------
// The vanilla day speed handler would keep resetting our day length, so it stays out of the way
modded class SCR_TimeAndWeatherHandlerComponent
{
	override protected void HandleDaytimeAcceleration(bool setup = false)
	{
	}
}

modded class TimeAndWeatherManagerEntity
{
	protected bool m_bSRZ_DeepNight;
	protected bool m_bSRZ_Started;

	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		RplComponent rpl = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (!rpl || rpl.IsProxy())
			return;

		m_bSRZ_Started = false;
		SRZ_NightTick();
		GetGame().GetCallqueue().CallLater(SRZ_NightTick, 10000, true);
	}

	protected void SRZ_NightTick()
	{
		array<SCR_DayTimeInfoBase> info = {};
		GetDayTimeInfoArray(info, GetYear(), GetMonth(), GetDay());
		if (info.Count() < 4)
			return;

		float sunrise = info[0].GetDayTime();
		float sunset = info[3].GetDayTime();
		float dayHours = sunset - sunrise;
		float nightHours = 24 - dayHours;
		if (dayHours <= 0 || nightHours <= 0)
			return;

		float now = GetTime().ToTimeOfTheDay();
		float sinceSunset = now - sunset;
		if (sinceSunset < 0)
			sinceSunset += 24;

		float toSunrise = sunrise - now;
		if (toSunrise < 0)
			toSunrise += 24;

		bool night = now >= sunset || now < sunrise;
		bool deep = night && sinceSunset >= SRZ_ServerTweaks.TWILIGHT_HOURS && toSunrise >= SRZ_ServerTweaks.TWILIGHT_HOURS;

		if (m_bSRZ_Started && deep == m_bSRZ_DeepNight)
			return;

		m_bSRZ_Started = true;
		m_bSRZ_DeepNight = deep;

		float speed = 1;
		if (deep)
		{
			// Hours of deep night that would need to pass at normal speed, versus how many real hours it should feel like
			float deepHours = nightHours - SRZ_ServerTweaks.TWILIGHT_HOURS * 2;
			float wanted = SRZ_ServerTweaks.NIGHT_SHARE / (1 - SRZ_ServerTweaks.NIGHT_SHARE) * dayHours - SRZ_ServerTweaks.TWILIGHT_HOURS * 2;
			if (deepHours > 0)
			{
				if (wanted <= 0)
					speed = 20;
				else
					speed = Math.Clamp(deepHours / wanted, 1, 20);
			}
		}

		SetDayDuration(SRZ_ServerTweaks.DAY_LENGTH / speed);
		Print(string.Format("[SRZ Nights] %1, day length %2s", deep, SRZ_ServerTweaks.DAY_LENGTH / speed));
	}
}

// ---------------- AFK kick ----------------
// Server side. A player counts as active when their character moves, turns, or they click the warning away.
class SRZ_AFKEntry
{
	vector m_vPos;
	float m_fYaw;
	int m_iIdle;
	bool m_bWarned;
}

class SRZ_AFK
{
	protected static const int TICK = 15;
	protected static ref map<int, ref SRZ_AFKEntry> s_mPlayers;

	static void Start()
	{
		if (!Replication.IsServer())
			return;

		s_mPlayers = new map<int, ref SRZ_AFKEntry>();
		GetGame().GetCallqueue().Remove(Tick);
		GetGame().GetCallqueue().CallLater(Tick, TICK * 1000, true);
	}

	static void MarkActive(int playerId)
	{
		if (!s_mPlayers)
			return;

		SRZ_AFKEntry e = s_mPlayers.Get(playerId);
		if (e)
		{
			e.m_iIdle = 0;
			e.m_bWarned = false;
		}
	}

	protected static bool IsExempt(PlayerManager pm, int playerId)
	{
		return pm.HasPlayerRole(playerId, EPlayerRole.ADMINISTRATOR) || pm.HasPlayerRole(playerId, EPlayerRole.GAME_MASTER) || pm.HasPlayerRole(playerId, EPlayerRole.SESSION_ADMINISTRATOR);
	}

	protected static void Tick()
	{
		PlayerManager pm = GetGame().GetPlayerManager();
		if (!pm)
			return;

		array<int> ids = {};
		pm.GetPlayers(ids);

		// Forget players who left
		array<int> gone = {};
		foreach (int known, SRZ_AFKEntry unused : s_mPlayers)
		{
			if (!ids.Contains(known))
				gone.Insert(known);
		}
		foreach (int g : gone)
		{
			s_mPlayers.Remove(g);
		}

		bool enforce = ids.Count() >= SRZ_ServerTweaks.AFK_MIN_PLAYERS;

		foreach (int id : ids)
		{
			SRZ_AFKEntry e = s_mPlayers.Get(id);
			if (!e)
			{
				e = new SRZ_AFKEntry();
				s_mPlayers.Set(id, e);
			}

			IEntity ent = pm.GetPlayerControlledEntity(id);
			if (ent)
			{
				vector pos = ent.GetOrigin();
				float yaw = ent.GetYawPitchRoll()[0];
				if (vector.DistanceSq(pos, e.m_vPos) > 0.25 || Math.AbsFloat(yaw - e.m_fYaw) > 3)
				{
					e.m_iIdle = 0;
					e.m_bWarned = false;
				}
				else
				{
					e.m_iIdle += TICK;
				}

				e.m_vPos = pos;
				e.m_fYaw = yaw;
			}
			else
			{
				e.m_iIdle += TICK;
			}

			if (!enforce || IsExempt(pm, id))
				continue;

			if (e.m_iIdle >= SRZ_ServerTweaks.AFK_KICK_AFTER)
			{
				Print(string.Format("[SRZ AFK] Kicking player %1 after %2s idle", id, e.m_iIdle));
				pm.KickPlayer(id, PlayerManagerKickReason.KICK, 0);
				continue;
			}

			if (!e.m_bWarned && e.m_iIdle >= SRZ_ServerTweaks.AFK_KICK_AFTER - SRZ_ServerTweaks.AFK_WARN_BEFORE)
			{
				e.m_bWarned = true;
				SCR_PlayerController pc = SCR_PlayerController.Cast(pm.GetPlayerController(id));
				if (pc)
					pc.SRZ_SendAFKWarning();
			}
		}
	}
}

modded class SCR_PlayerController
{
	void SRZ_SendAFKWarning()
	{
		Rpc(RpcDo_SRZ_AFKWarning);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_SRZ_AFKWarning()
	{
		DialogUI dialog = DialogUI.Cast(GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.ErrorDialog));
		if (!dialog)
			return;

		dialog.SetTitle("AFK warning");
		dialog.SetMessage("You've been inactive for a while. Move or press OK in the next minute or you'll be kicked.");
		dialog.m_OnConfirm.Insert(SRZ_AFKConfirmed);
		dialog.m_OnCancel.Insert(SRZ_AFKConfirmed);
	}

	protected void SRZ_AFKConfirmed()
	{
		Rpc(RpcAsk_SRZ_AFKActive);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_SRZ_AFKActive()
	{
		SRZ_AFK.MarkActive(GetPlayerId());
	}
}

modded class SCR_BaseGameMode
{
	override void OnGameStart()
	{
		super.OnGameStart();

		if (Replication.IsServer())
			SRZ_AFK.Start();
	}
}
