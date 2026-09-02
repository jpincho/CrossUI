#pragma once
#include "CrossUI.h"

typedef enum
	{
	cuiType_Dead = -1,
	cuiType_Window = 0,
	cuiType_Label,
	cuiType_Button,
	cuiType_CheckBox,
	cuiType_TextBox,
	cuiType_TextArea,
	cuiType_ComboBox,
	cuiType_ListBox,
	cuiType_Tree,
	cuiType_Slider,
	cuiType_ProgressBar,
	cuiType_GroupBox,
	cuiType_Panel
	} cuiWidgetType;

typedef struct cuiWidget cuiWidget;

struct cuiWidget
	{
	cuiWidgetType Type;
	cuiWidget *Parent;
	cuiWidget **Children;
	unsigned ChildCount;
	unsigned ChildCapacity;
	void *NativeHandle;
	void *NativeInner;
	void *UserData;
	char *Text;
	char **Items;
	unsigned ItemCount;
	unsigned ItemCapacity;
	int X, Y;
	unsigned Width, Height;
	int SliderMin, SliderMax, SliderValue;
	int ProgressValue;
	int SelectedIndex;
	bool Visible;
	bool Enabled;
	bool Checked;
	bool BeingDestroyed;
	cuiCallback OnClick;
	void *OnClickUserData;
	cuiCallback OnChange;
	void *OnChangeUserData;
	cuiCallback OnClose;
	void *OnCloseUserData;
	};

bool cuiBackend_Initialize ( void );
void cuiBackend_Shutdown ( void );
const char *cuiBackend_Name ( void );
bool cuiBackend_Realize ( cuiWidget *Widget );
void cuiBackend_DestroyNative ( cuiWidget *Widget );
void cuiBackend_SetText ( cuiWidget *Widget, const char *Text );
void cuiBackend_GetText ( cuiWidget *Widget, char *Buffer, size_t BufferSize );
void cuiBackend_SetBounds ( cuiWidget *Widget );
void cuiBackend_SetVisible ( cuiWidget *Widget );
void cuiBackend_SetEnabled ( cuiWidget *Widget );
void cuiBackend_SetFocus ( cuiWidget *Widget );
void cuiBackend_SetChecked ( cuiWidget *Widget );
bool cuiBackend_GetChecked ( cuiWidget *Widget );
void cuiBackend_ClearItems ( cuiWidget *Widget );
int cuiBackend_AddItem ( cuiWidget *Widget, const char *Text );
void cuiBackend_SetSelected ( cuiWidget *Widget, int Index );
int cuiBackend_GetSelected ( cuiWidget *Widget );
cuiTreeItem cuiBackend_TreeAddItem ( cuiWidget *Widget, cuiTreeItem Parent, const char *Text );
void cuiBackend_TreeSetItemText ( cuiWidget *Widget, cuiTreeItem Item, const char *Text );
void cuiBackend_TreeGetItemText ( cuiWidget *Widget, cuiTreeItem Item, char *Buffer, size_t BufferSize );
void cuiBackend_TreeRemoveItem ( cuiWidget *Widget, cuiTreeItem Item );
void cuiBackend_TreeClear ( cuiWidget *Widget );
cuiTreeItem cuiBackend_TreeGetSelected ( cuiWidget *Widget );
void cuiBackend_TreeSetSelected ( cuiWidget *Widget, cuiTreeItem Item );
void cuiBackend_TreeExpand ( cuiWidget *Widget, cuiTreeItem Item, bool Expand );
void cuiBackend_TreeSetItemData ( cuiWidget *Widget, cuiTreeItem Item, void *Data );
void *cuiBackend_TreeGetItemData ( cuiWidget *Widget, cuiTreeItem Item );
void cuiBackend_SliderSetRange ( cuiWidget *Widget );
void cuiBackend_SliderSetValue ( cuiWidget *Widget );
int cuiBackend_SliderGetValue ( cuiWidget *Widget );
void cuiBackend_ProgressSetValue ( cuiWidget *Widget );
bool cuiBackend_Update ( bool Wait );
cuiResult cuiBackend_MessageBox ( cuiWidget *Parent, const char *Title, const char *Message, cuiMessageType Type );
char *cuiBackend_FileDialog ( cuiWidget *Parent, const char *Title, const char *Filter, bool Save, bool Folder );

char *cuiInternal_StrDup ( const char *Text );
void cuiInternal_CopyToBuffer ( char *Buffer, size_t BufferSize, const char *Text );
void cuiInternal_FireClick ( cuiWidget *Widget );
void cuiInternal_FireChange ( cuiWidget *Widget );
void cuiInternal_FireClose ( cuiWidget *Widget );
cuiWidget *cuiInternal_FindByNative ( const void *NativeHandle );
unsigned cuiInternal_CountWindows ( void );
void cuiInternal_DestroyWidget ( cuiWidget *Widget );
cuiWidget *cuiInternal_AsWidget ( GUIHandle Handle );
cuiWidget *cuiInternal_GetFirstWindow ( void );
bool cuiInternal_IsAlive ( const cuiWidget *Widget );
