#include "CrossUI_Internal.h"
#include "CrossUI_BackendFunctions.h"
#include "CrossUI.h"
#include <stdlib.h>

static bool Initialized = false;

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
	cuiInternal_DestroyAllWidgets();
	cuiBackend_Shutdown ();
	Initialized = false;
	}

const char *cuiGetBackendName ( void )
	{
	return cuiBackend_Name ();
	}

bool cuiUpdate ( const bool Wait )
	{
	return cuiBackend_Update ( Wait );
	}

cuiHandle cuiCreateWidget ( const cuiWidgetType Type, const cuiHandle ParentHandle, const char *Text, const int X, const int Y, const unsigned Width, const unsigned Height )
	{
	cuiWidget *NewWidget = cuiInternal_CreateWidgetEntry ( Type, cuiInternal_HandleToWidget ( ParentHandle ), Text, X, Y, Width, Height );
	if ( NewWidget == NULL )
		return cuiHandle_Invalid;
	if ( cuiBackend_CreateNativeWidget ( NewWidget ) == false )
		{
		free ( NewWidget );
		return cuiHandle_Invalid;
		}
	return cuiInternal_WidgetToHandle ( NewWidget );
	}

void cuiDestroyWidget ( const cuiHandle WidgetHandle )
	{
	cuiWidget *Widget = cuiInternal_HandleToWidget ( WidgetHandle );
	if ( Widget == NULL )
		return;
	cuiInternal_DestroyWidgetEntry ( Widget );
	}

void cuiSetWidgetCallbacks ( const cuiHandle WidgetHandle, cuiWidgetCallbacks Callbacks )
	{
	cuiWidget *Widget = cuiInternal_HandleToWidget ( WidgetHandle );
	if ( Widget == NULL )
		return;
	Widget->Callbacks = Callbacks;
	}

cuiWidgetCallbacks cuiGetWidgetCallbacks ( const cuiHandle WidgetHandle )
	{
	cuiWidget *Widget = cuiInternal_HandleToWidget ( WidgetHandle );
	static cuiWidgetCallbacks Empty = {0};
	if ( Widget == NULL )
		return Empty;
	return Widget->Callbacks;
	}

void cuiSetWidgetText ( const cuiHandle WidgetHandle, const char *Text )
	{
	cuiWidget *Widget = cuiInternal_HandleToWidget ( WidgetHandle );
	cuiInternal_SetWidgetText ( Widget, Text );
	}

void cuiGetWidgetText ( const cuiHandle WidgetHandle, char *Buffer, const unsigned BufferSize )
	{
	cuiWidget *Widget = cuiInternal_HandleToWidget ( WidgetHandle );
	cuiInternal_GetText ( Widget, Buffer, BufferSize );
	}

#if 0

void cuiSetWindowTitle ( const cuiHandle WindowHandle, const char *Title )
	{
	}

unsigned cuiGetWindowCount ( void )
	{
	return cuiInternal_CountWindows ();
	}

bool cuiUpdate ( void )
	{
	if ( Initialized == false )
		return false;
	return cuiBackend_Update ();
	}
#endif


cuiHandle cuiCreateWindow ( const char *Title, const int X, const int Y, const unsigned Width, const unsigned Height )
	{
	cuiWidget *NewWidget = cuiInternal_CreateWidgetEntry ( cuiType_Window, NULL, Title, X, Y, Width, Height );
	if ( NewWidget == NULL )
		return cuiHandle_Invalid;
	if ( cuiBackend_CreateNativeWidget ( NewWidget ) == false )
		{
		free ( NewWidget );
		return cuiHandle_Invalid;
		}
	return cuiInternal_WidgetToHandle ( NewWidget );
	}

cuiHandle cuiCreateButton ( const cuiHandle ParentHandle, const char *Text, const int X, const int Y, const unsigned Width, const unsigned Height )
	{
	return cuiCreateWidget ( cuiType_Button, ParentHandle, Text, X, Y, Width, Height );
	}

cuiHandle cuiCreateLabel ( const cuiHandle ParentHandle, const char *Text, const int X, const int Y, const unsigned Width, const unsigned Height )
	{
	return cuiCreateWidget ( cuiType_Label, ParentHandle, Text, X, Y, Width, Height );
	}

cuiHandle cuiCreateCheckbox ( const cuiHandle ParentHandle, const char *Text, const int X, const int Y, const unsigned Width, const unsigned Height )
	{
	return cuiCreateWidget ( cuiType_CheckBox, ParentHandle, Text, X, Y, Width, Height );
	}

bool cuiGetChecked ( const cuiHandle WidgetHandle )
	{
	cuiWidget *Widget = cuiInternal_HandleToWidget ( WidgetHandle );
	if ( Widget == NULL )
		return false;
	Widget->CheckBoxData.Checked = cuiBackend_CheckBox_GetState ( Widget );
	return Widget->CheckBoxData.Checked;
	}

void cuiSetChecked ( const cuiHandle WidgetHandle, const bool Checked )
	{
	cuiWidget *Widget = cuiInternal_HandleToWidget ( WidgetHandle );
	if ( Widget == NULL )
		return;
	Widget->CheckBoxData.Checked = Checked;
	cuiBackend_CheckBox_SetState ( Widget, Checked );
	}

cuiHandle cuiCreateTextBox ( const cuiHandle ParentHandle, const char *Text, const int X, const int Y, const unsigned Width, const unsigned Height )
	{
	return cuiCreateWidget ( cuiType_TextBox, ParentHandle, Text, X, Y, Width, Height );
	}

cuiHandle cuiCreateTextArea ( const cuiHandle ParentHandle, const char *Text, const int X, const int Y, const unsigned Width, const unsigned Height )
	{
	return cuiCreateWidget ( cuiType_TextArea, ParentHandle, Text, X, Y, Width, Height );
	}

cuiHandle cuiCreateComboBox ( const cuiHandle ParentHandle, const char *Text, const int X, const int Y, const unsigned Width, const unsigned Height )
	{
	return cuiCreateWidget ( cuiType_ComboBox, ParentHandle, Text, X, Y, Width, Height );
	}

cuiHandle cuiCreateListBox ( const cuiHandle ParentHandle, const char *Text, const int X, const int Y, const unsigned Width, const unsigned Height )
	{
	return cuiCreateWidget ( cuiType_ListBox, ParentHandle, Text, X, Y, Width, Height );
	}

cuiHandle cuiCreateTree ( const cuiHandle ParentHandle, const int X, const int Y, const unsigned Width, const unsigned Height )
	{
	return cuiCreateWidget ( cuiType_Tree, ParentHandle, NULL, X, Y, Width, Height );
	}

cuiHandle cuiCreateGroupBox ( const cuiHandle ParentHandle, const char *Text, const int X, const int Y, const unsigned Width, const unsigned Height )
	{
	return cuiCreateWidget ( cuiType_GroupBox, ParentHandle, Text, X, Y, Width, Height );
	}

cuiHandle cuiCreatePanel ( const cuiHandle ParentHandle, const char *Text, const int X, const int Y, const unsigned Width, const unsigned Height )
	{
	return cuiCreateWidget ( cuiType_Panel, ParentHandle, Text, X, Y, Width, Height );
	}

