class SR_LootManagerComponentClass : ScriptComponentClass
{
};

class SR_LootManagerComponent : ScriptComponent
{
	[Attribute(desc: "Loot config resource (.conf) containing tiers/categories/items", params: "conf")]
	protected ResourceName masterConfig;

	
	protected ref map<string, ref map<string, ref array<ref SR_LootItem>>> lootMap;

	
	private static SR_LootManagerComponent managerInstance;
	
	static SR_LootManagerComponent GetInstance() 
	{
		return managerInstance;
	}

	override void OnPostInit(IEntity owner)
    {
        super.OnPostInit(owner);
        managerInstance = this;
        SetEventMask(owner, EntityEvent.INIT);
    }
	
	override void OnDelete(IEntity owner)  
	{
		if(managerInstance == this)
			managerInstance = null;
		
		super.OnDelete(owner);
	}
	
	
    override void EOnInit(IEntity owner)
    {
		if (!GetGame().InPlayMode())
			return;
		PrintFormat("[SR LOOT] Initialisation");
		CacheLoot();
		super.EOnInit(owner);
    }
	
	protected void CacheLoot() 
	{
		lootMap = new map<string, ref map<string, ref array<ref SR_LootItem>>>();

		
		Resource conf = BaseContainerTools.LoadContainer(masterConfig);
		if (!conf)
		{
			Print(string.Format("[SR LOOT] failed to load config '%1'", masterConfig), LogLevel.WARNING);
			return;
		}

		SR_LootConfig lootConfig = SR_LootConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(conf.GetResource().ToBaseContainer()));
		if (!lootConfig)
			return;
		
		foreach ( SR_LootTier tier : lootConfig.tiers ) 
		{
			if (!tier || !tier.categories)
				continue;
			
			ref map<string, ref array<ref SR_LootItem>> categoryMap = new map<string, ref array<ref SR_LootItem>>();
			
			foreach ( SR_LootCategory category : tier.categories) 
			{
				if (!category || !category.items)
					continue;
				
				category.categoryItemWeightTotal = 0;
				
				foreach (SR_LootItem item : category.items)
				{
					if (item)
						category.categoryItemWeightTotal += item.weight;
				}
				categoryMap.Insert(category.category, category.items);
			}
			
			lootMap.Insert(tier.lootTier, categoryMap);
			
		}
		Print("[SR LOOT] Loot cached!", LogLevel.NORMAL);
	}
	
	ref array<SR_LootItem> RequestLoot(string containerTier, array<ref SR_CategoryWeight> categories, int slotCount)
	{
		ref array<SR_LootItem> selectedItems = new array<SR_LootItem>();
		Print("[SR LOOT] Loot cached!", LogLevel.NORMAL);
		if (containerTier.IsEmpty() || !categories || categories.IsEmpty() || slotCount <= 0) 
			return selectedItems;
		Print("[SR LOOT] Loot cached!", LogLevel.NORMAL);
		if (!lootMap || !lootMap.Contains(containerTier))
			return selectedItems;
		Print("[SR LOOT] Loot cached!", LogLevel.NORMAL);
		ref map<string, ref array<ref SR_LootItem>> tierCategories = lootMap.Get(containerTier);
	
	
		ref array<SR_CategoryWeight> validCategories = new array<SR_CategoryWeight>();
		int catWeightSum = 0;
	
		foreach (SR_CategoryWeight catObj : categories) 
		{
			if (!catObj) continue;
	
			string catName = catObj.GetCategoryName();
			if (!tierCategories.Contains(catName)) continue;
	
			validCategories.Insert(catObj);
			catWeightSum += catObj.GetWeight();
		}
	
		if (catWeightSum <= 0 || validCategories.IsEmpty())
			return selectedItems;
	
		
		for (int i = 0; i < slotCount; i++)
		{
			
			int catRoll = Math.RandomIntInclusive(1, catWeightSum);
			array<ref SR_LootItem> selectedCat = null;
	
			foreach (SR_CategoryWeight catObj : validCategories)
			{
				PrintFormat("[SR LOOT] Current roll weight %1 vs category weight %2", catRoll, catObj.GetWeight(), LogLevel.NORMAL);
				catRoll -= catObj.GetWeight();
				if (catRoll <= 0)
				{
					PrintFormat("[SR LOOT] Category %1 was chosen", catObj.GetCategoryName(), LogLevel.NORMAL);
					selectedCat = tierCategories.Get(catObj.GetCategoryName());
					break;
				}
			}
	
			if (!selectedCat || selectedCat.IsEmpty())
				continue;
	
			
			int categoryItemWeightTotal = 0;
			foreach (SR_LootItem item : selectedCat)
			{
				if (item)
					categoryItemWeightTotal += item.weight;
			}
	
			if (categoryItemWeightTotal <= 0)
				continue;
	
		
			int itemRoll = Math.RandomIntInclusive(1, categoryItemWeightTotal);
	
			foreach (SR_LootItem itemObj : selectedCat)
			{
				if (!itemObj) continue;
				PrintFormat("[SR LOOT] Current roll weight %1 vs item weight %2", itemRoll, itemObj.weight, LogLevel.NORMAL);
				itemRoll -= itemObj.weight;
				if (itemRoll <= 0)
				{
					itemObj.spawnCount = Math.RandomIntInclusive(1, itemObj.maxCount);
					PrintFormat("[SR LOOT] Item %1 was chosen with quantity %2", itemObj, itemObj.spawnCount, LogLevel.NORMAL);
					selectedItems.Insert(itemObj);
					break;
				}
			}
		}
	
		return selectedItems;
	}
	
	
		
}


