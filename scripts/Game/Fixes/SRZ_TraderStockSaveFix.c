// scripts/Game/Fixes/SRZ_TraderStockSaveFix.c

modded class ARMST_DIALOGS_COMPONENT
{
    protected bool m_bSRZ_StockSaveDirty = false;
    protected bool m_bSRZ_StockSavePending = false;

    //------------------------------------------------------------------------------------------------
    override void ChangeTraderStock(ResourceName prefabName, int delta)
    {
        if (!Replication.IsServer())
            return;

        EnsureStockLoaded();

        string key = GetStockKey(m_Actor, prefabName);
        int currentStock = GetTraderStockLocal(prefabName);

        int newStock = currentStock + delta;
        if (newStock < 0)
            newStock = 0;

        if (currentStock == -1)
        {
            if (delta > 0)
                ProcessSupplyItems(prefabName, delta);
            return;
        }

        m_mTraderStock.Set(key, newStock);

        SRZ_ScheduleStockSave();
        BroadcastStockUpdate(prefabName, newStock);

        Print("[ARMST TRADER] Stock: " + prefabName + " " + currentStock + " -> " + newStock, LogLevel.NORMAL);

        if (delta > 0)
            ProcessSupplyItems(prefabName, delta);
    }

    //------------------------------------------------------------------------------------------------
    override void AddSupplyToTraderStock(ResourceName supplyPrefabName, int amount)
    {
        if (!Replication.IsServer())
            return;

        EnsureStockLoaded();

        string key = GetStockKey(m_Actor, supplyPrefabName);
        int currentStock = GetTraderStockLocal(supplyPrefabName);

        if (currentStock == -1)
            currentStock = 0;

        int newStock = currentStock + amount;
        m_mTraderStock.Set(key, newStock);

        SRZ_ScheduleStockSave();
        BroadcastStockUpdate(supplyPrefabName, newStock);

        Print("[ARMST TRADER] Supply stock updated: " + supplyPrefabName + " " + currentStock + " -> " + newStock, LogLevel.NORMAL);
    }

    //------------------------------------------------------------------------------------------------
    // Multiple stock changes within the delay window collapse into a single disk write.
    protected void SRZ_ScheduleStockSave()
    {
        m_bSRZ_StockSaveDirty = true;

        if (m_bSRZ_StockSavePending)
            return;

        m_bSRZ_StockSavePending = true;
        GetGame().GetCallqueue().CallLater(SRZ_DoDeferredStockSave, 5000, false);
    }

    //------------------------------------------------------------------------------------------------
    protected void SRZ_DoDeferredStockSave()
    {
        m_bSRZ_StockSavePending = false;

        if (!m_bSRZ_StockSaveDirty)
            return;

        m_bSRZ_StockSaveDirty = false;
        ARMST_TraderStockFileManager.SaveStockToFile(m_Actor, m_mTraderStock);
    }
};