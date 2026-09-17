#include "GTK3BackendInternal.h"

GtkWidget *GetContainer ( const cuiWidget *Widget )
	{
	ASSERT_FAIL ( Widget != NULL );
	cuiWidget *Parent = Widget->Parent;
	while ( Parent != NULL )
		{
		if ( Parent->NativeInnerHandle != NULL )
			return GTK_WIDGET ( Parent->NativeInnerHandle );
		Parent = Parent->Parent;
		}
	return NULL;
	}

GtkWidget *GetInnermostWidget ( const cuiWidget *Widget )
	{
	ASSERT_FAIL ( Widget != NULL );
	if ( Widget->NativeInnerHandle != NULL )
		return GTK_WIDGET ( Widget->NativeInnerHandle );
	ASSERT_FAIL ( Widget->NativeHandle != NULL );
	return GTK_WIDGET ( Widget->NativeHandle );
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
	GtkWidget *Native = GTK_WIDGET ( Widget->NativeHandle );
	if ( Native == NULL )
		return;
	LOG_DEBUG ( "Destroying widget %p (%s)", Widget, Stringify_cuiWidgetType ( Widget->Type ) );
	//if ( Widget->Type == cuiType_Tree )
	//cuiBackend_TreeClear ( Widget );
	Widget->NativeHandle = NULL;
	Widget->NativeInnerHandle = NULL;
	gtk_widget_destroy ( Native );
	}

void FinishedCreatingNewWidget ( cuiWidget *Widget )
	{
	ASSERT_FAIL ( Widget != NULL );
	GtkWidget *Container = GetContainer ( Widget );
	gtk_widget_set_size_request ( Widget->NativeHandle, ( gint ) Widget->Width, ( gint ) Widget->Height );
	if ( Container != NULL )
		gtk_fixed_put ( GTK_FIXED ( Container ), Widget->NativeHandle, Widget->X, Widget->Y );
	if ( Widget->Visible )
		gtk_widget_show_all ( Widget->NativeHandle );
	else
		gtk_widget_hide ( Widget->NativeHandle );
	gtk_widget_set_sensitive ( Widget->NativeHandle, Widget->Enabled ? TRUE : FALSE );
	LOG_DEBUG ( "Created widget %p (%s)", Widget, Stringify_cuiWidgetType ( Widget->Type ) );
	}

void cuiBackend_SetVisible ( cuiWidget *Widget, const bool Visible )
	{
	ASSERT_FAIL ( Widget != NULL );
	GtkWidget *Native = GTK_WIDGET ( Widget->NativeHandle );
	if ( Native == NULL )
		return;
	Widget->Visible = Visible;
	if ( Widget->Visible )
		gtk_widget_show_all ( Native );
	else
		gtk_widget_hide ( Native );
	}

void cuiBackend_SetEnabled ( cuiWidget *Widget, const bool Enabled )
	{
	ASSERT_FAIL ( Widget != NULL );
	GtkWidget *Native = GTK_WIDGET ( Widget->NativeHandle );
	if ( Native == NULL )
		return;
	Widget->Enabled = Enabled;
	gtk_widget_set_sensitive ( Native, Widget->Enabled ? TRUE : FALSE );
	}
