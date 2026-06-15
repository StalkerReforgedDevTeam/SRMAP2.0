class SRZ_RPNotificationHandlerClass : ScriptComponentClass
{
}

class SRZ_RPNotificationHandler : ScriptComponent
{
    protected static SRZ_RPNotificationHandler s_Instance;

    protected ref array<string> m_MessageQueue = new array<string>();
    protected ref array<string> m_ListBuffer = new array<string>();
    protected bool m_IsProcessing = false;
    protected bool m_IsBufferingList = false;

    static SRZ_RPNotificationHandler GetInstance()
    {
        return s_Instance;
    }

    override void OnPostInit(IEntity owner)
    {
        super.OnPostInit(owner);
        SetEventMask(owner, EntityEvent.FRAME);
        s_Instance = this;
    }

    void QueueMessage(string message)
    {
        if (message.IsEmpty())
            return;

        m_MessageQueue.Insert(message);

        if (!m_IsProcessing)
            GetGame().GetCallqueue().CallLater(ProcessNextMessage, 100, false);
    }

    protected void ProcessNextMessage()
    {
        if (m_MessageQueue.IsEmpty())
        {
            m_IsProcessing = false;
            return;
        }

        m_IsProcessing = true;
        string message = m_MessageQueue[0];
        m_MessageQueue.RemoveOrdered(0);

        SCR_PopUpNotification popup = SCR_PopUpNotification.GetInstance();
        if (popup)
            popup.PopupMsg(message, 5.0);

        GetGame().GetCallqueue().CallLater(ProcessNextMessage, 500, false);
    }
}