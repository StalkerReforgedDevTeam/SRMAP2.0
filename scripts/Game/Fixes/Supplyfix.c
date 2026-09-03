//------------------------------------------------------------------------------------------------
// Standalone fix - does NOT touch the original ARMST_DIALOGS_COMPONENT file at all.
// Put this at scripts/Game/Fixes/Supplyfix.c (replacing what's there now).
//
// Uses "modded class" - the same mechanism your own code already uses for
// "modded class SCR_PlayerController" - to add a new field and override two existing methods on
// a class you don't own the source of. Nothing in the original ARMST files is edited.
//
// TWO THINGS THIS FIXES:
//
// 1) LoadInitialTraderStock() bug: once a trader's stock file exists (created on its very
//    first-ever load), the database was never scanned again - any item added to
//    ARMST_DATABASE_ITEM after that point was permanently invisible to the stock system.
//    Fixed: load the file first (existing counts aren't reset), then always also scan the
//    database and add anything not already cached. ALSO reconciles the other direction: an item
//    that was tracked before but has since been set back to unlimited (Trader Count = -1) in the
//    database is now removed from the cache instead of staying stuck at its last cached number.
//
// 2) NEW: a per-item stock CAP. Right now ChangeTraderStock() only clamps stock at a minimum of
//    0 - there's no upper bound, so repeatedly turning in supply crates (or selling an item back)
//    can push stock above whatever m_fTraderCount you configured. This adds a new field,
//    m_mTraderStockMax, populated from m_fTraderCount at the same time as the stock cache, and
//    clamps every stock change to it. m_fTraderCount now does double duty: it's both the
//    starting stock AND the cap - set it once per item per trader in the Workbench like you
//    already do, no new attribute needed.
//------------------------------------------------------------------------------------------------

modded class ARMST_DIALOGS_COMPONENT
{
	// NEW field: per-item stock cap, keyed the same way as m_mTraderStock (via GetStockKey).
	// -1 or missing = no cap tracked (shouldn't happen for anything in m_mTraderStock, since an
	// item only gets added to that cache when it HAS a configured m_fTraderCount >= 0).
	protected ref map<string, int> m_mTraderStockMax = new map<string, int>();

	//------------------------------------------------------------------------------------------------
	//! Optional getter if you want to show "12 / 25" in the UI later - not required for the cap
	//! itself to work, just convenient to have.
	int GetTraderStockMaxLocal(ResourceName prefabName)
	{
		string key = GetStockKey(m_Actor, prefabName);
		if (m_mTraderStockMax.Contains(key))
			return m_mTraderStockMax.Get(key);
		return -1;
	}

	//------------------------------------------------------------------------------------------------
	override void LoadInitialTraderStock()
	{
		if (!Replication.IsServer())
		{
			return;
		}
		if (m_Actor.IsEmpty())
		{
			return;
		}
		m_mTraderStock.Clear();
		m_mTraderStockMax.Clear();

		bool fileExisted = ARMST_TraderStockFileManager.StockFileExists(m_Actor);
		bool addedNewEntries = false;

		// Step 1: load whatever is already persisted, if anything. Preserves current stock
		// counts instead of resetting everything back to database defaults on every load.
		if (fileExisted)
		{
			ref map<ResourceName, int> fileStock = ARMST_TraderStockFileManager.LoadStockFromFile(m_Actor);
			foreach (ResourceName prefab, int count : fileStock)
			{
				m_mTraderStock.Set(prefab, count);
			}
			Print("[ARMST TRADER] Кэш загружен из файла: " + m_mTraderStock.Count() + " товаров", LogLevel.NORMAL);
		}

		// Step 2: ALWAYS scan the database too (this used to only run when no file existed yet -
		// that was bug #1). For every item with a configured cap, record the cap in
		// m_mTraderStockMax, and if it isn't already in m_mTraderStock (from the file), seed its
		// starting stock from the same value. Never overwrite an existing m_mTraderStock entry -
		// only m_mTraderStockMax is always refreshed from the current database config, since the
		// cap should reflect whatever you've most recently set in the Workbench.
		ARMST_EDITOR_GLOBAL_SETTINGS db = ARMST_EDITOR_GLOBAL_SETTINGS.GetInstance();
		if (db)
		{
			array<ref array<ref ResourceName>> allCategories = {
				m_sTraderCategory1, m_sTraderCategory2, m_sTraderCategory3,
				m_sTraderCategory4, m_sTraderCategory5, m_sTraderCategory6,
				m_sTraderCategory7, m_sTraderCategory8, m_sTraderCategory9,
				m_sTraderCategory10, m_sTraderCategory11
			};

			foreach (array<ref ResourceName> category : allCategories)
			{
				if (!category || category.IsEmpty())
					continue;

				foreach (ResourceName configResource : category)
				{
					if (configResource.IsEmpty() || !configResource.StartsWith("{"))
						continue;

					ARMST_DATABASE_ITEM dbItem = db.FindItemByPrefab(configResource);
					if (!dbItem || !dbItem.m_ItemTrader)
						continue;

					// Same "specific actor wins, fall back to ALL" priority used elsewhere in
					// the original class (LoadTraderCategory, FindTraderItemData, etc).
					ARMST_DATABASE_ITEM_TRADER specificTraderInfo = null;
					ARMST_DATABASE_ITEM_TRADER allTraderInfo = null;
					foreach (ARMST_DATABASE_ITEM_TRADER traderInfo : dbItem.m_ItemTrader)
					{
						if (traderInfo.m_Actor == m_Actor)
						{
							specificTraderInfo = traderInfo;
							break;
						}
						else if (traderInfo.m_Actor == "ALL")
						{
							allTraderInfo = traderInfo;
						}
					}

					ARMST_DATABASE_ITEM_TRADER selectedTraderInfo = specificTraderInfo;
					if (!selectedTraderInfo)
						selectedTraderInfo = allTraderInfo;

					// m_fTraderCount < 0 (the -1 default) means unlimited at this trader -
					// no cap, no stock tracking, same meaning as before this fix.
					if (!selectedTraderInfo || selectedTraderInfo.m_fTraderCount < 0)
						continue;

					int configuredMax = (int)selectedTraderInfo.m_fTraderCount;
					m_mTraderStockMax.Set(configResource, configuredMax);

					if (!m_mTraderStock.Contains(configResource))
					{
						m_mTraderStock.Set(configResource, configuredMax);
						addedNewEntries = true;
					}
				}
			}
		}

		// Step 2.5: reconcile removals. Step 2 only ever ADDS items that the database says should
		// be tracked - it never un-tracks one you've since set back to unlimited (Trader Count =
		// -1) or removed the trader entry for. Without this, an item that was EVER capped once
		// stays stuck at its last cached number forever, no matter what you change the database
		// to afterward. Anything currently in m_mTraderStock that the scan above did NOT just
		// confirm as still-tracked (i.e. isn't in m_mTraderStockMax) gets dropped, so
		// GetTraderStockLocal() goes back to reporting -1 (unlimited) for it.
		ref array<string> keysToRemove = new array<string>();
		foreach (string existingKey, int existingCount : m_mTraderStock)
		{
			if (!m_mTraderStockMax.Contains(existingKey))
				keysToRemove.Insert(existingKey);
		}
		foreach (string removeKey : keysToRemove)
		{
			m_mTraderStock.Remove(removeKey);
			addedNewEntries = true; // reuse this flag to mean "cache changed, needs re-save"
			Print("[ARMST TRADER] Позиция больше не ограничена в БД, снята с учёта: " + removeKey, LogLevel.NORMAL);
		}

		// Step 3: persist. Always write on first-ever load; otherwise only rewrite if the scan
		// actually added or removed something, so an unchanged trader doesn't touch its file
		// every load. (m_mTraderStockMax is rebuilt fresh every load from the database, not
		// persisted to file - it should always reflect the current Workbench config, not a stale
		// saved value.)
		if (!fileExisted)
		{
			ARMST_TraderStockFileManager.CreateInitialStockFile(m_Actor, m_mTraderStock);
			Print("[ARMST TRADER] Создан файл: " + m_mTraderStock.Count() + " товаров", LogLevel.NORMAL);
		}
		else if (addedNewEntries)
		{
			ARMST_TraderStockFileManager.SaveStockToFile(m_Actor, m_mTraderStock);
			Print("[ARMST TRADER] Файл обновлён по текущей БД: итого " + m_mTraderStock.Count() + " товаров", LogLevel.NORMAL);
		}

		m_bStockLoaded = true;
	}

	//------------------------------------------------------------------------------------------------
	override void ChangeTraderStock(ResourceName prefabName, int delta)
	{
		if (!Replication.IsServer())
			return;

		EnsureStockLoaded();

		string key = GetStockKey(m_Actor, prefabName);
		int currentStock = GetTraderStockLocal(prefabName);

		if (currentStock == -1)
		{
			// Untracked/unlimited item - nothing to clamp or persist, same as before this fix.
			// Still process supply cascades for it if it was sold (delta > 0).
			if (delta > 0)
				ProcessSupplyItems(prefabName, delta);
			return;
		}

		int newStock = currentStock + delta;
		if (newStock < 0)
			newStock = 0;

		// NEW: clamp to the configured cap, if we have one recorded for this item.
		if (m_mTraderStockMax.Contains(key))
		{
			int maxStock = m_mTraderStockMax.Get(key);
			if (newStock > maxStock)
				newStock = maxStock;
		}

		m_mTraderStock.Set(key, newStock);
		ARMST_TraderStockFileManager.SaveStockToFile(m_Actor, m_mTraderStock);
		BroadcastStockUpdate(prefabName, newStock);
		Print("[ARMST TRADER] Stock: " + prefabName + " " + currentStock + " -> " + newStock, LogLevel.NORMAL);

		if (delta > 0)
			ProcessSupplyItems(prefabName, delta);
	}
}

//------------------------------------------------------------------------------------------------
// Optional, unrelated: ProcessSupplyItems() in the original class has leftover debug Print()
// calls that just spam the log on every sale of a supply-linked item. Can't be removed via
// modded class (only whole methods can be overridden, not individual lines from one you don't
// own) - only worth doing if you get write access to the original file later.