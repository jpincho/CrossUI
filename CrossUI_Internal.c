#include "CrossUI_Internal.h"
#include "CrossUI_BackendFunctions.h"
#include <Platform/Platform.h>
#include <Platform/ArrayUtils.h>

#if defined (PLATFORM_COMPILER_GNU)
#include <stdlib.h>
#include <string.h>
#endif

static cuiWidget *WidgetList = NULL;
static unsigned WidgetCount = 0;


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
			ARRAY_DELETE_AND_SWAP ( Parent->Children, Parent->ChildCount, Iterator );
			return;
			}
		}
	}

static void cuiInternal_AddWidgetToList ( cuiWidget *Widget )
	{
	if ( Widget == NULL )
		return;
	Widget->Previous = NULL;
	Widget->Next = WidgetList;
	if ( WidgetList != NULL )
		WidgetList->Previous = Widget;
	WidgetList = Widget;
	++WidgetCount;
	}

static void cuiInternal_RemoveWidgetFromList ( cuiWidget *Widget )
	{
	if ( Widget == NULL )
		return;

	if ( WidgetList == Widget )
		WidgetList = Widget->Next;

	if ( Widget->Next != NULL )
		Widget->Next->Previous = Widget->Previous;
	if ( Widget->Previous != NULL )
		Widget->Previous->Next = Widget->Next;
	--WidgetCount;
	}

cuiWidget *cuiInternal_CreateWidgetEntry ( cuiWidgetType Type, cuiWidget *ParentWidget, const char *Text, const int X, const int Y, const unsigned Width, const unsigned Height )
	{
	cuiWidget *NewWidget = ( cuiWidget * ) calloc ( 1, sizeof ( cuiWidget ) );
	if ( NewWidget == NULL )
		return NULL;
	NewWidget->Type = Type;
	NewWidget->X = X;
	NewWidget->Y = Y;
	NewWidget->Width = Width;
	NewWidget->Height = Height;
	NewWidget->Text = strdup ( Text );
	NewWidget->Parent = ParentWidget;
	cuiInternal_AddChildToParent ( ParentWidget, NewWidget );
	NewWidget->Enabled = true;
	NewWidget->Visible = true;

	cuiInternal_AddWidgetToList ( NewWidget );
	return NewWidget;
	}

void cuiInternal_DestroyWidgetEntry ( cuiWidget *Widget )
	{
	if ( ( Widget == NULL ) || Widget->BeingDestroyed )
		return;
	Widget->BeingDestroyed = true;

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
	while ( WidgetCount > 0 )
		{
		cuiInternal_DestroyWidgetEntry ( WidgetList );
		}
	}

unsigned cuiInternal_GetWidgetCount ( void )
	{
	return WidgetCount;
	}

cuiHandle cuiInternal_WidgetToHandle ( const cuiWidget *Widget )
	{
	return ( cuiHandle ) Widget;
	}

cuiWidget *cuiInternal_HandleToWidget ( const cuiHandle Handle )
	{
	return ( cuiWidget* ) Handle;
	}

const char *Stringify_cuiWidgetType ( const cuiWidgetType Value )
	{
	switch ( Value )
		{
#define STRINGIFY(X) case cuiType_##X:return #X;
			STRINGIFY ( Dead );
			STRINGIFY ( Window );
			STRINGIFY ( Label );
			STRINGIFY ( Button );
			STRINGIFY ( CheckBox );
			STRINGIFY ( TextBox );
			STRINGIFY ( TextArea );
			STRINGIFY ( ComboBox );
			STRINGIFY ( ListBox );
			STRINGIFY ( Tree );
			STRINGIFY ( Slider );
			STRINGIFY ( ProgressBar );
			STRINGIFY ( GroupBox );
			STRINGIFY ( Panel );
#undef STRINGIFY
		}
	return "Unknown cuiWidgetType";
	}