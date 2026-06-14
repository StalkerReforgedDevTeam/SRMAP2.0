class ADB_Supabase
{
	protected string m_domain;
	protected string m_uriStub;
	protected string m_apiKey;
	
	void ADB_Supabase(string domain, string stub, string apiKey)
	{
		m_domain = domain;
		m_uriStub = stub;
		m_apiKey = apiKey;
	}
	
	bool Select(string tableName, string query = "*", string data = "", RestCallback cb = null)
	{
		string uri = string.Format("https:/\/%1.%2/rest/v1/%3?select=%4", m_uriStub, m_domain, tableName, query);
		string fullRestUrl = uri;
		
		RestContext ctx = GetGame().GetRestApi().GetContext(uri);
        if (!ctx)
        {
            Print("ERROR: Failed to create REST context", LogLevel.ERROR);
            return false;
        }
		
		ctx.SetHeaders(string.Format("Content-Type,application/json,apikey,%1,Authorization,Bearer %1", m_apiKey));
		ctx.GET(cb, data);
		
		return true;
	}
	
	bool Insert(string tableName, JsonApiStruct data, RestCallback cb = null)
	{
		data.Pack();
		return InsertRaw(tableName, data.AsString(), cb);
	}
	
	bool InsertRaw(string tableName, string data = "", RestCallback cb = null)
	{
		string uri = string.Format("https:/\/%1.%2/rest/v1/%3", m_uriStub, m_domain, tableName);
		string fullRestUrl = uri;
		
		RestContext ctx = GetGame().GetRestApi().GetContext(uri);
        if (!ctx)
        {
            Print("ERROR: Failed to create REST context", LogLevel.ERROR);
            return false;
        }
		
		ctx.SetHeaders(string.Format("Content-Type,application/json,apikey,%1,Authorization,Bearer %1", m_apiKey));
		ctx.POST(cb, "", data);
		
		return true;
	}
	
	bool Upsert(string tableName, JsonApiStruct data, RestCallback cb = null)
	{
		data.Pack();
		return UpsertRaw(tableName, data.AsString(), cb);
	}
	
	bool UpsertRaw(string tableName, string data, RestCallback cb = null)
	{
		string uri = string.Format("https:/\/%1.%2/rest/v1/%3", m_uriStub, m_domain, tableName);
		string fullRestUrl = uri;
		
		RestContext ctx = GetGame().GetRestApi().GetContext(uri);
        if (!ctx)
        {
            Print("ERROR: Failed to create REST context", LogLevel.ERROR);
            return false;
        }
		
		ctx.SetHeaders(string.Format("Content-Type,application/json,apikey,%1,Authorization,Bearer %1", m_apiKey));
		ctx.POST(cb, "", data);
		
		return true;
	}
	
	bool CallFunc(string funcName, JsonApiStruct args, RestCallback cb = null)
	{
		args.Pack();
		return CallFuncRaw(funcName, args.AsString(), cb);
	}
	
	bool CallFuncRaw(string funcName, string args, RestCallback cb = null)
	{
		string uri = string.Format("https:/\/%1.%2/rest/v1/rpc/%3", m_uriStub, m_domain, funcName);
		string fullRestUrl = uri;
		
		RestContext ctx = GetGame().GetRestApi().GetContext(uri);
        if (!ctx)
        {
            Print("ERROR: Failed to create REST context", LogLevel.ERROR);
            return false;
        }
		
		ctx.SetHeaders(string.Format("Content-Type,application/json,apikey,%1,Authorization,Bearer %1", m_apiKey));
		ctx.POST(cb, "", args);
		
		return true;
	}
}