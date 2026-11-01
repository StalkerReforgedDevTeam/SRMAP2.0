// Readable pages: taking the page in hand opens the ARMST book screen with its text.
// The text lives on the prefab (m_sSRZReadText), so no database row is needed for it.
modded class ARMST_ItemUseComponent : SCR_ConsumableItemComponent
{
	[Attribute("", UIWidgets.EditBox, "SRZ: text shown in the book screen when this item is taken in hand", category: "SRZ Readable")]
	string m_sSRZReadText;

	[Attribute("{986D965E329F91E1}UI/Book/data/notes.edds", UIWidgets.ResourcePickerThumbnail, "SRZ: page image behind the text", category: "SRZ Readable", params: "edds")]
	ResourceName m_sSRZReadImage;

	protected bool m_bSRZReadOpen;

	override void ModeSwitch(EGadgetMode mode, IEntity charOwner)
	{
		super.ModeSwitch(mode, charOwner);
		if (m_sSRZReadText.IsEmpty())
			return;
		if (mode != EGadgetMode.IN_HAND)
			return;
		if (System.IsConsoleApp())
			return;
		IEntity controlled = SCR_PlayerController.GetLocalControlledEntity();
		if (!controlled || controlled != charOwner)
			return;
		GetGame().GetCallqueue().CallLater(SRZ_OpenPage, 350, false);
	}

	protected void SRZ_OpenPage()
	{
		if (!GetGame() || !GetGame().InPlayMode())
			return;
		MenuManager menuManager = GetGame().GetMenuManager();
		if (!menuManager)
			return;
		if (menuManager.IsAnyMenuOpen())
			return;
		MenuBase menu = menuManager.OpenMenu(ChimeraMenuPreset.BookMenus);
		if (!menu)
			return;
		GetGame().GetInputManager().ActivateContext("BookContext");
		ARMST_BOOK_UI book = ARMST_BOOK_UI.Cast(menu);
		if (!book)
			return;
		IEntity user = SCR_PlayerController.GetLocalControlledEntity();
		book.Init(user, m_sSRZReadText, m_sSRZReadImage);
	}

	override void ActivateAction()
	{
		if (!m_sSRZReadText.IsEmpty())
		{
			SRZ_OpenPage();
			return;
		}
		super.ActivateAction();
	}
}
