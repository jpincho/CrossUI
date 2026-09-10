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

void cuiDestroyWindow ( const cuiHandle WindowHandle )
	{
	cuiWidget *Widget = cuiInternal_HandleToWidget ( WindowHandle );
	if ( Widget == NULL )
		return;
	if ( Widget->Type != cuiType_Window )
		return;
	cuiInternal_DestroyWidgetEntry ( Widget );
	}

cuiWidgetCallbacks cuiGetWidgetCallbacks ( const cuiHandle WidgetHandle )
	{
	cuiWidget *Widget = cuiInternal_HandleToWidget ( WidgetHandle );
	static cuiWidgetCallbacks Empty = {0};
	if ( Widget == NULL )
		return Empty;
	return Widget->Callbacks;
	}

void cuiSetWidgetCallbacks ( const cuiHandle WidgetHandle, cuiWidgetCallbacks Callbacks )
	{
	cuiWidget *Widget = cuiInternal_HandleToWidget ( WidgetHandle );
	if ( Widget == NULL )
		return;
	Widget->Callbacks = Callbacks;
	}

cuiHandle cuiCreateButton ( const cuiHandle ParentHandle, const char *Text, const int X, const int Y, const unsigned Width, const unsigned Height )
	{
	cuiWidget *NewWidget = cuiInternal_CreateWidgetEntry ( cuiType_Button, cuiInternal_HandleToWidget ( ParentHandle ), Text, X, Y, Width, Height );
	if ( NewWidget == NULL )
		return cuiHandle_Invalid;
	if ( cuiBackend_CreateNativeWidget ( NewWidget ) == false )
		{
		free ( NewWidget );
		return cuiHandle_Invalid;
		}
	return cuiInternal_WidgetToHandle ( NewWidget );
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