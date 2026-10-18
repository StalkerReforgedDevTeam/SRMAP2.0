// Artifact detectors: the use button turns them on/off. While on they drain battery (server side) and scan
// on their own while held in your hands. Beeps/lights speed up as you get closer, and revealing an artifact
// gives a flash, a sound and a hint. Dead battery = detector switches off.

modded class ARMST_DETECTOR_ARTS_COMPONENTS
{
	protected static const ResourceName SRZ_REVEAL_FX = "{A1292362DBE60921}Prefabs/Anomals/anom_base/anomal_bolt_actions.ptc";
	protected static const float SRZ_IDLE_TICK_MS = 1000;
	protected static const float SRZ_DRAIN_TICK_MS = 10000;

	[RplProp()]
	protected bool m_bSRZ_On;

	protected bool m_bSRZ_LocalOn;
	protected float m_fSRZ_LastToggle = -10;
	protected static const float SRZ_TOGGLE_GAP_S = 0.4;

	protected bool m_bSRZ_Tracking;
	protected IEntity m_SRZ_Announced;
	protected IEntity m_SRZ_Nearest;
	protected float m_fSRZ_NearestDist;

	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if (System.IsConsoleApp() || SCR_Global.IsEditMode() || !GetGame().InPlayMode())
			return;

		GetGame().GetCallqueue().CallLater(SRZ_Tick, SRZ_IDLE_TICK_MS, false);
	}

	override void OnDelete(IEntity owner)
	{
		GetGame().GetCallqueue().Remove(SRZ_Tick);
		GetGame().GetCallqueue().Remove(SRZ_DrainTick);
		GetGame().GetCallqueue().Remove(SRZ_FlashStep);
		super.OnDelete(owner);
	}

	bool SRZ_IsOn()
	{
		if (Replication.IsServer())
			return m_bSRZ_On;

		return m_bSRZ_LocalOn;
	}

	// The carrier's own machine keeps its own on/off state so the HUD answers right away; the server confirms it
	void SRZ_ClientSetOn(bool on)
	{
		if (m_bSRZ_LocalOn == on)
			return;

		m_bSRZ_LocalOn = on;
		if (!on)
		{
			if (SRZ_BatteryDead())
				SRZ_Notify("The battery is dead.");
			else
				SRZ_Notify("Detector off.");
		}
	}

	// Use button: ask the server to flip the power state (a dedicated server ignores the local call)
	override void ArmstArtDetectorToggleScan(IEntity owner)
	{
		if (System.IsConsoleApp())
			return;

		BaseWorld world = GetGame().GetWorld();
		float now = 0;
		if (world)
			now = world.GetWorldTime() * 0.001;

		if (now - m_fSRZ_LastToggle < SRZ_TOGGLE_GAP_S)
			return;

		m_fSRZ_LastToggle = now;
		bool turningOn = !m_bSRZ_LocalOn;
		if (turningOn && SRZ_BatteryDead())
		{
			SRZ_Notify("The battery is dead.");
			return;
		}

		m_bSRZ_LocalOn = turningOn;

		string state = "Detector off.";
		if (turningOn)
			state = "Detector on.";
		SRZ_Notify(state);

		SCR_PlayerController controller = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (controller)
			controller.SRZ_RequestDetectorPower(GetOwner(), turningOn);
	}

	void SRZ_ServerSetOn(bool on)
	{
		if (!Replication.IsServer())
			return;

		if (on && SRZ_BatteryDead())
			on = false;

		if (m_bSRZ_On == on)
			return;

		m_bSRZ_On = on;
		Replication.BumpMe();

		GetGame().GetCallqueue().Remove(SRZ_DrainTick);
		if (on)
			GetGame().GetCallqueue().CallLater(SRZ_DrainTick, SRZ_DRAIN_TICK_MS, true);

		SRZ_TellCarrier(on);
	}

	protected void SRZ_TellCarrier(bool on)
	{
		IEntity carrier = GetOwner().GetRootParent();
		PlayerManager players = GetGame().GetPlayerManager();
		if (!carrier || !players)
			return;

		int playerId = players.GetPlayerIdFromControlledEntity(carrier);
		SCR_PlayerController controller = SCR_PlayerController.Cast(players.GetPlayerController(playerId));
		if (controller)
			controller.SRZ_SendDetectorState(GetOwner(), on);
	}

	// m_BatteryDescrease is now battery % lost per minute while on
	protected void SRZ_DrainTick()
	{
		ARMST_ITEMS_STATS_COMPONENTS stats = ARMST_ITEMS_STATS_COMPONENTS.Cast(GetOwner().FindComponent(ARMST_ITEMS_STATS_COMPONENTS));
		if (!stats || !m_bSRZ_On)
		{
			SRZ_ServerSetOn(false);
			return;
		}

		stats.DecreaseCondition(m_BatteryDescrease * SRZ_DRAIN_TICK_MS / 60000);
		if (stats.GetConditionLevel() <= 0)
			SRZ_ServerSetOn(false);
	}

	// Small ARMST HUD line instead of the big hint box
	protected void SRZ_Notify(string text)
	{
		SCR_PlayerController controller = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (controller)
			controller.RpcDo_AmmoCount(text);
	}

	protected bool SRZ_BatteryDead()
	{
		ARMST_ITEMS_STATS_COMPONENTS stats = ARMST_ITEMS_STATS_COMPONENTS.Cast(GetOwner().FindComponent(ARMST_ITEMS_STATS_COMPONENTS));
		return stats && stats.GetConditionLevel() <= 0;
	}

	protected void SRZ_Tick()
	{
		float delay = SRZ_IDLE_TICK_MS;

		if (SRZ_IsActive() && SRZ_FindNearest())
		{
			SRZ_Feedback(m_SRZ_Nearest, m_fSRZ_NearestDist);
			delay = Math.Clamp(m_fSRZ_NearestDist * 90, 150, 1200);
		}
		else if (m_bSRZ_Tracking)
		{
			SRZ_StopTracking();
		}

		GetGame().GetCallqueue().CallLater(SRZ_Tick, delay, false);
	}

	protected bool SRZ_IsActive()
	{
		if (!m_bSRZ_LocalOn || SRZ_BatteryDead() || GetMode() != EGadgetMode.IN_HAND)
			return false;

		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(SCR_PlayerController.GetLocalPlayerId());
		return player && GetOwner().GetRootParent() == player && IsPlayerAlive();
	}

	protected bool SRZ_FindNearest()
	{
		m_SRZ_Nearest = null;
		m_fSRZ_NearestDist = float.MAX;
		GetGame().GetWorld().QueryEntitiesBySphere(GetOwner().GetOrigin(), m_fDistance_Find_Art, SRZ_QueryArtifact);
		return m_SRZ_Nearest != null;
	}

	protected bool SRZ_QueryArtifact(IEntity ent)
	{
		if (!ent || ent.GetParent() || !ent.FindComponent(ARMST_ARTEFACT_COMPONENTS))
			return true;

		float dist = vector.Distance(GetOwner().GetOrigin(), ent.GetOrigin());
		if (dist < m_fSRZ_NearestDist)
		{
			m_fSRZ_NearestDist = dist;
			m_SRZ_Nearest = ent;
		}
		return true;
	}

	protected void SRZ_Feedback(IEntity artifact, float distance)
	{
		ARMST_ARTEFACT_COMPONENTS comp = ARMST_ARTEFACT_COMPONENTS.Cast(artifact.FindComponent(ARMST_ARTEFACT_COMPONENTS));
		if (!comp)
			return;

		// ARMST's Voron arrow needs the player (it used to be set by the old toggle)
		m_Player = GetGame().GetPlayerManager().GetPlayerControlledEntity(SCR_PlayerController.GetLocalPlayerId());

		if (!m_bSRZ_Tracking)
		{
			m_bSRZ_Tracking = true;
			m_bIsScanning = true;
			m_bArtDetectorState = true;
			EnableVisualFeedback();
			if (m_DetectorType == ARMST_DETECTOR.DETECTOR_OLIMP)
				StartBlinking(distance);
		}

		// ARMST appends to this string on every light change; keep it from growing forever
		if (m_RemappingString.Length() > 4000)
			m_RemappingString = "";

		bool hidden = comp.getArtVisible();

		// A visible artifact must not go through ARMST's reveal (it would hide it again after a timer),
		// so feed the lights a distance just outside the reveal radius
		float feedDistance = distance;
		if (!hidden && distance < m_fDistance_Founded)
			feedDistance = m_fDistance_Founded + 0.01;

		switch (m_DetectorType)
		{
			case ARMST_DETECTOR.DETECTOR_MUHA:   UpdateMuhaFeedback(feedDistance, comp); break;
			case ARMST_DETECTOR.DETECTOR_OLIMP:  UpdateOlimpFeedback(feedDistance, comp); break;
			case ARMST_DETECTOR.DETECTOR_VORON:  UpdateVoronFeedback(feedDistance, comp); break;
			case ARMST_DETECTOR.DETECTOR_DIATEL: UpdateDiatelFeedback(feedDistance, comp); break;
		}

		bool revealedNow = hidden && !comp.getArtVisible();
		bool reachedVisible = !hidden && distance < m_fDistance_Founded && m_SRZ_Announced != artifact;
		if (revealedNow || reachedVisible)
		{
			if (reachedVisible)
			{
				SCR_SoundManagerModule soundManager = SCR_SoundManagerModule.GetInstance(GetOwner().GetWorld());
				if (soundManager)
					soundManager.CreateAndPlayAudioSource(GetOwner(), "DETECTOR_ART_LEVEL_ANALIS_FOUNDED_ART");
			}
			SRZ_OnReveal(artifact);
		}
	}

	// Rapid blink of the detector's own lights/screen when something is found
	protected int m_iSRZ_FlashStep;

	protected void SRZ_FlashDetector()
	{
		m_iSRZ_FlashStep = 0;
		GetGame().GetCallqueue().Remove(SRZ_FlashStep);
		SRZ_FlashStep();
	}

	protected void SRZ_FlashStep()
	{
		bool on = (m_iSRZ_FlashStep % 2) == 0;

		switch (m_DetectorType)
		{
			case ARMST_DETECTOR.DETECTOR_MUHA:
				if (on) { EnableMuhaLight(); EnableMuhaLight2(); }
				else { DisableMuhaLight(); DisableMuhaLight2(); }
				break;
			case ARMST_DETECTOR.DETECTOR_OLIMP:
				SetOlimpLightState(on);
				break;
			case ARMST_DETECTOR.DETECTOR_DIATEL:
				if (on)
				{
					for (int i = 2; i <= 8; i++)
						SetDiatelLedState(i, true, m_Diatel_Led_On_Red);
				}
				else
				{
					DisableDiatelLights();
				}
				break;
			case ARMST_DETECTOR.DETECTOR_VORON:
				if (on)
				{
					if (m_wVoronRing_40m) m_wVoronRing_40m.SetColorInt(ARGB(255, 255, 0, 0));
					if (m_wVoronRing_30m) m_wVoronRing_30m.SetColorInt(ARGB(255, 255, 0, 0));
					if (m_wVoronRing_20m) m_wVoronRing_20m.SetColorInt(ARGB(255, 255, 0, 0));
					if (m_wVoronRing_10m) m_wVoronRing_10m.SetColorInt(ARGB(255, 255, 0, 0));
					if (m_wVoronRing_5m) m_wVoronRing_5m.SetColorInt(ARGB(255, 255, 0, 0));
					if (m_wVoronRing_3m) m_wVoronRing_3m.SetColorInt(ARGB(255, 255, 0, 0));
				}
				else
				{
					UpdateVoronRingVisuals(999);
				}
				break;
		}

		m_iSRZ_FlashStep++;
		if (m_iSRZ_FlashStep < 6)
			GetGame().GetCallqueue().CallLater(SRZ_FlashStep, 150, false);
	}

	protected void SRZ_OnReveal(IEntity artifact)
	{
		m_SRZ_Announced = artifact;
		SRZ_FlashDetector();

		vector mat[4];
		artifact.GetTransform(mat);

		ParticleEffectEntitySpawnParams fxParams = new ParticleEffectEntitySpawnParams();
		fxParams.TransformMode = ETransformMode.WORLD;
		fxParams.Transform[0] = mat[0];
		fxParams.Transform[1] = mat[1];
		fxParams.Transform[2] = mat[2];
		fxParams.Transform[3] = mat[3];
		fxParams.PlayOnSpawn = true;
		fxParams.DeleteWhenStopped = true;
		ParticleEffectEntity.SpawnParticleEffect(SRZ_REVEAL_FX, fxParams);

		SRZ_Notify("Something is here...");
	}

	protected void SRZ_StopTracking()
	{
		m_bSRZ_Tracking = false;
		m_SRZ_Announced = null;
		m_bIsScanning = false;
		m_bArtDetectorState = false;

		if (m_DetectorType == ARMST_DETECTOR.DETECTOR_OLIMP)
			StopBlinking();

		DisableVisualFeedback();
	}
}

modded class SCR_PlayerController
{
	void SRZ_RequestDetectorPower(IEntity detector, bool on)
	{
		if (!detector)
			return;

		if (Replication.IsServer())
		{
			SRZ_ApplyDetectorPower(detector, on);
			return;
		}

		Rpc(RpcAsk_SRZ_DetectorPower, SCR_EntityHelper.EntityToRplId(detector), on);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_SRZ_DetectorPower(RplId detectorId, bool on)
	{
		IEntity detector = SCR_EntityHelper.RplIdToEntity(detectorId);
		if (!detector)
			return;

		// Only the player carrying it can switch it
		IEntity player = GetMainEntity();
		if (!player || detector.GetRootParent() != player)
			return;

		SRZ_ApplyDetectorPower(detector, on);
	}

	protected void SRZ_ApplyDetectorPower(IEntity detector, bool on)
	{
		ARMST_DETECTOR_ARTS_COMPONENTS comp = ARMST_DETECTOR_ARTS_COMPONENTS.Cast(detector.FindComponent(ARMST_DETECTOR_ARTS_COMPONENTS));
		if (!comp)
			return;

		comp.SRZ_ServerSetOn(on);
		SRZ_SendDetectorState(detector, comp.SRZ_IsOn());
	}

	void SRZ_SendDetectorState(IEntity detector, bool on)
	{
		if (!detector)
			return;

		RplComponent rpl = RplComponent.Cast(FindComponent(RplComponent));
		if (!rpl || rpl.IsOwner())
		{
			SRZ_ReceiveDetectorState(detector, on);
			return;
		}

		Rpc(RpcDo_SRZ_DetectorState, SCR_EntityHelper.EntityToRplId(detector), on);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_SRZ_DetectorState(RplId detectorId, bool on)
	{
		SRZ_ReceiveDetectorState(SCR_EntityHelper.RplIdToEntity(detectorId), on);
	}

	protected void SRZ_ReceiveDetectorState(IEntity detector, bool on)
	{
		if (!detector)
			return;

		ARMST_DETECTOR_ARTS_COMPONENTS comp = ARMST_DETECTOR_ARTS_COMPONENTS.Cast(detector.FindComponent(ARMST_DETECTOR_ARTS_COMPONENTS));
		if (comp)
			comp.SRZ_ClientSetOn(on);
	}
}
