#include "CrossUI_Internal.h"
#include <stdlib.h>
#include <string.h>

static bool Initialized = false;
static cuiWidget **AllWidgets = NULL;
static unsigned AllCount = 0;
static unsigned AllCapacity = 0;

char *cuiInternal_StrDup ( const char *Text )
	{
	const char *Source = ( Text != NULL ) ? Text : "";
	const size_t Length = strlen ( Source );
	char *Copy = ( char * ) malloc ( Length + 1 );
	if ( Copy == NULL )
		return NULL;
	memcpy ( Copy, Source, Length + 1 );
	return Copy;
	}

void cuiInternal_CopyToBuffer ( char *Buffer, size_t BufferSize, const char *Text )
	{
	if ( ( Buffer == NULL ) || ( BufferSize == 0 ) )
		return;
	if ( Text == NULL )
		Text = "";
	strncpy ( Buffer, Text, BufferSize - 1 );
	Buffer[BufferSize - 1] = 0;
	}

static bool EnsureCapacity ( void **Array, unsigned *Capacity, const unsigned Needed, const size_t ElementSize )
	{
	if ( Needed <= *Capacity )
		return true;
	unsigned NewCapacity = ( *Capacity == 0 ) ? 8 : ( *Capacity * 2 );
	while ( NewCapacity < Needed )
		NewCapacity *= 2;
	void *Grown = realloc ( *Array, NewCapacity * ElementSize );
	if ( Grown == NULL )
		return false;
	*Array = Grown;
	*Capacity = NewCapacity;
	return true;
	}

static void RegisterWidget ( cuiWidget *Widget )
	{
	if ( EnsureCapacity ( ( void ** ) &AllWidgets, &AllCapacity, AllCount + 1, sizeof ( cuiWidget * ) ) == false )
		return;
	AllWidgets[AllCount++] = Widget;
	}

static void UnregisterWidget ( cuiWidget *Widget )
	{
	unsigned Index;
	for ( Index = 0; Index < AllCount; ++Index )
		{
		if ( AllWidgets[Index] == Widget )
			{
			AllWidgets[Index] = AllWidgets[AllCount - 1];
			--AllCount;
			return;
			}
		}
	}

static void AddChild ( cuiWidget *Parent, cuiWidget *Child )
	{
	if ( Parent == NULL )
		return;
	if ( EnsureCapacity ( ( void ** ) &Parent->Children, &Parent->ChildCapacity, Parent->ChildCount + 1, sizeof ( cuiWidget * ) ) == false )
		return;
	Parent->Children[Parent->ChildCount++] = Child;
	}

static void RemoveChild ( cuiWidget *Parent, cuiWidget *Child )
	{
	unsigned Index;
	if ( Parent == NULL )
		return;
	for ( Index = 0; Index < Parent->ChildCount; ++Index )
		{
		if ( Parent->Children[Index] == Child )
			{
			memmove ( &Parent->Children[Index], &Parent->Children[Index + 1], ( Parent->ChildCount - Index - 1 ) * sizeof ( cuiWidget * ) );
			--Parent->ChildCount;
			return;
			}
		}
	}

static void FreeItems ( cuiWidget *Widget )
	{
	unsigned Index;
	if ( Widget->Items == NULL )
		return;
	for ( Index = 0; Index < Widget->ItemCount; ++Index )
		free ( Widget->Items[Index] );
	free ( Widget->Items );
	Widget->Items = NULL;
	Widget->ItemCount = 0;
	Widget->ItemCapacity = 0;
	}

static bool PushItem ( cuiWidget *Widget, const char *Text )
	{
	if ( EnsureCapacity ( ( void ** ) &Widget->Items, &Widget->ItemCapacity, Widget->ItemCount + 1, sizeof ( char * ) ) == false )
		return false;
	Widget->Items[Widget->ItemCount] = cuiInternal_StrDup ( Text );
	if ( Widget->Items[Widget->ItemCount] == NULL )
		return false;
	++Widget->ItemCount;
	return true;
	}

cuiWidget *cuiInternal_AsWidget ( GUIHandle Handle )
	{
	unsigned Index;
	cuiWidget *Widget = ( cuiWidget * ) Handle;
	if ( Widget == NULL )
		return NULL;
	for ( Index = 0; Index < AllCount; ++Index )
		{
		if ( AllWidgets[Index] == Widget )
			return ( Widget->Type == cuiType_Dead ) ? NULL : Widget;
		}
	return NULL;
	}

bool cuiInternal_IsAlive ( const cuiWidget *Widget )
	{
	return ( Widget != NULL ) && ( Widget->Type != cuiType_Dead ) && ( Widget->BeingDestroyed == false );
	}

cuiWidget *cuiInternal_FindByNative ( const void *NativeHandle )
	{
	unsigned Index;
	if ( NativeHandle == NULL )
		return NULL;
	for ( Index = 0; Index < AllCount; ++Index )
		{
		cuiWidget *Widget = AllWidgets[Index];
		if ( cuiInternal_IsAlive ( Widget ) == false )
			continue;
		if ( ( Widget->NativeHandle == NativeHandle ) || ( Widget->NativeInner == NativeHandle ) )
			return Widget;
		}
	return NULL;
	}

cuiWidget *cuiInternal_GetFirstWindow ( void )
	{
	unsigned Index;
	for ( Index = 0; Index < AllCount; ++Index )
		{
		if ( cuiInternal_IsAlive ( AllWidgets[Index] ) && ( AllWidgets[Index]->Type == cuiType_Window ) )
			return AllWidgets[Index];
		}
	return NULL;
	}

unsigned cuiInternal_CountWindows ( void )
	{
	unsigned Index;
	unsigned Count = 0;
	for ( Index = 0; Index < AllCount; ++Index )
		{
		if ( cuiInternal_IsAlive ( AllWidgets[Index] ) && ( AllWidgets[Index]->Type == cuiType_Window ) )
			++Count;
		}
	return Count;
	}

void cuiInternal_FireClick ( cuiWidget *Widget )
	{
	if ( cuiInternal_IsAlive ( Widget ) && ( Widget->OnClick != NULL ) )
		Widget->OnClick ( ( GUIHandle ) Widget, Widget->OnClickUserData );
	}

void cuiInternal_FireChange ( cuiWidget *Widget )
	{
	if ( cuiInternal_IsAlive ( Widget ) && ( Widget->OnChange != NULL ) )
		Widget->OnChange ( ( GUIHandle ) Widget, Widget->OnChangeUserData );
	}

void cuiInternal_FireClose ( cuiWidget *Widget )
	{
	if ( cuiInternal_IsAlive ( Widget ) && ( Widget->OnClose != NULL ) )
		Widget->OnClose ( ( GUIHandle ) Widget, Widget->OnCloseUserData );
	}

void cuiInternal_DestroyWidget ( cuiWidget *Widget )
	{
	if ( ( Widget == NULL ) || Widget->BeingDestroyed )
		return;
	Widget->BeingDestroyed = true;

	while ( Widget->ChildCount > 0 )
		cuiInternal_DestroyWidget ( Widget->Children[Widget->ChildCount - 1] );

	cuiBackend_DestroyNative ( Widget );
	RemoveChild ( Widget->Parent, Widget );
	UnregisterWidget ( Widget );
	FreeItems ( Widget );
	free ( Widget->Children );
	free ( Widget->Text );
	Widget->Type = cuiType_Dead;
	free ( Widget );
	}

static GUIHandle CreateWidget ( const cuiWidgetType Type, const GUIHandle ParentHandle, const char *Text,
                                const int X, const int Y, const unsigned Width, const unsigned Height )
	{
	cuiWidget *Parent;
	cuiWidget *Widget;

	if ( Initialized == false )
		return GUIHandle_Invalid;
	if ( Type != cuiType_Window )
		{
		Parent = cuiInternal_AsWidget ( ParentHandle );
		if ( Parent == NULL )
			return GUIHandle_Invalid;
		}
	else
		Parent = cuiInternal_AsWidget ( ParentHandle );

	Widget = ( cuiWidget * ) calloc ( 1, sizeof ( cuiWidget ) );
	if ( Widget == NULL )
		return GUIHandle_Invalid;

	Widget->Type = Type;
	Widget->Parent = Parent;
	Widget->Text = cuiInternal_StrDup ( Text );
	Widget->X = X;
	Widget->Y = Y;
	Widget->Width = Width;
	Widget->Height = Height;
	Widget->Visible = true;
	Widget->Enabled = true;
	Widget->SelectedIndex = -1;
	Widget->SliderMin = 0;
	Widget->SliderMax = 100;
	Widget->SliderValue = 0;
	Widget->ProgressValue = 0;

	if ( Widget->Text == NULL )
		{
		free ( Widget );
		return GUIHandle_Invalid;
		}

	AddChild ( Parent, Widget );
	RegisterWidget ( Widget );

	if ( cuiBackend_Realize ( Widget ) == false )
		{
		cuiInternal_DestroyWidget ( Widget );
		return GUIHandle_Invalid;
		}
	return ( GUIHandle ) Widget;
	}

bool cuiInitialize ( void )
	{
	if ( Initialized )
		return true;
	if ( cuiBackend_Initialize () == false )
		return false;
	Initialized = true;
	return true;
	}

void cuiShutdown ( void )
	{
	if ( Initialized == false )
		return;
	while ( AllCount > 0 )
		cuiInternal_DestroyWidget ( AllWidgets[AllCount - 1] );
	cuiBackend_Shutdown ();
	free ( AllWidgets );
	AllWidgets = NULL;
	AllCount = 0;
	AllCapacity = 0;
	Initialized = false;
	}

const char *cuiGetBackendName ( void )
	{
	return cuiBackend_Name ();
	}

GUIHandle cuiCreateWindow ( const char *NewTitle, const int NewX, const int NewY, const unsigned NewWidth, const unsigned NewHeight )
	{
	return CreateWidget ( cuiType_Window, GUIHandle_Invalid, NewTitle, NewX, NewY, NewWidth, NewHeight );
	}

void cuiDestroyWindow ( const GUIHandle WindowHandle )
	{
	cuiDestroyWidget ( WindowHandle );
	}

void cuiSetWindowTitle ( const GUIHandle WindowHandle, const char *Title )
	{
	cuiSetText ( WindowHandle, Title );
	}

unsigned cuiGetWindowCount ( void )
	{
	return cuiInternal_CountWindows ();
	}

bool cuiUpdate ( const bool Wait )
	{
	if ( Initialized == false )
		return false;
	return cuiBackend_Update ( Wait );
	}

void cuiRun ( void )
	{
	while ( cuiUpdate ( true ) )
		{
		}
	}

GUIHandle cuiCreateLabel ( const GUIHandle Parent, const char *Text, const int X, const int Y, const unsigned Width, const unsigned Height )
	{
	return CreateWidget ( cuiType_Label, Parent, Text, X, Y, Width, Height );
	}

GUIHandle cuiCreateButton ( const GUIHandle Parent, const char *Text, const int X, const int Y, const unsigned Width, const unsigned Height )
	{
	return CreateWidget ( cuiType_Button, Parent, Text, X, Y, Width, Height );
	}

GUIHandle cuiCreateCheckBox ( const GUIHandle Parent, const char *Text, const int X, const int Y, const unsigned Width, const unsigned Height )
	{
	return CreateWidget ( cuiType_CheckBox, Parent, Text, X, Y, Width, Height );
	}

GUIHandle cuiCreateTextBox ( const GUIHandle Parent, const char *Text, const int X, const int Y, const unsigned Width, const unsigned Height )
	{
	return CreateWidget ( cuiType_TextBox, Parent, Text, X, Y, Width, Height );
	}

GUIHandle cuiCreateTextArea ( const GUIHandle Parent, const char *Text, const int X, const int Y, const unsigned Width, const unsigned Height )
	{
	return CreateWidget ( cuiType_TextArea, Parent, Text, X, Y, Width, Height );
	}

GUIHandle cuiCreateComboBox ( const GUIHandle Parent, const int X, const int Y, const unsigned Width, const unsigned Height )
	{
	return CreateWidget ( cuiType_ComboBox, Parent, "", X, Y, Width, Height );
	}

GUIHandle cuiCreateListBox ( const GUIHandle Parent, const int X, const int Y, const unsigned Width, const unsigned Height )
	{
	return CreateWidget ( cuiType_ListBox, Parent, "", X, Y, Width, Height );
	}

GUIHandle cuiCreateTree ( const GUIHandle Parent, const int X, const int Y, const unsigned Width, const unsigned Height )
	{
	return CreateWidget ( cuiType_Tree, Parent, "", X, Y, Width, Height );
	}

GUIHandle cuiCreateSlider ( const GUIHandle Parent, const int MinValue, const int MaxValue, const int Value, const int X, const int Y, const unsigned Width, const unsigned Height )
	{
	GUIHandle Handle = CreateWidget ( cuiType_Slider, Parent, "", X, Y, Width, Height );
	cuiWidget *Widget = cuiInternal_AsWidget ( Handle );
	if ( Widget == NULL )
		return GUIHandle_Invalid;
	Widget->SliderMin = MinValue;
	Widget->SliderMax = MaxValue;
	Widget->SliderValue = Value;
	cuiBackend_SliderSetRange ( Widget );
	cuiBackend_SliderSetValue ( Widget );
	return Handle;
	}

GUIHandle cuiCreateProgressBar ( const GUIHandle Parent, const int X, const int Y, const unsigned Width, const unsigned Height )
	{
	return CreateWidget ( cuiType_ProgressBar, Parent, "", X, Y, Width, Height );
	}

GUIHandle cuiCreateGroupBox ( const GUIHandle Parent, const char *Text, const int X, const int Y, const unsigned Width, const unsigned Height )
	{
	return CreateWidget ( cuiType_GroupBox, Parent, Text, X, Y, Width, Height );
	}

GUIHandle cuiCreatePanel ( const GUIHandle Parent, const int X, const int Y, const unsigned Width, const unsigned Height )
	{
	return CreateWidget ( cuiType_Panel, Parent, "", X, Y, Width, Height );
	}

void cuiDestroyWidget ( const GUIHandle Widget )
	{
	cuiWidget *Resolved = cuiInternal_AsWidget ( Widget );
	if ( Resolved != NULL )
		cuiInternal_DestroyWidget ( Resolved );
	}

void cuiSetText ( const GUIHandle Widget, const char *Text )
	{
	cuiWidget *Resolved = cuiInternal_AsWidget ( Widget );
	char *Copy;
	if ( Resolved == NULL )
		return;
	Copy = cuiInternal_StrDup ( Text );
	if ( Copy == NULL )
		return;
	free ( Resolved->Text );
	Resolved->Text = Copy;
	cuiBackend_SetText ( Resolved, Resolved->Text );
	}

void cuiGetText ( const GUIHandle Widget, char *Buffer, const size_t BufferSize )
	{
	cuiWidget *Resolved = cuiInternal_AsWidget ( Widget );
	if ( Resolved == NULL )
		{
		cuiInternal_CopyToBuffer ( Buffer, BufferSize, "" );
		return;
		}
	cuiBackend_GetText ( Resolved, Buffer, BufferSize );
	}

void cuiSetBounds ( const GUIHandle Widget, const int X, const int Y, const unsigned Width, const unsigned Height )
	{
	cuiWidget *Resolved = cuiInternal_AsWidget ( Widget );
	if ( Resolved == NULL )
		return;
	Resolved->X = X;
	Resolved->Y = Y;
	Resolved->Width = Width;
	Resolved->Height = Height;
	cuiBackend_SetBounds ( Resolved );
	}

void cuiSetVisible ( const GUIHandle Widget, const bool Visible )
	{
	cuiWidget *Resolved = cuiInternal_AsWidget ( Widget );
	if ( Resolved == NULL )
		return;
	Resolved->Visible = Visible;
	cuiBackend_SetVisible ( Resolved );
	}

void cuiSetEnabled ( const GUIHandle Widget, const bool Enabled )
	{
	cuiWidget *Resolved = cuiInternal_AsWidget ( Widget );
	if ( Resolved == NULL )
		return;
	Resolved->Enabled = Enabled;
	cuiBackend_SetEnabled ( Resolved );
	}

bool cuiIsVisible ( const GUIHandle Widget )
	{
	cuiWidget *Resolved = cuiInternal_AsWidget ( Widget );
	return ( Resolved != NULL ) ? Resolved->Visible : false;
	}

bool cuiIsEnabled ( const GUIHandle Widget )
	{
	cuiWidget *Resolved = cuiInternal_AsWidget ( Widget );
	return ( Resolved != NULL ) ? Resolved->Enabled : false;
	}

void cuiSetFocus ( const GUIHandle Widget )
	{
	cuiWidget *Resolved = cuiInternal_AsWidget ( Widget );
	if ( Resolved != NULL )
		cuiBackend_SetFocus ( Resolved );
	}

void cuiSetUserData ( const GUIHandle Widget, void *UserData )
	{
	cuiWidget *Resolved = cuiInternal_AsWidget ( Widget );
	if ( Resolved != NULL )
		Resolved->UserData = UserData;
	}

void *cuiGetUserData ( const GUIHandle Widget )
	{
	cuiWidget *Resolved = cuiInternal_AsWidget ( Widget );
	return ( Resolved != NULL ) ? Resolved->UserData : NULL;
	}

GUIHandle cuiGetParent ( const GUIHandle Widget )
	{
	cuiWidget *Resolved = cuiInternal_AsWidget ( Widget );
	return ( Resolved != NULL ) ? ( GUIHandle ) Resolved->Parent : GUIHandle_Invalid;
	}

void cuiSetClickCallback ( const GUIHandle Widget, const cuiCallback Callback, void *UserData )
	{
	cuiWidget *Resolved = cuiInternal_AsWidget ( Widget );
	if ( Resolved == NULL )
		return;
	Resolved->OnClick = Callback;
	Resolved->OnClickUserData = UserData;
	}

void cuiSetChangeCallback ( const GUIHandle Widget, const cuiCallback Callback, void *UserData )
	{
	cuiWidget *Resolved = cuiInternal_AsWidget ( Widget );
	if ( Resolved == NULL )
		return;
	Resolved->OnChange = Callback;
	Resolved->OnChangeUserData = UserData;
	}

void cuiSetCloseCallback ( const GUIHandle Window, const cuiCallback Callback, void *UserData )
	{
	cuiWidget *Resolved = cuiInternal_AsWidget ( Window );
	if ( Resolved == NULL )
		return;
	Resolved->OnClose = Callback;
	Resolved->OnCloseUserData = UserData;
	}

bool cuiGetChecked ( const GUIHandle Widget )
	{
	cuiWidget *Resolved = cuiInternal_AsWidget ( Widget );
	if ( Resolved == NULL )
		return false;
	Resolved->Checked = cuiBackend_GetChecked ( Resolved );
	return Resolved->Checked;
	}

void cuiSetChecked ( const GUIHandle Widget, const bool Checked )
	{
	cuiWidget *Resolved = cuiInternal_AsWidget ( Widget );
	if ( Resolved == NULL )
		return;
	Resolved->Checked = Checked;
	cuiBackend_SetChecked ( Resolved );
	}

static void ClearListLike ( const GUIHandle Widget )
	{
	cuiWidget *Resolved = cuiInternal_AsWidget ( Widget );
	if ( Resolved == NULL )
		return;
	FreeItems ( Resolved );
	Resolved->SelectedIndex = -1;
	cuiBackend_ClearItems ( Resolved );
	}

static int AddListLikeItem ( const GUIHandle Widget, const char *Text )
	{
	cuiWidget *Resolved = cuiInternal_AsWidget ( Widget );
	if ( Resolved == NULL )
		return -1;
	if ( PushItem ( Resolved, Text ) == false )
		return -1;
	cuiBackend_AddItem ( Resolved, Text );
	return ( int ) Resolved->ItemCount - 1;
	}

void cuiComboClear ( const GUIHandle Widget )
	{
	ClearListLike ( Widget );
	}

int cuiComboAddItem ( const GUIHandle Widget, const char *Text )
	{
	return AddListLikeItem ( Widget, Text );
	}

void cuiComboSetSelected ( const GUIHandle Widget, const int Index )
	{
	cuiWidget *Resolved = cuiInternal_AsWidget ( Widget );
	if ( Resolved == NULL )
		return;
	Resolved->SelectedIndex = Index;
	cuiBackend_SetSelected ( Resolved, Index );
	}

int cuiComboGetSelected ( const GUIHandle Widget )
	{
	cuiWidget *Resolved = cuiInternal_AsWidget ( Widget );
	if ( Resolved == NULL )
		return -1;
	Resolved->SelectedIndex = cuiBackend_GetSelected ( Resolved );
	return Resolved->SelectedIndex;
	}

void cuiListClear ( const GUIHandle Widget )
	{
	ClearListLike ( Widget );
	}

int cuiListAddItem ( const GUIHandle Widget, const char *Text )
	{
	return AddListLikeItem ( Widget, Text );
	}

void cuiListSetSelected ( const GUIHandle Widget, const int Index )
	{
	cuiComboSetSelected ( Widget, Index );
	}

int cuiListGetSelected ( const GUIHandle Widget )
	{
	return cuiComboGetSelected ( Widget );
	}

cuiTreeItem cuiTreeAddItem ( const GUIHandle Tree, const cuiTreeItem Parent, const char *Text )
	{
	cuiWidget *Resolved = cuiInternal_AsWidget ( Tree );
	if ( Resolved == NULL )
		return cuiTreeItem_Root;
	return cuiBackend_TreeAddItem ( Resolved, Parent, Text );
	}

void cuiTreeSetItemText ( const GUIHandle Tree, const cuiTreeItem Item, const char *Text )
	{
	cuiWidget *Resolved = cuiInternal_AsWidget ( Tree );
	if ( Resolved == NULL )
		return;
	cuiBackend_TreeSetItemText ( Resolved, Item, Text );
	}

void cuiTreeGetItemText ( const GUIHandle Tree, const cuiTreeItem Item, char *Buffer, const size_t BufferSize )
	{
	cuiWidget *Resolved = cuiInternal_AsWidget ( Tree );
	if ( Resolved == NULL )
		{
		cuiInternal_CopyToBuffer ( Buffer, BufferSize, "" );
		return;
		}
	cuiBackend_TreeGetItemText ( Resolved, Item, Buffer, BufferSize );
	}

void cuiTreeRemoveItem ( const GUIHandle Tree, const cuiTreeItem Item )
	{
	cuiWidget *Resolved = cuiInternal_AsWidget ( Tree );
	if ( Resolved == NULL )
		return;
	cuiBackend_TreeRemoveItem ( Resolved, Item );
	}

void cuiTreeClear ( const GUIHandle Tree )
	{
	cuiWidget *Resolved = cuiInternal_AsWidget ( Tree );
	if ( Resolved == NULL )
		return;
	cuiBackend_TreeClear ( Resolved );
	}

cuiTreeItem cuiTreeGetSelected ( const GUIHandle Tree )
	{
	cuiWidget *Resolved = cuiInternal_AsWidget ( Tree );
	if ( Resolved == NULL )
		return cuiTreeItem_Root;
	return cuiBackend_TreeGetSelected ( Resolved );
	}

void cuiTreeSetSelected ( const GUIHandle Tree, const cuiTreeItem Item )
	{
	cuiWidget *Resolved = cuiInternal_AsWidget ( Tree );
	if ( Resolved == NULL )
		return;
	cuiBackend_TreeSetSelected ( Resolved, Item );
	}

void cuiTreeExpand ( const GUIHandle Tree, const cuiTreeItem Item, const bool Expand )
	{
	cuiWidget *Resolved = cuiInternal_AsWidget ( Tree );
	if ( Resolved == NULL )
		return;
	cuiBackend_TreeExpand ( Resolved, Item, Expand );
	}

void cuiTreeSetItemData ( const GUIHandle Tree, const cuiTreeItem Item, void *Data )
	{
	cuiWidget *Resolved = cuiInternal_AsWidget ( Tree );
	if ( Resolved == NULL )
		return;
	cuiBackend_TreeSetItemData ( Resolved, Item, Data );
	}

void *cuiTreeGetItemData ( const GUIHandle Tree, const cuiTreeItem Item )
	{
	cuiWidget *Resolved = cuiInternal_AsWidget ( Tree );
	if ( Resolved == NULL )
		return NULL;
	return cuiBackend_TreeGetItemData ( Resolved, Item );
	}

void cuiSliderSetRange ( const GUIHandle Widget, const int MinValue, const int MaxValue )
	{
	cuiWidget *Resolved = cuiInternal_AsWidget ( Widget );
	if ( Resolved == NULL )
		return;
	Resolved->SliderMin = MinValue;
	Resolved->SliderMax = MaxValue;
	cuiBackend_SliderSetRange ( Resolved );
	}

void cuiSliderSetValue ( const GUIHandle Widget, const int Value )
	{
	cuiWidget *Resolved = cuiInternal_AsWidget ( Widget );
	if ( Resolved == NULL )
		return;
	Resolved->SliderValue = Value;
	cuiBackend_SliderSetValue ( Resolved );
	}

int cuiSliderGetValue ( const GUIHandle Widget )
	{
	cuiWidget *Resolved = cuiInternal_AsWidget ( Widget );
	if ( Resolved == NULL )
		return 0;
	Resolved->SliderValue = cuiBackend_SliderGetValue ( Resolved );
	return Resolved->SliderValue;
	}

void cuiProgressSetValue ( const GUIHandle Widget, const int Percent )
	{
	cuiWidget *Resolved = cuiInternal_AsWidget ( Widget );
	int Clamped = Percent;
	if ( Resolved == NULL )
		return;
	if ( Clamped < 0 )
		Clamped = 0;
	if ( Clamped > 100 )
		Clamped = 100;
	Resolved->ProgressValue = Clamped;
	cuiBackend_ProgressSetValue ( Resolved );
	}

int cuiProgressGetValue ( const GUIHandle Widget )
	{
	cuiWidget *Resolved = cuiInternal_AsWidget ( Widget );
	return ( Resolved != NULL ) ? Resolved->ProgressValue : 0;
	}

cuiResult cuiMessageBox ( const GUIHandle Parent, const char *Title, const char *Message, const cuiMessageType Type )
	{
	return cuiBackend_MessageBox ( cuiInternal_AsWidget ( Parent ), Title, Message, Type );
	}

char *cuiOpenFileDialog ( const GUIHandle Parent, const char *Title, const char *Filter )
	{
	return cuiBackend_FileDialog ( cuiInternal_AsWidget ( Parent ), Title, Filter, false, false );
	}

char *cuiSaveFileDialog ( const GUIHandle Parent, const char *Title, const char *Filter )
	{
	return cuiBackend_FileDialog ( cuiInternal_AsWidget ( Parent ), Title, Filter, true, false );
	}

char *cuiSelectFolderDialog ( const GUIHandle Parent, const char *Title )
	{
	return cuiBackend_FileDialog ( cuiInternal_AsWidget ( Parent ), Title, NULL, false, true );
	}
