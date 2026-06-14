typedef func ADB_BackendInfoReceived_Event;
void ADB_BackendInfoReceived_Event(ADB_BackendInfoComponent backendInfoComponent);
typedef ScriptInvokerBase<ADB_BackendInfoReceived_Event> ADB_BackendInfoReceived_Invoker;

[ComponentEditorProps(category: "ADB/Components/PlayerController", description: "Holds important backend info for ADB to function. Should be attached to your main player controller")]
class ADB_BackendInfoComponentClass : ScriptComponentClass {}

class ADB_BackendInfoComponent : ScriptComponent
{
	protected int m_backendId;
	protected int m_sessionId;
	
	protected ref ADB_BackendInfoReceived_Invoker m_onBackendInfoReceivedInvoker;
	
	void ADB_BackendInfoComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_onBackendInfoReceivedInvoker = new ADB_BackendInfoReceived_Invoker();
	}
	
	static bool IsValidBackendId(int backendId)
	{
		return backendId > 1;
	}
	
	void SetInfo(int backendId, int sessionId)
	{
		m_backendId = backendId;
		m_sessionId = sessionId;
		
		m_onBackendInfoReceivedInvoker.Invoke(this);
	}
	
	int GetBackendId()
	{
		return m_backendId;
	}
	
	int GetSessionId()
	{
		return m_sessionId;
	}
	
	ADB_BackendInfoReceived_Invoker GetInvokerOnBackendInfoReceived()
	{
		return m_onBackendInfoReceivedInvoker;
	}
}