[BaseContainerProps()]
class ADB_AttributeListComponentClass : ScriptComponentClass {}

class ADB_AttributeListComponent : ScriptComponent
{
	[Attribute()]
	ref array<ref ADB_AttributeString> stringAttributes;
}