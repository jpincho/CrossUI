#include "GTK3BackendInternal.h"

GtkWidget *GetContainer ( const cuiWidget *Widget )
	{
	ASSERT_FAIL ( Widget != NULL );
	cuiWidget *Parent = Widget->Parent;
	while ( Parent != NULL )
		{
		if ( Parent->NativeLayoutHandle != NULL )
			return GTK_WIDGET ( Parent->NativeLayoutHandle );
		Parent = Parent->Parent;
		}
	return NULL;
	}

bool cuiBackend_Initialize ( int argc, char *argv[] )
	{
	return gtk_init_check ( ( int* ) &argc, ( char *** ) &argv );
	}

void cuiBackend_Shutdown ( void )
	{
	}

void cuiBackend_DestroyNativeWidget ( cuiWidget *Widget )
	{
	ASSERT_FAIL ( Widget != NULL );
	if ( ( Widget == NULL ) )
		return;

	LOG_DEBUG ( "Destroying widget %p (%s)", Widget, Stringify_cuiWidgetType ( Widget->Type ) );
	if ( Widget->Type == cuiType_Tree )
		cuiClearTreeItems ( Widget );

	GtkWidget *OutmostWidget = GetOutmostWidgetHandle ( Widget );
	if ( OutmostWidget != NULL )
		{
		g_signal_handlers_disconnect_matched ( OutmostWidget, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, Widget );
		gtk_widget_destroy ( OutmostWidget );
		}
	Widget->NativeHandle = NULL;
	Widget->ScrollHandle = NULL;
	Widget->NativeLayoutHandle = NULL;
	}

void FinishedCreatingNewWidget ( cuiWidget *Widget )
	{
	ASSERT_FAIL ( Widget != NULL );
	if ( ( Widget == NULL ) || ( Widget->NativeHandle == NULL ) )
		return;
	GtkWidget *Container = GetContainer ( Widget );
	GtkWidget *OutmostWidget = Widget->NativeHandle;
	if ( Widget->ScrollHandle != NULL )
		OutmostWidget = Widget->ScrollHandle;
	gtk_widget_set_size_request ( OutmostWidget, ( gint ) Widget->Width, ( gint ) Widget->Height );
	if ( Container != NULL )
		gtk_fixed_put ( GTK_FIXED ( Container ), OutmostWidget, Widget->X, Widget->Y );
	if ( Widget->Visible )
		gtk_widget_show_all ( OutmostWidget );
	else
		gtk_widget_hide ( OutmostWidget );
	gtk_widget_set_sensitive ( OutmostWidget, Widget->Enabled ? TRUE : FALSE );
	LOG_DEBUG ( "Created widget %p (%s)", Widget, Stringify_cuiWidgetType ( Widget->Type ) );
	}
