// The save database load spawns every saved stash, shelter and storage in one frame. With around 12,000
// saved items that frame runs past the host's 120 second freeze check and the watchdog kills the server
// on every start. This spreads the spawning over frames with a time budget, so the server keeps ticking.
// The next step is always scheduled before any entity is spawned, so one broken saved entity cannot stall the load.
modded class EPF_PersistenceManager
{
	protected static const int SRZ_BUDGET_MS = 80;
	protected static const int SRZ_STEP_MS = 10;

	protected ref array<ref EDF_DbEntity> m_aSRZ_LoadQueue = {};
	protected int m_iSRZ_LoadIndex;
	protected bool m_bSRZ_LoadRunning;
	protected int m_iSRZ_LoadStart;

	//------------------------------------------------------------------------------------------------
	override void OnTypeCollectionLoaded(EDF_EDbOperationStatusCode code, array<ref EDF_DbEntity> findResults)
	{
		foreach (EDF_DbEntity findResult : findResults)
		{
			m_aSRZ_LoadQueue.Insert(findResult);
		}

		m_iPendingLoadTypes--;

		if (m_bSRZ_LoadRunning)
			return;

		m_bSRZ_LoadRunning = true;
		m_iSRZ_LoadStart = System.GetTickCount();
		Print("[SRZ Save] Staggered load started", LogLevel.NORMAL);
		GetGame().GetCallqueue().CallLater(SRZ_LoadStep, SRZ_STEP_MS, false);
	}

	//------------------------------------------------------------------------------------------------
	protected void SRZ_LoadStep()
	{
		if (m_iSRZ_LoadIndex >= m_aSRZ_LoadQueue.Count() && m_iPendingLoadTypes <= 0)
		{
			SRZ_LoadFinished();
			return;
		}

		GetGame().GetCallqueue().CallLater(SRZ_LoadStep, SRZ_STEP_MS, false);

		int started = System.GetTickCount();
		while (m_iSRZ_LoadIndex < m_aSRZ_LoadQueue.Count())
		{
			EDF_DbEntity findResult = m_aSRZ_LoadQueue[m_iSRZ_LoadIndex];
			m_iSRZ_LoadIndex++;
			SRZ_LoadOne(findResult);

			if (System.GetTickCount() - started >= SRZ_BUDGET_MS)
				break;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SRZ_LoadOne(EDF_DbEntity findResult)
	{
		EPF_EntitySaveData saveData = EPF_EntitySaveData.Cast(findResult);
		if (!saveData)
		{
			Debug.Error(string.Format("Unexpected database find result type '%1' encountered during entity load. Ignored.", findResult.Type().ToString()));
			return;
		}

		EPF_PersistenceComponent persistenceComponent = m_mBakedRoots.Get(saveData.GetId());
		if (persistenceComponent)
		{
			if (persistenceComponent.GetOwner())
				persistenceComponent.Load(saveData);

			return;
		}

		SpawnWorldEntity(saveData);
	}

	//------------------------------------------------------------------------------------------------
	protected void SRZ_LoadFinished()
	{
		int seconds = (System.GetTickCount() - m_iSRZ_LoadStart) / 1000;
		Print(string.Format("[SRZ Save] Staggered load finished, %1 saved root entities in %2 s", m_iSRZ_LoadIndex, seconds), LogLevel.NORMAL);

		m_aSRZ_LoadQueue.Clear();
		m_iSRZ_LoadIndex = 0;
		m_bSRZ_LoadRunning = false;

		m_mBakedRoots = null;
		OnSetup();
		GetGame().GetCallqueue().CallLater(TryCompleteSetup, 100, true);
	}
}
