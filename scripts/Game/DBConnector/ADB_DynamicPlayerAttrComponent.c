[BaseContainerProps()]
class ADB_PlayerAttrComponentClass : ScriptComponentClass {}

class ADB_PlayerAttrComponent : ScriptComponent
{
	[RplProp()]
	protected ref array<string> m_attrsStringKeys;
	
	[RplProp()]
	protected ref array<string> m_attrsStringValues;
	
	void ADB_PlayerAttrComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_attrsStringKeys = new array<string>();
		m_attrsStringValues = new array<string>();
	}
	
	protected override void OnPostInit(IEntity owner)
	{
		
	}
	
	bool DoesAttrHaveValue(string key, string value)
	{
		foreach (int i, string attrKey : m_attrsStringKeys)
		{
			if (key == attrKey)
			{
				if (m_attrsStringValues[i] == value)
					return true;
			}
		}
		
		return false;
	}
	
	void SetStringAttributes(array<string> keys, array<string> values)
	{
		m_attrsStringKeys = keys;
		m_attrsStringValues = values;
		
		Replication.BumpMe();
	}
	
	void SetStringAttributesFromDto(array<ref ADB_DtoPlayerAttrString> dtos)
	{
		m_attrsStringKeys.Clear();
		m_attrsStringValues.Clear();
		
		foreach (ref auto dto : dtos)
		{
			m_attrsStringKeys.Insert(dto.key);
			m_attrsStringValues.Insert(dto.value);
		}
		
		Replication.BumpMe();
	}
}