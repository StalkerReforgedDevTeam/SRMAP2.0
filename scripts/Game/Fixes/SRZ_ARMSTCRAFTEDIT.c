[BaseContainerProps(), BaseContainerCustomTitleField("m_PrefabCraft")]
modded class ARMST_CRAFT_DATA_INFO
{	

	[Attribute("false", desc: "Should the player be able to disassemble back this crafting recipe", UIWidgets.CheckBox)];
	bool m_bOneSided;
	
	

};


modded class ARMST_CRAFT_UI : ChimeraMenuBase
{
	override void LoadPlayerDisassemblyItems()
	{
	    if (!TextDissList)
	        return;
	
	    TextDissList.ClearItems();
	
	    // Получаем все рецепты разборки: это те же m_AllRecipes, но будем фильтровать
	    ARMST_EDITOR_GLOBAL_SETTINGS craftManager = ARMST_EDITOR_GLOBAL_SETTINGS.GetInstance();
	    if (!craftManager)
	    {
	        Print("[ARMST CRAFT] Craft manager not found for disassembly!", LogLevel.ERROR);
	        return;
	    }
	
	    // Получаем ВСЕ рецепты (они содержат как крафт, так и данные для разборки)
	    ref array<ref ARMST_CRAFT_DATA_INFO> allRecipes = new array<ref ARMST_CRAFT_DATA_INFO>();
	    craftManager.GetAllCrafts(allRecipes);
	
	    // Получаем инвентарь игрока
	    SCR_InventoryStorageManagerComponent storageMan = SCR_InventoryStorageManagerComponent.Cast(m_User.FindComponent(SCR_InventoryStorageManagerComponent));
	    if (!storageMan)
	    {
	        Print("[ARMST CRAFT] Player inventory not found", LogLevel.ERROR);
	        return;
	    }
	
	    array<IEntity> playerItems = new array<IEntity>();
	    storageMan.GetItems(playerItems);
	
	    // Собираем уникальные префабы из инвентаря
	    ref map<ResourceName, int> inventoryPrefabs = new map<ResourceName, int>();
	    foreach (IEntity item : playerItems)
	    {
	        if (!item) continue;
	        ResourceName prefab = item.GetPrefabData().GetPrefabName();
	        if (prefab.IsEmpty()) continue;
	        inventoryPrefabs.Set(prefab, inventoryPrefabs.Get(prefab) + 1);
	    }
	
	    // Теперь находим, какие из этих предметов можно разобрать (есть в рецептах как результат крафта)
	    foreach (ARMST_CRAFT_DATA_INFO recipe : allRecipes)
	    {
	        if (recipe.m_PrefabCraft.IsEmpty()) continue;
			if (recipe.m_bOneSided == true) continue;
	
	        // Проверяем, есть ли такой предмет у игрока
	        if (inventoryPrefabs.Contains(recipe.m_PrefabCraft))
	        {
	            string displayName = Helpers.GetPrefabDisplayName(recipe.m_PrefabCraft);
	            if (displayName.IsEmpty())
	                displayName = "#armst_craft_unknown_item";
	
	            // Добавляем в список разборки
	            int row = TextDissList.AddItem(displayName, recipe, 0, -1);
	            if (row >= 0)
	                Print("[ARMST CRAFT] Добавлен предмет для разборки: " + displayName, LogLevel.NORMAL);
	        }
	    }
	}
}