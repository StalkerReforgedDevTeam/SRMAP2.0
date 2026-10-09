modded class ARMST_CRAFT_UI
{
	protected int m_iSRZ_Tab;
	protected bool m_bSRZ_OnlyRepair;
	protected bool m_bSRZ_Listening;

	//------------------------------------------------------------------------------------------------
	override void Init(IEntity User, bool only_repair = false)
	{
		m_bSRZ_OnlyRepair = only_repair;

		super.Init(User, only_repair);

		if (!m_wRoot)
			return;

		SRZ_AddListeners();
		SRZ_SelectRow(0);
	}

	//------------------------------------------------------------------------------------------------
	override void ShowCraftList()
	{
		super.ShowCraftList();
		m_iSRZ_Tab = 0;
	}

	//------------------------------------------------------------------------------------------------
	override void ShowDissList()
	{
		super.ShowDissList();
		m_iSRZ_Tab = 1;
	}

	//------------------------------------------------------------------------------------------------
	override void ShowRepairList()
	{
		super.ShowRepairList();
		m_iSRZ_Tab = 2;
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		SRZ_RemoveListeners();
		GetGame().GetInputManager().RemoveActionListener("Escape", EActionTrigger.DOWN, CloseNotebook);

		super.OnMenuClose();
	}

	//------------------------------------------------------------------------------------------------
	protected void SRZ_AddListeners()
	{
		if (m_bSRZ_Listening)
			return;

		InputManager input = GetGame().GetInputManager();
		input.AddActionListener("MenuUp", EActionTrigger.DOWN, SRZ_OnUp);
		input.AddActionListener("MenuDown", EActionTrigger.DOWN, SRZ_OnDown);
		input.AddActionListener("MenuSelect", EActionTrigger.DOWN, SRZ_OnConfirm);
		input.AddActionListener("MenuBack", EActionTrigger.DOWN, SRZ_OnBack);
		input.AddActionListener("MenuTabLeft", EActionTrigger.DOWN, SRZ_OnTabLeft);
		input.AddActionListener("MenuTabRight", EActionTrigger.DOWN, SRZ_OnTabRight);
		m_bSRZ_Listening = true;
	}

	//------------------------------------------------------------------------------------------------
	protected void SRZ_RemoveListeners()
	{
		if (!m_bSRZ_Listening)
			return;

		InputManager input = GetGame().GetInputManager();
		input.RemoveActionListener("MenuUp", EActionTrigger.DOWN, SRZ_OnUp);
		input.RemoveActionListener("MenuDown", EActionTrigger.DOWN, SRZ_OnDown);
		input.RemoveActionListener("MenuSelect", EActionTrigger.DOWN, SRZ_OnConfirm);
		input.RemoveActionListener("MenuBack", EActionTrigger.DOWN, SRZ_OnBack);
		input.RemoveActionListener("MenuTabLeft", EActionTrigger.DOWN, SRZ_OnTabLeft);
		input.RemoveActionListener("MenuTabRight", EActionTrigger.DOWN, SRZ_OnTabRight);
		m_bSRZ_Listening = false;
	}

	//------------------------------------------------------------------------------------------------
	protected TextListboxWidget SRZ_GetActiveList()
	{
		if (m_iSRZ_Tab == 1)
			return TextDissList;

		if (m_iSRZ_Tab == 2)
			return TextRepairList;

		return TextCraftList;
	}

	//------------------------------------------------------------------------------------------------
	protected void SRZ_SelectRow(int row)
	{
		TextListboxWidget list = SRZ_GetActiveList();
		if (!list)
			return;

		int count = list.GetNumItems();
		if (count <= 0)
			return;

		row = Math.ClampInt(row, 0, count - 1);

		GetGame().GetWorkspace().SetFocusedWidget(null);
		list.SelectRow(row);
		list.EnsureVisible(row);
		OnItemSelected(list, row, 0, -1, 0);
	}

	//------------------------------------------------------------------------------------------------
	protected void SRZ_MoveSelection(int delta)
	{
		TextListboxWidget list = SRZ_GetActiveList();
		if (!list)
			return;

		int row = list.GetSelectedRow();
		if (row < 0)
			row = 0;
		else
			row += delta;

		SRZ_SelectRow(row);
	}

	//------------------------------------------------------------------------------------------------
	protected void SRZ_SwitchTab(int delta)
	{
		if (m_bSRZ_OnlyRepair)
			return;

		int tab = m_iSRZ_Tab + delta;
		if (tab > 2)
			tab = 0;
		else if (tab < 0)
			tab = 2;

		AudioSystem.PlaySound("{BA40246541CEFACD}Sounds/Items/PDA/ui_menu_click.wav");

		if (tab == 0)
			ShowCraftList();
		else if (tab == 1)
			ShowDissList();
		else
			ShowRepairList();

		SRZ_SelectRow(0);
	}

	//------------------------------------------------------------------------------------------------
	protected void SRZ_OnUp()
	{
		SRZ_MoveSelection(-1);
	}

	//------------------------------------------------------------------------------------------------
	protected void SRZ_OnDown()
	{
		SRZ_MoveSelection(1);
	}

	//------------------------------------------------------------------------------------------------
	protected void SRZ_OnTabLeft()
	{
		SRZ_SwitchTab(-1);
	}

	//------------------------------------------------------------------------------------------------
	protected void SRZ_OnTabRight()
	{
		SRZ_SwitchTab(1);
	}

	//------------------------------------------------------------------------------------------------
	protected void SRZ_OnBack()
	{
		Close();
	}

	//------------------------------------------------------------------------------------------------
	protected void SRZ_OnConfirm()
	{
		TextListboxWidget list = SRZ_GetActiveList();
		if (!list || list.GetSelectedRow() < 0)
			return;

		if (m_iSRZ_Tab == 0)
			HandleCraftAction();
		else if (m_iSRZ_Tab == 1)
			HandleDissolveAction();
		else
			HandleRepairAction();
	}
}