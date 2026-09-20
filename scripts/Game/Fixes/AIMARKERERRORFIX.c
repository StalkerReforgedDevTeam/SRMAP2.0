modded class SCR_AIEnemyMarkingSystem
{
	protected override void OnUpdatePoint(WorldUpdatePointArgs args)
	{
		SCR_MapMarkerManagerComponent markerMgr = SCR_MapMarkerManagerComponent.GetInstance();
		for (int idx = m_mPlacedMarkersFactions.Count() - 1; idx >= 0; idx--)
		{
			SCR_MapMarkerBase marker = m_mPlacedMarkersFactions.GetKey(idx);
			if (!marker)
			{
				m_mPlacedMarkersFactions.RemoveElement(idx);
				continue;
			}

			if (marker.GetTimestamp())
				continue;

			if (markerMgr)
				markerMgr.RemoveStaticMarker(marker);
			m_mPlacedMarkersFactions.RemoveElement(idx);
		}

		super.OnUpdatePoint(args);
	}

	override void MarkTarget(notnull SCR_AITargetInfo targetInfo, notnull SCR_AIGroup spottingGroup)
	{
		ChimeraWorld world = GetWorld();
		SCR_Faction spottingGroupFaction = SCR_Faction.Cast(spottingGroup.GetFaction());
		if (!spottingGroupFaction || !spottingGroupFaction.CanAIMarkTargets())
			return;

		WorldTimestamp currentTime = world.GetServerTimestamp();
		WorldTimestamp markerUpdateCutoffTime = currentTime.PlusSeconds(-m_fGroupMarkingCooldown);

		EMilitarySymbolIdentity identity = GetIdentitySymbol(targetInfo.m_Faction);
		EMilitarySymbolIcon icon = GetMilitarySymbolFromTarget(targetInfo);

		array<SCR_MapMarkerBase> overriddenMarkers = {};

		WorldTimestamp previousTime;
		float markerSqDistance;
		float minimumSqDistance = Math.Pow(GetMinimumMarkDistanceFromType(icon), 2);
		foreach (SCR_MapMarkerBase thisMarker, EMilitarySymbolIdentity thisFactionIdentitySymbol : m_mPlacedMarkersFactions)
		{
			if (icon != thisMarker.GetFlags()
				|| identity != thisFactionIdentitySymbol
				|| thisMarker.GetMarkerFactionFlags() != Math.Pow(2, spottingGroup.GetFactionIndex()))
				continue;

			markerSqDistance = GetMarkerDistanceSqFromPoint(targetInfo.m_vWorldPos, thisMarker);

			if (markerSqDistance < minimumSqDistance)
			{
				previousTime = thisMarker.GetTimestamp();

				if (previousTime.Greater(markerUpdateCutoffTime))
					return;

				overriddenMarkers.Insert(thisMarker);
			}
		}

		SCR_MapMarkerManagerComponent markerMgr = SCR_MapMarkerManagerComponent.GetInstance();
		if (!markerMgr)
			return;

		SCR_MapMarkerBase marker = markerMgr.PrepareMilitaryMarker(identity, EMilitarySymbolDimension.LAND, icon);
		if (!marker)
			return;

		IEntity leaderEntity = spottingGroup.GetLeaderEntity();
		if (!leaderEntity)
			return;

		float distance = vector.Distance(targetInfo.m_vWorldPos, leaderEntity.GetOrigin());
		vector randomness = SCR_Math.GetMathRandomGenerator().GenerateRandomPointInRadius(0, m_fBaseInaccuracy + distance * m_fInaccuracyByDistance, vector.Zero, false);

		marker.SetWorldPos(targetInfo.m_vWorldPos[0] + randomness[0], targetInfo.m_vWorldPos[2] + randomness[1]);
		marker.SetMarkerFactionFlags(Math.Pow(2, spottingGroup.GetFactionIndex()));
		marker.SetTimestamp(currentTime);
		marker.SetTimestampVisibility(true);
		marker.SetDragged(false);

		TimeContainer ingameTime = world.GetTimeAndWeatherManager().GetTime();
		marker.SetCustomText(string.Format(FORMAT_MILITARY_TIME, ingameTime.m_iHours.ToString(2), ingameTime.m_iSeconds.ToString(2)));

		markerMgr.InsertStaticMarker(marker, false, true);
		m_mPlacedMarkersFactions.Insert(marker, identity);

		for (int idx = overriddenMarkers.Count() - 1; idx >= 0; idx--)
		{
			SCR_MapMarkerBase overriddenMarker = overriddenMarkers[idx];
			markerMgr.RemoveStaticMarker(overriddenMarker);
			m_mPlacedMarkersFactions.Remove(overriddenMarker);
		}
	}
}