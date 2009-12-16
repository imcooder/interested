#pragma once

#define MENU_ITEM_TEXT_SIZE  1024


typedef struct tagMENUINFO
{
	MENUITEMINFO mii;
	WCHAR text[MENU_ITEM_TEXT_SIZE];
	BOOL bFollowSeparator;
	HMENU hOwnerMenu;
	HWND hDestWnd;
	DWORD fPosition;
} MENUINFO, *LPMENUINFO;

typedef struct tagMENU
{
	HMENU hMenu;
	BOOL bInitMenuSent;
	HMENU hPrevMenu;
} MENU, *LPMENU;
