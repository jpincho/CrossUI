#if defined (_WIN32)
#ifndef WINVER
#define WINVER 0x0601
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif
#ifndef _WIN32_IE
#define _WIN32_IE 0x0800
#endif
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "CrossUI_Internal.h"
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shlobj.h>
#include <objbase.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef _MSC_VER
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

#define CUI_WINDOW_CLASS L"CrossUIWindow"
#define CUI_FIRST_CONTROL_ID 100

typedef struct
	{
	HTREEITEM Handle;
	void *UserData;
	} cuiWinTreeItem;

static HINSTANCE AppInstance = NULL;
static ATOM WindowClassAtom = 0;
static int NextControlId = CUI_FIRST_CONTROL_ID;
static bool QuitPosted = false;

static wchar_t *Widen ( const char *Text )
	{
	int Count;
	wchar_t *Wide;
	if ( Text == NULL )
		Text = "";
	Count = MultiByteToWideChar ( CP_UTF8, 0, Text, -1, NULL, 0 );
	if ( Count <= 0 )
		return NULL;
	Wide = ( wchar_t * ) malloc ( ( size_t ) Count * sizeof ( wchar_t ) );
	if ( Wide == NULL )
		return NULL;
	MultiByteToWideChar ( CP_UTF8, 0, Text, -1, Wide, Count );
	return Wide;
	}

static char *Narrow ( const wchar_t *Text )
	{
	int Count;
	char *Utf8;
	if ( Text == NULL )
		Text = L"";
	Count = WideCharToMultiByte ( CP_UTF8, 0, Text, -1, NULL, 0, NULL, NULL );
	if ( Count <= 0 )
		return NULL;
	Utf8 = ( char * ) malloc ( ( size_t ) Count );
	if ( Utf8 == NULL )
		return NULL;
	WideCharToMultiByte ( CP_UTF8, 0, Text, -1, Utf8, Count, NULL, NULL );
	return Utf8;
	}

static HWND AsHwnd ( void *Handle )
	{
	return ( HWND ) Handle;
	}

static cuiWidget *TopLevelWindow ( cuiWidget *Widget )
	{
	while ( ( Widget != NULL ) && ( Widget->Parent != NULL ) )
		Widget = Widget->Parent;
	return Widget;
	}

static HWND ParentHwnd ( cuiWidget *Widget )
	{
	cuiWidget *Top = TopLevelWindow ( Widget );
	return ( Top != NULL ) ? AsHwnd ( Top->NativeHandle ) : NULL;
	}

static void ClientOffset ( cuiWidget *Widget, int *X, int *Y )
	{
	cuiWidget *Parent;
	*X = Widget->X;
	*Y = Widget->Y;
	Parent = Widget->Parent;
	while ( ( Parent != NULL ) && ( Parent->Type != cuiType_Window ) )
		{
		*X += Parent->X;
		*Y += Parent->Y;
		if ( Parent->Type == cuiType_GroupBox )
			*Y += 16;
		Parent = Parent->Parent;
		}
	}

static HWND InteractiveHwnd ( cuiWidget *Widget )
	{
	if ( Widget->NativeInner != NULL )
		return AsHwnd ( Widget->NativeInner );
	return AsHwnd ( Widget->NativeHandle );
	}

static HWND CreateChild ( cuiWidget *Widget, const wchar_t *ClassName, DWORD Style, DWORD ExStyle )
	{
	int X, Y;
	HWND Parent = ParentHwnd ( Widget );
	wchar_t *Title = Widen ( Widget->Text );
	HWND Handle;
	ClientOffset ( Widget, &X, &Y );
	Handle = CreateWindowExW ( ExStyle, ClassName, Title,
	                           Style | WS_CHILD | ( Widget->Visible ? WS_VISIBLE : 0 ),
	                           X, Y, ( int ) Widget->Width, ( int ) Widget->Height,
	                           Parent, ( HMENU ) ( intptr_t ) NextControlId++,
	                           AppInstance, NULL );
	free ( Title );
	if ( Handle != NULL )
		EnableWindow ( Handle, Widget->Enabled ? TRUE : FALSE );
	return Handle;
	}

static void CopyWindowText ( HWND Handle, char *Buffer, size_t BufferSize )
	{
	int Length = GetWindowTextLengthW ( Handle );
	wchar_t *Wide = ( wchar_t * ) malloc ( ( ( size_t ) Length + 1 ) * sizeof ( wchar_t ) );
	char *Utf8;
	if ( Wide == NULL )
		{
		cuiInternal_CopyToBuffer ( Buffer, BufferSize, "" );
		return;
		}
	GetWindowTextW ( Handle, Wide, Length + 1 );
	Utf8 = Narrow ( Wide );
	free ( Wide );
	cuiInternal_CopyToBuffer ( Buffer, BufferSize, Utf8 );
	free ( Utf8 );
	}

static LRESULT CALLBACK WindowProc ( HWND Hwnd, UINT Message, WPARAM WParam, LPARAM LParam )
	{
	cuiWidget *Self = cuiInternal_FindByNative ( Hwnd );

	switch ( Message )
		{
		case WM_COMMAND:
			{
			HWND Control = ( HWND ) LParam;
			UINT Notify = HIWORD ( WParam );
			cuiWidget *Widget = cuiInternal_FindByNative ( Control );
			if ( Widget == NULL )
				break;
			if ( ( Widget->Type == cuiType_Button ) && ( Notify == BN_CLICKED ) )
				cuiInternal_FireClick ( Widget );
			else if ( ( Widget->Type == cuiType_CheckBox ) && ( Notify == BN_CLICKED ) )
				{
				Widget->Checked = ( SendMessageW ( Control, BM_GETCHECK, 0, 0 ) == BST_CHECKED );
				cuiInternal_FireChange ( Widget );
				}
			else if ( ( Widget->Type == cuiType_TextBox ) && ( Notify == EN_CHANGE ) )
				cuiInternal_FireChange ( Widget );
			else if ( ( Widget->Type == cuiType_TextArea ) && ( Notify == EN_CHANGE ) )
				cuiInternal_FireChange ( Widget );
			else if ( ( Widget->Type == cuiType_ComboBox ) && ( ( Notify == CBN_SELCHANGE ) || ( Notify == CBN_EDITCHANGE ) ) )
				cuiInternal_FireChange ( Widget );
			else if ( ( Widget->Type == cuiType_ListBox ) && ( Notify == LBN_SELCHANGE ) )
				cuiInternal_FireChange ( Widget );
			return 0;
			}
		case WM_HSCROLL:
		case WM_VSCROLL:
			{
			HWND Control = ( HWND ) LParam;
			cuiWidget *Widget = cuiInternal_FindByNative ( Control );
			if ( ( Widget != NULL ) && ( Widget->Type == cuiType_Slider ) )
				{
				Widget->SliderValue = ( int ) SendMessageW ( Control, TBM_GETPOS, 0, 0 );
				cuiInternal_FireChange ( Widget );
				}
			return 0;
			}
		case WM_NOTIFY:
			{
			NMHDR *Header = ( NMHDR * ) LParam;
			cuiWidget *Widget = cuiInternal_FindByNative ( Header->hwndFrom );
			if ( ( Widget != NULL ) && ( Widget->Type == cuiType_Tree ) && ( Header->code == TVN_SELCHANGED ) )
				cuiInternal_FireChange ( Widget );
			return 0;
			}
		case WM_CLOSE:
			if ( Self != NULL )
				{
				cuiInternal_FireClose ( Self );
				cuiInternal_DestroyWidget ( Self );
				}
			return 0;
		case WM_DESTROY:
			if ( cuiInternal_CountWindows () == 0 )
				{
				QuitPosted = true;
				PostQuitMessage ( 0 );
				}
			return 0;
		}
	return DefWindowProcW ( Hwnd, Message, WParam, LParam );
	}

bool cuiBackend_Initialize ( void )
	{
	INITCOMMONCONTROLSEX Controls;
	WNDCLASSEXW Class;
	AppInstance = GetModuleHandleW ( NULL );
	memset ( &Controls, 0, sizeof ( Controls ) );
	Controls.dwSize = sizeof ( Controls );
	Controls.dwICC = ICC_WIN95_CLASSES | ICC_TREEVIEW_CLASSES | ICC_BAR_CLASSES | ICC_PROGRESS_CLASS | ICC_LISTVIEW_CLASSES | ICC_STANDARD_CLASSES;
	InitCommonControlsEx ( &Controls );
	CoInitializeEx ( NULL, COINIT_APARTMENTTHREADED );

	memset ( &Class, 0, sizeof ( Class ) );
	Class.cbSize = sizeof ( Class );
	Class.style = CS_HREDRAW | CS_VREDRAW;
	Class.lpfnWndProc = WindowProc;
	Class.hInstance = AppInstance;
	Class.hCursor = LoadCursorW ( NULL, IDC_ARROW );
	Class.hbrBackground = ( HBRUSH ) ( COLOR_WINDOW + 1 );
	Class.lpszClassName = CUI_WINDOW_CLASS;
	Class.hIcon = LoadIconW ( NULL, IDI_APPLICATION );
	WindowClassAtom = RegisterClassExW ( &Class );
	QuitPosted = false;
	NextControlId = CUI_FIRST_CONTROL_ID;
	return WindowClassAtom != 0;
	}

void cuiBackend_Shutdown ( void )
	{
	if ( WindowClassAtom != 0 )
		{
		UnregisterClassW ( CUI_WINDOW_CLASS, AppInstance );
		WindowClassAtom = 0;
		}
	CoUninitialize ();
	}

const char *cuiBackend_Name ( void )
	{
	return "Win32";
	}

bool cuiBackend_Realize ( cuiWidget *Widget )
	{
	HWND Handle = NULL;

	switch ( Widget->Type )
		{
		case cuiType_Window:
			{
			wchar_t *Title = Widen ( Widget->Text );
			int X = ( Widget->X < 0 ) ? CW_USEDEFAULT : Widget->X;
			int Y = ( Widget->Y < 0 ) ? CW_USEDEFAULT : Widget->Y;
			Handle = CreateWindowExW ( WS_EX_APPWINDOW | WS_EX_CONTROLPARENT, CUI_WINDOW_CLASS, Title,
			                           WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPCHILDREN,
			                           X, Y, ( int ) Widget->Width, ( int ) Widget->Height,
			                           NULL, NULL, AppInstance, NULL );
			free ( Title );
			Widget->NativeHandle = Handle;
			Widget->NativeInner = Handle;
			return Handle != NULL;
			}
		case cuiType_Label:
			Handle = CreateChild ( Widget, L"STATIC", SS_LEFT | SS_CENTERIMAGE, 0 );
			break;
		case cuiType_Button:
			Handle = CreateChild ( Widget, L"BUTTON", WS_TABSTOP | BS_PUSHBUTTON, 0 );
			break;
		case cuiType_CheckBox:
			Handle = CreateChild ( Widget, L"BUTTON", WS_TABSTOP | BS_AUTOCHECKBOX, 0 );
			if ( Handle != NULL )
				SendMessageW ( Handle, BM_SETCHECK, Widget->Checked ? BST_CHECKED : BST_UNCHECKED, 0 );
			break;
		case cuiType_TextBox:
			Handle = CreateChild ( Widget, L"EDIT", WS_TABSTOP | ES_AUTOHSCROLL | ES_LEFT, WS_EX_CLIENTEDGE );
			break;
		case cuiType_TextArea:
			Handle = CreateChild ( Widget, L"EDIT", WS_TABSTOP | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN | ES_LEFT, WS_EX_CLIENTEDGE );
			break;
		case cuiType_ComboBox:
			Handle = CreateChild ( Widget, WC_COMBOBOX, WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWNLIST | CBS_HASSTRINGS, 0 );
			break;
		case cuiType_ListBox:
			Handle = CreateChild ( Widget, WC_LISTBOX, WS_TABSTOP | WS_VSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT, WS_EX_CLIENTEDGE );
			break;
		case cuiType_Tree:
			Handle = CreateChild ( Widget, WC_TREEVIEW, WS_TABSTOP | WS_BORDER | TVS_HASLINES | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_SHOWSELALWAYS, WS_EX_CLIENTEDGE );
			break;
		case cuiType_Slider:
			Handle = CreateChild ( Widget, TRACKBAR_CLASS, WS_TABSTOP | TBS_AUTOTICKS | TBS_HORZ, 0 );
			if ( Handle != NULL )
				{
				SendMessageW ( Handle, TBM_SETRANGE, TRUE, MAKELPARAM ( Widget->SliderMin, Widget->SliderMax ) );
				SendMessageW ( Handle, TBM_SETPOS, TRUE, Widget->SliderValue );
				}
			break;
		case cuiType_ProgressBar:
			Handle = CreateChild ( Widget, PROGRESS_CLASS, PBS_SMOOTH, 0 );
			if ( Handle != NULL )
				{
				SendMessageW ( Handle, PBM_SETRANGE, 0, MAKELPARAM ( 0, 100 ) );
				SendMessageW ( Handle, PBM_SETPOS, ( WPARAM ) Widget->ProgressValue, 0 );
				}
			break;
		case cuiType_GroupBox:
			Handle = CreateChild ( Widget, L"BUTTON", BS_GROUPBOX, 0 );
			break;
		case cuiType_Panel:
			Handle = CreateChild ( Widget, L"STATIC", SS_LEFT, 0 );
			break;
		default:
			return false;
		}

	Widget->NativeHandle = Handle;
	Widget->NativeInner = Handle;
	return Handle != NULL;
	}

void cuiBackend_DestroyNative ( cuiWidget *Widget )
	{
	HWND Handle = AsHwnd ( Widget->NativeHandle );
	if ( Handle == NULL )
		return;
	if ( Widget->Type == cuiType_Tree )
		cuiBackend_TreeClear ( Widget );
	Widget->NativeHandle = NULL;
	Widget->NativeInner = NULL;
	DestroyWindow ( Handle );
	}

void cuiBackend_SetText ( cuiWidget *Widget, const char *Text )
	{
	HWND Handle = InteractiveHwnd ( Widget );
	wchar_t *Wide;
	if ( Handle == NULL )
		return;
	Wide = Widen ( Text );
	SetWindowTextW ( Handle, Wide );
	free ( Wide );
	}

void cuiBackend_GetText ( cuiWidget *Widget, char *Buffer, size_t BufferSize )
	{
	HWND Handle = InteractiveHwnd ( Widget );
	if ( Handle == NULL )
		{
		cuiInternal_CopyToBuffer ( Buffer, BufferSize, Widget->Text );
		return;
		}
	CopyWindowText ( Handle, Buffer, BufferSize );
	}

void cuiBackend_SetBounds ( cuiWidget *Widget )
	{
	HWND Handle = AsHwnd ( Widget->NativeHandle );
	int X, Y;
	unsigned Index;
	if ( Handle == NULL )
		return;
	if ( Widget->Type == cuiType_Window )
		{
		SetWindowPos ( Handle, NULL, Widget->X, Widget->Y, ( int ) Widget->Width, ( int ) Widget->Height, SWP_NOZORDER );
		return;
		}
	ClientOffset ( Widget, &X, &Y );
	SetWindowPos ( Handle, NULL, X, Y, ( int ) Widget->Width, ( int ) Widget->Height, SWP_NOZORDER );
	for ( Index = 0; Index < Widget->ChildCount; ++Index )
		cuiBackend_SetBounds ( Widget->Children[Index] );
	}

void cuiBackend_SetVisible ( cuiWidget *Widget )
	{
	HWND Handle = AsHwnd ( Widget->NativeHandle );
	if ( Handle != NULL )
		ShowWindow ( Handle, Widget->Visible ? SW_SHOW : SW_HIDE );
	}

void cuiBackend_SetEnabled ( cuiWidget *Widget )
	{
	HWND Handle = AsHwnd ( Widget->NativeHandle );
	if ( Handle != NULL )
		EnableWindow ( Handle, Widget->Enabled ? TRUE : FALSE );
	}

void cuiBackend_SetFocus ( cuiWidget *Widget )
	{
	HWND Handle = InteractiveHwnd ( Widget );
	if ( Handle != NULL )
		SetFocus ( Handle );
	}

void cuiBackend_SetChecked ( cuiWidget *Widget )
	{
	HWND Handle = InteractiveHwnd ( Widget );
	if ( ( Handle != NULL ) && ( Widget->Type == cuiType_CheckBox ) )
		SendMessageW ( Handle, BM_SETCHECK, Widget->Checked ? BST_CHECKED : BST_UNCHECKED, 0 );
	}

bool cuiBackend_GetChecked ( cuiWidget *Widget )
	{
	HWND Handle = InteractiveHwnd ( Widget );
	if ( ( Handle != NULL ) && ( Widget->Type == cuiType_CheckBox ) )
		return SendMessageW ( Handle, BM_GETCHECK, 0, 0 ) == BST_CHECKED;
	return Widget->Checked;
	}

void cuiBackend_ClearItems ( cuiWidget *Widget )
	{
	HWND Handle = InteractiveHwnd ( Widget );
	if ( Handle == NULL )
		return;
	if ( Widget->Type == cuiType_ComboBox )
		SendMessageW ( Handle, CB_RESETCONTENT, 0, 0 );
	else if ( Widget->Type == cuiType_ListBox )
		SendMessageW ( Handle, LB_RESETCONTENT, 0, 0 );
	}

int cuiBackend_AddItem ( cuiWidget *Widget, const char *Text )
	{
	HWND Handle = InteractiveHwnd ( Widget );
	wchar_t *Wide;
	UINT Message;
	if ( Handle == NULL )
		return -1;
	Wide = Widen ( Text );
	Message = ( Widget->Type == cuiType_ComboBox ) ? CB_ADDSTRING : LB_ADDSTRING;
	SendMessageW ( Handle, Message, 0, ( LPARAM ) Wide );
	free ( Wide );
	return ( int ) Widget->ItemCount - 1;
	}

void cuiBackend_SetSelected ( cuiWidget *Widget, int Index )
	{
	HWND Handle = InteractiveHwnd ( Widget );
	if ( Handle == NULL )
		return;
	if ( Widget->Type == cuiType_ComboBox )
		SendMessageW ( Handle, CB_SETCURSEL, ( WPARAM ) Index, 0 );
	else if ( Widget->Type == cuiType_ListBox )
		SendMessageW ( Handle, LB_SETCURSEL, ( WPARAM ) Index, 0 );
	}

int cuiBackend_GetSelected ( cuiWidget *Widget )
	{
	HWND Handle = InteractiveHwnd ( Widget );
	LRESULT Result;
	if ( Handle == NULL )
		return -1;
	if ( Widget->Type == cuiType_ComboBox )
		Result = SendMessageW ( Handle, CB_GETCURSEL, 0, 0 );
	else if ( Widget->Type == cuiType_ListBox )
		Result = SendMessageW ( Handle, LB_GETCURSEL, 0, 0 );
	else
		return -1;
	return ( Result == CB_ERR ) ? -1 : ( int ) Result;
	}

cuiTreeItem cuiBackend_TreeAddItem ( cuiWidget *Widget, cuiTreeItem Parent, const char *Text )
	{
	HWND Handle = InteractiveHwnd ( Widget );
	TVINSERTSTRUCTW Insert;
	cuiWinTreeItem *Item;
	wchar_t *Wide;
	if ( Handle == NULL )
		return cuiTreeItem_Root;
	Item = ( cuiWinTreeItem * ) calloc ( 1, sizeof ( cuiWinTreeItem ) );
	if ( Item == NULL )
		return cuiTreeItem_Root;
	Wide = Widen ( Text );
	memset ( &Insert, 0, sizeof ( Insert ) );
	Insert.hParent = ( Parent == cuiTreeItem_Root ) ? TVI_ROOT : ( ( cuiWinTreeItem * ) Parent )->Handle;
	Insert.hInsertAfter = TVI_LAST;
	Insert.item.mask = TVIF_TEXT | TVIF_PARAM;
	Insert.item.pszText = Wide;
	Insert.item.lParam = ( LPARAM ) Item;
	Item->Handle = TreeView_InsertItem ( Handle, &Insert );
	free ( Wide );
	if ( Item->Handle == NULL )
		{
		free ( Item );
		return cuiTreeItem_Root;
		}
	return ( cuiTreeItem ) Item;
	}

void cuiBackend_TreeSetItemText ( cuiWidget *Widget, cuiTreeItem Item, const char *Text )
	{
	HWND Handle = InteractiveHwnd ( Widget );
	TVITEMW TreeItem;
	wchar_t *Wide;
	if ( ( Handle == NULL ) || ( Item == NULL ) )
		return;
	Wide = Widen ( Text );
	memset ( &TreeItem, 0, sizeof ( TreeItem ) );
	TreeItem.mask = TVIF_HANDLE | TVIF_TEXT;
	TreeItem.hItem = ( ( cuiWinTreeItem * ) Item )->Handle;
	TreeItem.pszText = Wide;
	SendMessageW ( Handle, TVM_SETITEMW, 0, ( LPARAM ) &TreeItem );
	free ( Wide );
	}

void cuiBackend_TreeGetItemText ( cuiWidget *Widget, cuiTreeItem Item, char *Buffer, size_t BufferSize )
	{
	HWND Handle = InteractiveHwnd ( Widget );
	TVITEMW TreeItem;
	wchar_t Wide[512];
	char *Utf8;
	if ( ( Handle == NULL ) || ( Item == NULL ) )
		{
		cuiInternal_CopyToBuffer ( Buffer, BufferSize, "" );
		return;
		}
	memset ( &TreeItem, 0, sizeof ( TreeItem ) );
	TreeItem.mask = TVIF_HANDLE | TVIF_TEXT;
	TreeItem.hItem = ( ( cuiWinTreeItem * ) Item )->Handle;
	TreeItem.pszText = Wide;
	TreeItem.cchTextMax = 512;
	SendMessageW ( Handle, TVM_GETITEMW, 0, ( LPARAM ) &TreeItem );
	Utf8 = Narrow ( Wide );
	cuiInternal_CopyToBuffer ( Buffer, BufferSize, Utf8 );
	free ( Utf8 );
	}

static void FreeTreeItemRecursive ( HWND Handle, HTREEITEM Node )
	{
	HTREEITEM Child = TreeView_GetChild ( Handle, Node );
	TVITEMW TreeItem;
	while ( Child != NULL )
		{
		HTREEITEM Next = TreeView_GetNextSibling ( Handle, Child );
		FreeTreeItemRecursive ( Handle, Child );
		Child = Next;
		}
	memset ( &TreeItem, 0, sizeof ( TreeItem ) );
	TreeItem.mask = TVIF_HANDLE | TVIF_PARAM;
	TreeItem.hItem = Node;
	if ( TreeView_GetItem ( Handle, &TreeItem ) )
		free ( ( void * ) TreeItem.lParam );
	}

void cuiBackend_TreeRemoveItem ( cuiWidget *Widget, cuiTreeItem Item )
	{
	HWND Handle = InteractiveHwnd ( Widget );
	HTREEITEM Node;
	if ( ( Handle == NULL ) || ( Item == NULL ) )
		return;
	Node = ( ( cuiWinTreeItem * ) Item )->Handle;
	FreeTreeItemRecursive ( Handle, Node );
	TreeView_DeleteItem ( Handle, Node );
	}

void cuiBackend_TreeClear ( cuiWidget *Widget )
	{
	HWND Handle = InteractiveHwnd ( Widget );
	HTREEITEM Node;
	if ( Handle == NULL )
		return;
	Node = TreeView_GetRoot ( Handle );
	while ( Node != NULL )
		{
		HTREEITEM Next = TreeView_GetNextSibling ( Handle, Node );
		FreeTreeItemRecursive ( Handle, Node );
		Node = Next;
		}
	TreeView_DeleteAllItems ( Handle );
	}

cuiTreeItem cuiBackend_TreeGetSelected ( cuiWidget *Widget )
	{
	HWND Handle = InteractiveHwnd ( Widget );
	HTREEITEM Node;
	TVITEMW TreeItem;
	if ( Handle == NULL )
		return cuiTreeItem_Root;
	Node = TreeView_GetSelection ( Handle );
	if ( Node == NULL )
		return cuiTreeItem_Root;
	memset ( &TreeItem, 0, sizeof ( TreeItem ) );
	TreeItem.mask = TVIF_HANDLE | TVIF_PARAM;
	TreeItem.hItem = Node;
	if ( TreeView_GetItem ( Handle, &TreeItem ) == FALSE )
		return cuiTreeItem_Root;
	return ( cuiTreeItem ) TreeItem.lParam;
	}

void cuiBackend_TreeSetSelected ( cuiWidget *Widget, cuiTreeItem Item )
	{
	HWND Handle = InteractiveHwnd ( Widget );
	if ( Handle == NULL )
		return;
	TreeView_SelectItem ( Handle, ( Item == NULL ) ? NULL : ( ( cuiWinTreeItem * ) Item )->Handle );
	}

void cuiBackend_TreeExpand ( cuiWidget *Widget, cuiTreeItem Item, bool Expand )
	{
	HWND Handle = InteractiveHwnd ( Widget );
	if ( ( Handle == NULL ) || ( Item == NULL ) )
		return;
	TreeView_Expand ( Handle, ( ( cuiWinTreeItem * ) Item )->Handle, Expand ? TVE_EXPAND : TVE_COLLAPSE );
	}

void cuiBackend_TreeSetItemData ( cuiWidget *Widget, cuiTreeItem Item, void *Data )
	{
	( void ) Widget;
	if ( Item != NULL )
		( ( cuiWinTreeItem * ) Item )->UserData = Data;
	}

void *cuiBackend_TreeGetItemData ( cuiWidget *Widget, cuiTreeItem Item )
	{
	( void ) Widget;
	return ( Item != NULL ) ? ( ( cuiWinTreeItem * ) Item )->UserData : NULL;
	}

void cuiBackend_SliderSetRange ( cuiWidget *Widget )
	{
	HWND Handle = InteractiveHwnd ( Widget );
	if ( Handle != NULL )
		SendMessageW ( Handle, TBM_SETRANGE, TRUE, MAKELPARAM ( Widget->SliderMin, Widget->SliderMax ) );
	}

void cuiBackend_SliderSetValue ( cuiWidget *Widget )
	{
	HWND Handle = InteractiveHwnd ( Widget );
	if ( Handle != NULL )
		SendMessageW ( Handle, TBM_SETPOS, TRUE, Widget->SliderValue );
	}

int cuiBackend_SliderGetValue ( cuiWidget *Widget )
	{
	HWND Handle = InteractiveHwnd ( Widget );
	if ( Handle != NULL )
		return ( int ) SendMessageW ( Handle, TBM_GETPOS, 0, 0 );
	return Widget->SliderValue;
	}

void cuiBackend_ProgressSetValue ( cuiWidget *Widget )
	{
	HWND Handle = InteractiveHwnd ( Widget );
	if ( Handle != NULL )
		SendMessageW ( Handle, PBM_SETPOS, ( WPARAM ) Widget->ProgressValue, 0 );
	}

bool cuiBackend_Update ( bool Wait )
	{
	MSG Message;
	BOOL Result;
	if ( ( cuiInternal_CountWindows () == 0 ) || QuitPosted )
		return false;
	if ( Wait )
		{
		Result = GetMessageW ( &Message, NULL, 0, 0 );
		if ( Result <= 0 )
			{
			QuitPosted = true;
			return false;
			}
		if ( IsDialogMessageW ( ( Message.hwnd != NULL ) ? GetAncestor ( Message.hwnd, GA_ROOT ) : NULL, &Message ) == FALSE )
			{
			TranslateMessage ( &Message );
			DispatchMessageW ( &Message );
			}
		}
	else
		{
		while ( PeekMessageW ( &Message, NULL, 0, 0, PM_REMOVE ) )
			{
			if ( Message.message == WM_QUIT )
				{
				QuitPosted = true;
				return false;
				}
			if ( IsDialogMessageW ( ( Message.hwnd != NULL ) ? GetAncestor ( Message.hwnd, GA_ROOT ) : NULL, &Message ) == FALSE )
				{
				TranslateMessage ( &Message );
				DispatchMessageW ( &Message );
				}
			}
		}
	return ( QuitPosted == false ) && ( cuiInternal_CountWindows () > 0 );
	}

cuiResult cuiBackend_MessageBox ( cuiWidget *Parent, const char *Title, const char *Message, cuiMessageType Type )
	{
	HWND Owner = ( Parent != NULL ) ? AsHwnd ( TopLevelWindow ( Parent )->NativeHandle ) : NULL;
	wchar_t *WideTitle = Widen ( ( Title != NULL ) ? Title : "" );
	wchar_t *WideMessage = Widen ( ( Message != NULL ) ? Message : "" );
	UINT Flags = MB_OK;
	int Result;
	cuiResult Mapped = cuiResult_OK;

	switch ( Type )
		{
		case cuiMessage_Warning:
			Flags |= MB_ICONWARNING;
			break;
		case cuiMessage_Error:
			Flags |= MB_ICONERROR;
			break;
		case cuiMessage_Question:
			Flags = MB_YESNO | MB_ICONQUESTION;
			break;
		default:
			Flags |= MB_ICONINFORMATION;
			break;
		}

	Result = MessageBoxW ( Owner, WideMessage, WideTitle, Flags );
	free ( WideTitle );
	free ( WideMessage );
	if ( Result == IDYES )
		Mapped = cuiResult_Yes;
	else if ( Result == IDNO )
		Mapped = cuiResult_No;
	else if ( Result == IDCANCEL )
		Mapped = cuiResult_Cancel;
	return Mapped;
	}

static void ToWin32Filter ( const char *Filter, wchar_t *Output, const size_t OutputChars )
	{
	size_t Index;
	size_t Out = 0;
	wchar_t *Wide;
	if ( ( Filter == NULL ) || ( Filter[0] == 0 ) )
		{
		Output[0] = 0;
		Output[1] = 0;
		return;
		}
	Wide = Widen ( Filter );
	for ( Index = 0; Wide[Index] != 0 && Out + 2 < OutputChars; ++Index )
		{
		Output[Out++] = ( Wide[Index] == L'|' ) ? 0 : Wide[Index];
		}
	Output[Out++] = 0;
	Output[Out] = 0;
	free ( Wide );
	}

static char *OpenSaveDialog ( cuiWidget *Parent, const char *Title, const char *Filter, bool Save )
	{
	OPENFILENAMEW OpenFile;
	wchar_t FileName[MAX_PATH];
	wchar_t FilterBuffer[1024];
	wchar_t *WideTitle = Widen ( ( Title != NULL ) ? Title : "" );
	HWND Owner = ( Parent != NULL ) ? AsHwnd ( TopLevelWindow ( Parent )->NativeHandle ) : NULL;
	BOOL Ok;
	char *Result = NULL;

	FileName[0] = 0;
	ToWin32Filter ( Filter, FilterBuffer, 1024 );
	memset ( &OpenFile, 0, sizeof ( OpenFile ) );
	OpenFile.lStructSize = sizeof ( OpenFile );
	OpenFile.hwndOwner = Owner;
	OpenFile.lpstrFilter = ( FilterBuffer[0] != 0 ) ? FilterBuffer : NULL;
	OpenFile.lpstrFile = FileName;
	OpenFile.nMaxFile = MAX_PATH;
	OpenFile.lpstrTitle = WideTitle;
	OpenFile.Flags = OFN_EXPLORER | OFN_NOCHANGEDIR | OFN_HIDEREADONLY;
	if ( Save )
		{
		OpenFile.Flags |= OFN_OVERWRITEPROMPT;
		Ok = GetSaveFileNameW ( &OpenFile );
		}
	else
		{
		OpenFile.Flags |= OFN_FILEMUSTEXIST;
		Ok = GetOpenFileNameW ( &OpenFile );
		}
	if ( Ok )
		Result = Narrow ( FileName );
	free ( WideTitle );
	return Result;
	}

static int CALLBACK BrowseCallback ( HWND Hwnd, UINT Message, LPARAM LParam, LPARAM Data )
	{
	( void ) LParam;
	if ( ( Message == BFFM_INITIALIZED ) && ( Data != 0 ) )
		SendMessageW ( Hwnd, BFFM_SETSELECTIONW, TRUE, Data );
	return 0;
	}

static char *FolderDialog ( cuiWidget *Parent, const char *Title )
	{
	BROWSEINFOW Browse;
	wchar_t DisplayName[MAX_PATH];
	wchar_t *WideTitle = Widen ( ( Title != NULL ) ? Title : "" );
	PIDLIST_ABSOLUTE List;
	char *Result = NULL;

	memset ( &Browse, 0, sizeof ( Browse ) );
	Browse.hwndOwner = ( Parent != NULL ) ? AsHwnd ( TopLevelWindow ( Parent )->NativeHandle ) : NULL;
	Browse.pszDisplayName = DisplayName;
	Browse.lpszTitle = WideTitle;
	Browse.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
	Browse.lpfn = BrowseCallback;
	List = SHBrowseForFolderW ( &Browse );
	if ( List != NULL )
		{
		wchar_t Path[MAX_PATH];
		if ( SHGetPathFromIDListW ( List, Path ) )
			Result = Narrow ( Path );
		CoTaskMemFree ( List );
		}
	free ( WideTitle );
	return Result;
	}

char *cuiBackend_FileDialog ( cuiWidget *Parent, const char *Title, const char *Filter, bool Save, bool Folder )
	{
	if ( Folder )
		return FolderDialog ( Parent, Title );
	return OpenSaveDialog ( Parent, Title, Filter, Save );
	}

#endif
