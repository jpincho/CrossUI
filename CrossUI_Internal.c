#include "CrossUI_Internal.h"
#include "CrossUI_InternalBackendFunctions.h"
#include <Platform/Platform.h>
#include <Platform/ArrayUtils.h>
#include <Platform/Logger.h>

#if defined (PLATFORM_COMPILER_GNU)
#include <stdlib.h>
#include <string.h>
#endif

bool Initialized = false;

static struct
	{
	cuiWidget *First, *Last;
	unsigned Count;
	} WidgetList = {0};

static void cuiInternal_AddChildToParent ( cuiWidget *Parent, cuiWidget *Child )
	{
	if ( Parent == NULL )
		return;
	ARRAY_PUSH_BACK ( Parent->Children, Parent->ChildCount, Parent->ChildCapacity, Child );
	}

static void cuiInternal_RemoveChildFromParent ( cuiWidget *Parent, cuiWidget *Child )
	{
	if ( Parent == NULL )
		return;

	for ( unsigned Iterator = 0; Iterator < Parent->ChildCount; ++Iterator )
		{
		if ( Parent->Children[Iterator] == Child )
			{
			ARRAY_DELETE_AND_SHIFT ( Parent->Children, Parent->ChildCount, Iterator );
			return;
			}
		}
	}

static void cuiInternal_AddWidgetToList ( cuiWidget *Widget )
	{
	if ( Widget == NULL )
		return;
	Widget->Previous = WidgetList.Last;
	Widget->Next = NULL;
	if ( WidgetList.Last != NULL )
		WidgetList.Last->Next = Widget;
	else
		WidgetList.First = Widget;
	WidgetList.Last = Widget;
	++WidgetList.Count;
	}

static void cuiInternal_RemoveWidgetFromList ( cuiWidget *Widget )
	{
	if ( Widget == NULL )
		return;

	if ( WidgetList.First == Widget )
		WidgetList.First = Widget->Next;
	if ( WidgetList.Last == Widget )
		WidgetList.Last = Widget->Previous;

	if ( Widget->Next != NULL )
		Widget->Next->Previous = Widget->Previous;
	if ( Widget->Previous != NULL )
		Widget->Previous->Next = Widget->Next;
	--WidgetList.Count;
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
	cuiInternal_AddChildToParent ( ParentWidget, NewWidget );

	cuiInternal_AddWidgetToList ( NewWidget );
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

	while ( Widget->ChildCount > 0 )
		cuiInternal_DestroyWidgetEntry ( Widget->Children[Widget->ChildCount - 1] );

	cuiInternal_RemoveChildFromParent ( Widget->Parent, Widget );
	cuiInternal_RemoveWidgetFromList ( Widget );
	cuiBackend_DestroyNativeWidget ( Widget );
	//FreeItems ( Widget );
	SAFE_DEL_C ( Widget->Children );
	SAFE_DEL_C ( Widget->Text );
	Widget->Type = cuiType_Dead;
	free ( Widget );
	}

void cuiInternal_DestroyAllWidgets ( void )
	{
	while ( WidgetList.Count > 0 )
		{
		cuiInternal_DestroyWidgetEntry ( WidgetList.First );
		}
	}

unsigned cuiInternal_GetWidgetCount ( void )
	{
	return WidgetList.Count;
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
			STRINGIFY ( ComboBox );
			STRINGIFY ( Slider );
			STRINGIFY ( ProgressBar );
			STRINGIFY ( Label );
			STRINGIFY ( TextArea );
			STRINGIFY ( TextBox );
			STRINGIFY ( ListBox );
			STRINGIFY ( GroupBox );
			STRINGIFY ( Tree );
#undef STRINGIFY
		}
	return "Unknown widget type";
	}