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

bool cuiBackend_Initialize ( void )
	{
	int Argc = 1;
	char Arg0[] = "CrossUI";
	char *Argv[] = { Arg0, NULL };
	char **ArgvPtr = Argv;
	return gtk_init_check ( &Argc, &ArgvPtr );
	}

void cuiBackend_Shutdown ( void )
	{
	}

void cuiBackend_DestroyNativeWidget ( cuiWidget *Widget )
	{
	ASSERT_FAIL ( Widget != NULL );
	if ( ( Widget == NULL ) || ( Widget->NativeHandle == NULL ) )
		return;

	LOG_DEBUG ( "Destroying widget %p (%s)", Widget, Stringify_cuiWidgetType ( Widget->Type ) );
	//if ( Widget->Type == cuiType_Tree )
	//cuiBackend_TreeClear ( Widget );
	g_signal_handlers_disconnect_matched ( GTK_WIDGET ( Widget->NativeHandle ), G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, Widget );
	gtk_widget_destroy ( GTK_WIDGET ( Widget->NativeHandle ) );
	Widget->NativeHandle = NULL;
	Widget->NativeLayoutHandle = NULL;
	}

void FinishedCreatingNewWidget ( cuiWidget *Widget )
	{
	ASSERT_FAIL ( Widget != NULL );
	if ( ( Widget == NULL ) || ( Widget->NativeHandle == NULL ) )
		return;
	GtkWidget *Container = GetContainer ( Widget );
	gtk_widget_set_size_request ( GTK_WIDGET ( Widget->NativeHandle ), ( gint ) Widget->Width, ( gint ) Widget->Height );
	if ( Container != NULL )
		gtk_fixed_put ( GTK_FIXED ( Container ), GTK_WIDGET ( Widget->NativeHandle ), Widget->X, Widget->Y );
	if ( Widget->Visible )
		gtk_widget_show_all ( GTK_WIDGET ( Widget->NativeHandle ) );
	else
		gtk_widget_hide ( GTK_WIDGET ( Widget->NativeHandle ) );
	gtk_widget_set_sensitive ( GTK_WIDGET ( Widget->NativeHandle ), Widget->Enabled ? TRUE : FALSE );
	LOG_DEBUG ( "Created widget %p (%s)", Widget, Stringify_cuiWidgetType ( Widget->Type ) );
	}
