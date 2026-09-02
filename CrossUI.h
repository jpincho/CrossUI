#pragma once
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void *GUIHandle;
#define GUIHandle_Invalid NULL

typedef void *cuiTreeItem;
#define cuiTreeItem_Root NULL

typedef void ( *cuiCallback ) ( GUIHandle Widget, void *UserData );

typedef enum
	{
	cuiMessage_Info = 0,
	cuiMessage_Warning,
	cuiMessage_Error,
	cuiMessage_Question
	} cuiMessageType;

typedef enum
	{
	cuiResult_None = 0,
	cuiResult_OK,
	cuiResult_Cancel,
	cuiResult_Yes,
	cuiResult_No
	} cuiResult;

bool cuiInitialize ( void );
void cuiShutdown ( void );
const char *cuiGetBackendName ( void );

GUIHandle cuiCreateWindow ( const char *NewTitle, const int NewX, const int NewY, const unsigned NewWidth, const unsigned NewHeight );
void cuiDestroyWindow ( const GUIHandle WindowHandle );
void cuiSetWindowTitle ( const GUIHandle WindowHandle, const char *Title );
unsigned cuiGetWindowCount ( void );
bool cuiUpdate ( const bool Wait );
void cuiRun ( void );

GUIHandle cuiCreateLabel ( const GUIHandle Parent, const char *Text, const int X, const int Y, const unsigned Width, const unsigned Height );
GUIHandle cuiCreateButton ( const GUIHandle Parent, const char *Text, const int X, const int Y, const unsigned Width, const unsigned Height );
GUIHandle cuiCreateCheckBox ( const GUIHandle Parent, const char *Text, const int X, const int Y, const unsigned Width, const unsigned Height );
GUIHandle cuiCreateTextBox ( const GUIHandle Parent, const char *Text, const int X, const int Y, const unsigned Width, const unsigned Height );
GUIHandle cuiCreateTextArea ( const GUIHandle Parent, const char *Text, const int X, const int Y, const unsigned Width, const unsigned Height );
GUIHandle cuiCreateComboBox ( const GUIHandle Parent, const int X, const int Y, const unsigned Width, const unsigned Height );
GUIHandle cuiCreateListBox ( const GUIHandle Parent, const int X, const int Y, const unsigned Width, const unsigned Height );
GUIHandle cuiCreateTree ( const GUIHandle Parent, const int X, const int Y, const unsigned Width, const unsigned Height );
GUIHandle cuiCreateSlider ( const GUIHandle Parent, const int MinValue, const int MaxValue, const int Value, const int X, const int Y, const unsigned Width, const unsigned Height );
GUIHandle cuiCreateProgressBar ( const GUIHandle Parent, const int X, const int Y, const unsigned Width, const unsigned Height );
GUIHandle cuiCreateGroupBox ( const GUIHandle Parent, const char *Text, const int X, const int Y, const unsigned Width, const unsigned Height );
GUIHandle cuiCreatePanel ( const GUIHandle Parent, const int X, const int Y, const unsigned Width, const unsigned Height );

void cuiDestroyWidget ( const GUIHandle Widget );
void cuiSetText ( const GUIHandle Widget, const char *Text );
void cuiGetText ( const GUIHandle Widget, char *Buffer, const size_t BufferSize );
void cuiSetBounds ( const GUIHandle Widget, const int X, const int Y, const unsigned Width, const unsigned Height );
void cuiSetVisible ( const GUIHandle Widget, const bool Visible );
void cuiSetEnabled ( const GUIHandle Widget, const bool Enabled );
bool cuiIsVisible ( const GUIHandle Widget );
bool cuiIsEnabled ( const GUIHandle Widget );
void cuiSetFocus ( const GUIHandle Widget );
void cuiSetUserData ( const GUIHandle Widget, void *UserData );
void *cuiGetUserData ( const GUIHandle Widget );
GUIHandle cuiGetParent ( const GUIHandle Widget );

void cuiSetClickCallback ( const GUIHandle Widget, const cuiCallback Callback, void *UserData );
void cuiSetChangeCallback ( const GUIHandle Widget, const cuiCallback Callback, void *UserData );
void cuiSetCloseCallback ( const GUIHandle Window, const cuiCallback Callback, void *UserData );

bool cuiGetChecked ( const GUIHandle Widget );
void cuiSetChecked ( const GUIHandle Widget, const bool Checked );

void cuiComboClear ( const GUIHandle Widget );
int cuiComboAddItem ( const GUIHandle Widget, const char *Text );
void cuiComboSetSelected ( const GUIHandle Widget, const int Index );
int cuiComboGetSelected ( const GUIHandle Widget );

void cuiListClear ( const GUIHandle Widget );
int cuiListAddItem ( const GUIHandle Widget, const char *Text );
void cuiListSetSelected ( const GUIHandle Widget, const int Index );
int cuiListGetSelected ( const GUIHandle Widget );

cuiTreeItem cuiTreeAddItem ( const GUIHandle Tree, const cuiTreeItem Parent, const char *Text );
void cuiTreeSetItemText ( const GUIHandle Tree, const cuiTreeItem Item, const char *Text );
void cuiTreeGetItemText ( const GUIHandle Tree, const cuiTreeItem Item, char *Buffer, const size_t BufferSize );
void cuiTreeRemoveItem ( const GUIHandle Tree, const cuiTreeItem Item );
void cuiTreeClear ( const GUIHandle Tree );
cuiTreeItem cuiTreeGetSelected ( const GUIHandle Tree );
void cuiTreeSetSelected ( const GUIHandle Tree, const cuiTreeItem Item );
void cuiTreeExpand ( const GUIHandle Tree, const cuiTreeItem Item, const bool Expand );
void cuiTreeSetItemData ( const GUIHandle Tree, const cuiTreeItem Item, void *Data );
void *cuiTreeGetItemData ( const GUIHandle Tree, const cuiTreeItem Item );

void cuiSliderSetRange ( const GUIHandle Widget, const int MinValue, const int MaxValue );
void cuiSliderSetValue ( const GUIHandle Widget, const int Value );
int cuiSliderGetValue ( const GUIHandle Widget );

void cuiProgressSetValue ( const GUIHandle Widget, const int Percent );
int cuiProgressGetValue ( const GUIHandle Widget );

cuiResult cuiMessageBox ( const GUIHandle Parent, const char *Title, const char *Message, const cuiMessageType Type );
/* Returned path must be free()'d by the caller. NULL if the dialog is cancelled.
   Filter format: "Shader Files (*.glsl)|*.glsl|All Files (*.*)|*.*" */
char *cuiOpenFileDialog ( const GUIHandle Parent, const char *Title, const char *Filter );
char *cuiSaveFileDialog ( const GUIHandle Parent, const char *Title, const char *Filter );
char *cuiSelectFolderDialog ( const GUIHandle Parent, const char *Title );

#ifdef __cplusplus
}
#endif
