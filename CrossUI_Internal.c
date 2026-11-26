#include "CrossUI_Internal.h"
#include "CrossUI_InternalBackendFunctions.h"
#include <Platform/Platform.h>
#include <Platform/ArrayUtils.h>
#include <Platform/Logger.h>
#include <Platform/PointerList.h>

#if defined (PLATFORM_COMPILER_GNU)
#include <stdlib.h>
#include <string.h>
#endif

bool Initialized = false;
PointerList WidgetList = {0};

static bool cuiInternal_AddChildToParent ( cuiWidget *Parent, cuiWidget *Child )
	{
	if ( Parent == NULL )
		return true;
	return PointerArray_AddAtEnd ( &Parent->Children, Child );
	}

static void cuiInternal_RemoveChildFromParent ( cuiWidget *Parent, cuiWidget *Child )
	{
	if ( Parent == NULL )
		return;

	long Index = PointerArray_Find ( &Parent->Children, Child );
	if ( Index != -1 )
		PointerArray_RemoveAndShift ( &Parent->Children, ( unsigned long ) Index );
	}

static bool cuiInternal_AddWidgetToList ( cuiWidget *Widget )
	{
	if ( Widget == NULL )
		return false;

	return PointerList_AddAtEnd ( &WidgetList, Widget ) != NULL;
	}

static void cuiInternal_RemoveWidgetFromList ( cuiWidget *Widget )
	{
	if ( Widget == NULL )
		return;

	PointerListNode *Node = PointerList_Find ( &WidgetList, NULL, Widget );
	if ( Node != NULL )
		PointerList_DestroyNode ( &WidgetList, Node );
	}

cuiWidget *cuiInternal_CreateWidgetEntry ( cuiWidgetType Type, cuiWidget *ParentWidget, const char *Text, const int X, const int Y, const unsigned Width, const unsigned Height )
	{
	if ( Initialized == false )
		{
		LOG_ERROR ( "CrossUI not initialized" );
		return NULL;
		}
	cuiWidget *NewWidget = ( cuiWidget * ) calloc ( 1, sizeof ( cuiWidget ) );
	if ( NewWidget == NULL )
		return NULL;
	NewWidget->Type = Type;
	NewWidget->X = X;
	NewWidget->Y = Y;
	NewWidget->Width = Width;
	NewWidget->Height = Height;
	if ( Text != NULL )
		{
		NewWidget->Text = strdup ( Text );
		if ( NewWidget->Text == NULL )
			{
			free ( NewWidget );
			return NULL;
			}
		}
	NewWidget->Parent = ParentWidget;
	NewWidget->Visible = true;
	NewWidget->Enabled = true;
	PointerArray_Initialize ( &NewWidget->Children );
	if ( cuiInternal_AddChildToParent ( ParentWidget, NewWidget ) == false )
		{
		PointerArray_Destroy ( &NewWidget->Children );
		SAFE_DEL_C ( NewWidget->Text );
		free ( NewWidget );
		return NULL;
		}
	if ( cuiInternal_AddWidgetToList ( NewWidget ) == false )
		{
		cuiInternal_RemoveChildFromParent ( ParentWidget, NewWidget );
		PointerArray_Destroy ( &NewWidget->Children );
		SAFE_DEL_C ( NewWidget->Text );
		free ( NewWidget );
		return NULL;
		}
	return NewWidget;
	}

void cuiInternal_DestroyWidgetEntry ( cuiWidget *Widget )
	{
	if ( ( Widget == NULL ) || ( Widget->BeingDestroyed ) )
		return;
	Widget->BeingDestroyed = true;

	if ( Widget->Callbacks.Destroyed )
		{
		Widget->Callbacks.Destroyed ( Widget );
		}

	for ( unsigned long Index = PointerArray_GetSize ( &Widget->Children ); Index > 0; --Index )
		{
		cuiInternal_DestroyWidgetEntry ( ( cuiWidget * ) PointerArray_Get ( &Widget->Children, Index - 1 ) );
		}
	PointerArray_Destroy ( &Widget->Children );

	cuiInternal_RemoveChildFromParent ( Widget->Parent, Widget );
	cuiInternal_RemoveWidgetFromList ( Widget );
	cuiBackend_DestroyNativeWidget ( Widget );
	//FreeItems ( Widget );
	SAFE_DEL_C ( Widget->Text );
	Widget->Type = cuiType_Dead;
	free ( Widget );
	}

void cuiInternal_DestroyAllWidgets ( void )
	{
	while ( PointerList_IsEmpty ( &WidgetList ) == false )
		{
		cuiWidget *Widget = ( cuiWidget * ) PointerList_GetNodeData ( PointerList_GetFirst ( &WidgetList ) );
		cuiInternal_DestroyWidgetEntry ( Widget );
		}
	}

unsigned cuiInternal_GetWidgetCount ( void )
	{
	return PointerList_GetSize ( &WidgetList );
	}

const char *Stringify_cuiWidgetType ( const cuiWidgetType Value )
	{
	switch ( Value )
		{
#define STRINGIFY(X) case cuiType_##X:return #X;
			STRINGIFY ( Dead );
			STRINGIFY ( Window );
			STRINGIFY ( Button );
			STRINGIFY ( CheckBox );
			STRINGIFY ( Slider );
			STRINGIFY ( ProgressBar );
			STRINGIFY ( Label );
			STRINGIFY ( TextArea );
			STRINGIFY ( TextBox );
			STRINGIFY ( GroupBox );
#undef STRINGIFY
		}
	return "Unknown widget type";
	}