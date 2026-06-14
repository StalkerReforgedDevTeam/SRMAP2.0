// Client-side component to handle notifications
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
	
	// Singleton
	static SRZ_RPNotificationHandler GetInstance()
	{
		return s_Instance;
	}
	
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		SetEventMask(owner, EntityEvent.FRAME);
		s_Instance = this;
		
		Print("[SRZ_RP] Notification handler initialized", LogLevel.NORMAL);
	}
	
	// Queue a message to display
	void QueueMessage(string message)
	{
		Print(string.Format("[SRZ_RP] Queueing message: %1", message), LogLevel.NORMAL);
		
		// Check if this is a list header (starts with ===)
		if (message.StartsWith("==="))
		{
			// If we were buffering a previous list, show it first
			if (m_IsBufferingList && m_ListBuffer.Count() > 0)
			{
				ShowBufferedList();
			}
			
			// Start new list buffer
			m_IsBufferingList = true;
			m_ListBuffer.Clear();
			m_ListBuffer.Insert(message);
		}
		// Check if this is a list item (starts with spaces/dashes or is admin header)
		else if (m_IsBufferingList)
		{
			m_ListBuffer.Insert(message);
			
			// Give it a moment to collect all list items, then show
			GetGame().GetCallqueue().CallLater(ShowBufferedList, 500, false);
		}
		else
		{
			// Regular single message - show immediately
			m_MessageQueue.Insert(message);
		}
	}
	
	// Show all buffered list items as one notification
	protected void ShowBufferedList()
	{
		if (!m_IsBufferingList || m_ListBuffer.Count() == 0)
			return;
			
		m_IsBufferingList = false;
		
		// Combine all list items into one message
		string combinedMessage = "";
		for (int i = 0; i < m_ListBuffer.Count(); i++)
		{
			combinedMessage += m_ListBuffer[i];
			if (i < m_ListBuffer.Count() - 1)
				combinedMessage += "\n";
		}
		
		Print(string.Format("[SRZ_RP] Showing combined list with %1 lines", m_ListBuffer.Count()), LogLevel.NORMAL);
		
		// Show as hint with longer duration
		SCR_HintManagerComponent hintMgr = SCR_HintManagerComponent.GetInstance();
		if (hintMgr)
		{
			// Show for 10 seconds for lists
			hintMgr.ShowCustomHint(combinedMessage, "RP Name System", 10);
			Print("[SRZ_RP] List shown via hint manager", LogLevel.NORMAL);
		}
		else
		{
			Print("[SRZ_RP] Hint manager not available, list only in console", LogLevel.WARNING);
			Print(combinedMessage, LogLevel.NORMAL);
		}
		
		m_ListBuffer.Clear();
	}
	
	// Process messages on frame update
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (m_MessageQueue.Count() == 0)
			return;
			
		if (m_IsProcessing)
			return;
		
		m_IsProcessing = true;
		
		// Process one message per frame to avoid spam
		string msg = m_MessageQueue[0];
		m_MessageQueue.Remove(0);
		
		ShowMessageOnScreen(msg);
		
		// Reset flag after a short delay
		GetGame().GetCallqueue().CallLater(ResetProcessing, 100, false);
	}
	
	protected void ResetProcessing()
	{
		m_IsProcessing = false;
	}
	
	// Show single message using popup
	protected void ShowMessageOnScreen(string message)
	{
		Print(string.Format("[SRZ_RP] Displaying single message: %1", message), LogLevel.NORMAL);
		
		// Use popup for single messages
		SCR_PopUpNotification popup = SCR_PopUpNotification.GetInstance();
		if (popup)
		{
			popup.PopupMsg(message, 5.0);
			Print("[SRZ_RP] Message shown via popup", LogLevel.NORMAL);
			return;
		}
		
		// Fallback to hint
		SCR_HintManagerComponent hintMgr = SCR_HintManagerComponent.GetInstance();
		if (hintMgr)
		{
			hintMgr.ShowCustomHint(message, "RP Name", 5);
			Print("[SRZ_RP] Message shown via hint", LogLevel.NORMAL);
			return;
		}
		
		Print("[SRZ_RP] No display method available, message only in console", LogLevel.WARNING);
	}
}