#include "CrossUI_Internal.h"
#include "CrossUI.h"
#include <stdlib.h>

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

void cuiDestroyWidget ( cuiWidget *Widget )
	{
	if ( Widget == NULL )
		return;
	cuiInternal_DestroyWidgetEntry ( Widget );
	}

void cuiSetWidgetCallbacks ( cuiWidget *Widget, cuiWidgetCallbacks Callbacks )
	{
	if ( Widget == NULL )
		return;
	Widget->Callbacks = Callbacks;
	}

cuiWidgetCallbacks cuiGetWidgetCallbacks ( const cuiWidget *Widget )
	{
	static cuiWidgetCallbacks Empty = {0};
	if ( Widget == NULL )
		return Empty;
	return Widget->Callbacks;
	}

cuiWidget *cuiGetParent ( const cuiWidget *Widget )
	{
	return ( Widget != NULL ) ? Widget->Parent : cuiWidget_Invalid;
	}

