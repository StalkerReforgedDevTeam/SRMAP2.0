modded class ARMST_TRADER_UI : ChimeraMenuBase
{
	protected static const int SRZ_TRADER_MAX_CELLS = 3;

	override void AddItemToGrid(ResourceName prefabName, bool isPlayerGrid = false)
	{
	    int columnSize, rowSize;
	    Helpers.GetPrefabGridSize(prefabName, columnSize, rowSize);
	
	    if (columnSize <= 0) columnSize = 1;
	    if (columnSize > 6) columnSize = 6;
	    if (rowSize <= 0) rowSize = 1;
	    if (columnSize > m_iMaxColumns) columnSize = m_iMaxColumns;
	    if (rowSize > m_iMaxRows) rowSize = m_iMaxRows;

	    if (!isPlayerGrid)
	    {
	        while (columnSize > SRZ_TRADER_MAX_CELLS || rowSize > SRZ_TRADER_MAX_CELLS)
	        {
	            columnSize = (columnSize + 1) / 2;
	            rowSize = (rowSize + 1) / 2;
	        }
	    }
	
	    ref array<GridLayoutWidget> targetGrids;
	    ref array<ref ARMST_CellMatrix> targetMatrices;
	    ref array<ref map<Widget, ResourceName>> targetMaps;
	
	    if (isPlayerGrid)
	    {
	        targetGrids = m_aGrids_Player;
	        targetMatrices = m_aMatrices_Player;
	        targetMaps = m_aItemMaps_Player;
	    }
	    else
	    {
			
	        targetGrids = m_aGrids_Trader;
	        targetMatrices = m_aMatrices_Trader;
	        targetMaps = m_aItemMaps_Trader;
	    }
	
	    bool placed = false;
	    for (int gridIndex = 0; gridIndex < targetGrids.Count(); gridIndex++)
	    {
	        GridLayoutWidget grid = targetGrids[gridIndex];
	        ARMST_CellMatrix matrix = targetMatrices[gridIndex];
	        ref map<Widget, ResourceName> itemMap = targetMaps[gridIndex];
	
	        if (!grid || !matrix)
			{
	            continue;
			}
	
			int stock2 = GetTraderStock(prefabName);
			if(stock2 == 0 && !isPlayerGrid)
				continue;
			
	        int outCol, outRow;
	        if (matrix.FindFirstFit(columnSize, rowSize, outCol, outRow))
	        {
	            Widget itemLayout = GetGame().GetWorkspace().CreateWidgets(PreviewItemLayout, grid);
	            if (!itemLayout) 
				{
				continue;
				}
	
	            ItemPreviewWidget preview = ItemPreviewWidget.Cast(itemLayout.FindAnyWidget("item"));
	            ButtonWidget button = ButtonWidget.Cast(itemLayout.FindAnyWidget("ItemButton"));
	            TextWidget text = TextWidget.Cast(itemLayout.FindAnyWidget("stackNumber"));
				if (!isPlayerGrid) // Только для сетки торговца
				{
					if(m_TradersStockSystem)
					{
					    int stock = GetTraderStock(prefabName);
					    if (stock == -1)
					    {
					        text.SetText("∞");
					    }
					    else
					    {
					        // Ограниченный товар — показываем количество
					        text.SetText(stock.ToString());
					    }
						if (stock == 0)
					        text.SetText("0");
						
						if (stock > 99)
					        text.SetText("99");
					}
					else
					{
					   text.SetText("∞");
					}
						
				}
				else
				{
				    // Для инвентаря игрока — показываем количество у игрока
				    int playerCount = GetInventoryCount(prefabName);
				    text.SetText(playerCount.ToString());
				}
	            if (preview && button)
	            {
	                preview.SetVisible(true);
	                button.SetVisible(true);
	
	                ChimeraWorld world = ChimeraWorld.CastFrom(GetGame().GetWorld());
	                if (world && preview)
	                {
	                    ItemPreviewManagerEntity manager = world.GetItemPreviewManager();
	                    if (manager)
	                    {
	                        manager.SetPreviewItemFromPrefab(preview, prefabName, null, false);
	                    }
	                }
	
	                int itemId = m_iNextItemId++;
	                matrix.Place(outCol, outRow, columnSize, rowSize, itemId);
	
	                // Сохраняем связь
	                itemMap.Set(button, prefabName);
	                button.AddHandler(this);
	
	                // Устанавливаем позицию
	                GridSlot.SetColumn(itemLayout, outCol);
	                GridSlot.SetRow(itemLayout, outRow);
	                GridSlot.SetColumnSpan(itemLayout, columnSize);
	                GridSlot.SetRowSpan(itemLayout, rowSize);
	
	                placed = true;
	                break;
	            }
	        }
	    }
	
	    if (!placed)
	    {
	        Print("[ARMST TRADER] Не удалось разместить предмет ни в одной сетке: " + prefabName, LogLevel.ERROR);
	    }
	}
}
