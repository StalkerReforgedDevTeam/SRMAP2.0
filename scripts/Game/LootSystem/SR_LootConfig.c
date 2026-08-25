[BaseContainerProps(), BaseContainerCustomTitleField("prefab")]
class SR_LootItem 
{

	[Attribute(defvalue: "", desc: "Prefab", params: "et")]
    ResourceName prefab;

    [Attribute(defvalue: "10", desc: "Spawn weight of the item")]
    int weight;

    [Attribute(defvalue: "1", desc: "Maximum stack count")]
    int maxCount;

    [Attribute(defvalue: "100", desc: "Minimum Condition")]
    float conditionMin;
	
	[Attribute(defvalue: "100", desc: "Maximum Condition")]
    float conditionMax; 
	
	int spawnCount;
}

[BaseContainerProps(), BaseContainerCustomTitleField("category")]
class SR_LootCategory
{

    [Attribute(defvalue: "New Category", desc: "Category name")]
    string category;
	
    [Attribute(desc: "Items assigned to this category")]
    ref array<ref SR_LootItem> items;

	int categoryItemWeightTotal;
}

[BaseContainerProps(), BaseContainerCustomTitleField("lootTier")]
class SR_LootTier
{

    [Attribute(defvalue: "Tier name", desc: "Tier name")]
    string lootTier;

    [Attribute(desc: "Categories assigned to this tier")]
    ref array<ref SR_LootCategory> categories;
	


}


[BaseContainerProps(configRoot: true)]
class SR_LootConfig 
{
	[Attribute(desc: "Top-level loot tiers")]
    ref array<ref SR_LootTier> tiers;	

}