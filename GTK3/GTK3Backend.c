#include "GTK3Backend.h"
#include <Platform/Platform.h>
#include <Platform/Logger.h>
#include <stdlib.h>
#include <gtk/gtk.h>

bool cuiBackend_Initialize ( void )
	{
	int Argc = 1;
	char Arg0[] = "CrossUI";
	char *Argv[] = { Arg0, NULL };
	char **ArgvPtr = Argv;
	gtk_init ( &Argc, &ArgvPtr );
	return true;
	}

void cuiBackend_Shutdown ( void )
	{
	}

const char *cuiBackend_Name ( void )
	{
	return "GTK";
	}

static gboolean OnDelete ( GtkWidget *Native, GdkEvent *Event, gpointer Data )
	{
	cuiWidget *Widget = ( cuiWidget * ) Data;
	UNUSED ( Native );
	UNUSED ( Event );
	if ( Widget->Callbacks.Destroyed )
		{
		Widget->Callbacks.Destroyed ( cuiInternal_WidgetToHandle ( Widget ) );
		}

	cuiInternal_DestroyWidgetEntry ( Widget );
	return TRUE;
	}

static gboolean OnConfigure ( GtkWidget *Native, GdkEvent *Event, gpointer Data )
	{
	cuiWidget *Widget = ( cuiWidget * ) Data;
	UNUSED ( Native );
	if ( Widget->Callbacks.Resized )
		{
		if ( ( Event->configure.width != Widget->Width ) || ( Event->configure.height != Widget->Height ) )
			{
			Widget->Callbacks.Resized ( cuiInternal_WidgetToHandle ( Widget ), Event->configure.width, Event->configure.height );
			Widget->Width = Event->configure.width;
			Widget->Height = Event->configure.height;
			}
		}
	if ( Widget->Callbacks.Moved )
		{
		if ( ( Event->configure.x != Widget->X ) || ( Event->configure.y != Widget->Y ) )
			{
			Widget->Callbacks.Moved ( cuiInternal_WidgetToHandle ( Widget ), Event->configure.width, Event->configure.height );
			Widget->X = Event->configure.x;
			Widget->Y = Event->configure.y;
			}
		}
	return TRUE;
	}

static void OnButtonClicked ( GtkButton *Button, gpointer Data )
	{
	cuiWidget *Widget = ( cuiWidget * ) Data;
	UNUSED ( Button );
	if ( Widget->Callbacks.Clicked )
		{
		Widget->Callbacks.Clicked ( cuiInternal_WidgetToHandle ( Widget ) );
		}
	}

static GtkWidget *GetContainer ( cuiWidget *Widget )
	{
	cuiWidget *Parent = Widget->Parent;
	while ( Parent != NULL )
		{
		if ( Parent->NativeInnerHandle != NULL )
			return GTK_WIDGET ( Parent->NativeInnerHandle );
		Parent = Parent->Parent;
		}
	return NULL;
	}

static void PlaceWidget ( cuiWidget *Widget, GtkWidget *Native )
	{
	GtkWidget *Container = GetContainer ( Widget );
	gtk_widget_set_size_request ( Native, ( gint ) Widget->Width, ( gint ) Widget->Height );
	if ( Container != NULL )
		gtk_fixed_put ( GTK_FIXED ( Container ), Native, Widget->X, Widget->Y );
	if ( Widget->Visible )
		gtk_widget_show_all ( Native );
	else
		gtk_widget_hide ( Native );
	gtk_widget_set_sensitive ( Native, Widget->Enabled ? TRUE : FALSE );
	}

bool cuiBackend_CreateNativeWidget ( cuiWidget *Widget )
	{
	GtkWidget *Native = NULL;
	LOG_DEBUG ( "Setting up widget %p as a %s", Widget, Stringify_cuiWidgetType ( Widget->Type ) );

	switch ( Widget->Type )
		{
		case cuiType_Window:
			{
			GtkWidget *Fixed = gtk_fixed_new ();
			Native = gtk_window_new ( GTK_WINDOW_TOPLEVEL );
			gtk_window_set_title ( GTK_WINDOW ( Native ), Widget->Text );
			gtk_window_set_default_size ( GTK_WINDOW ( Native ), ( gint ) Widget->Width, ( gint ) Widget->Height );
			if ( ( Widget->X >= 0 ) && ( Widget->Y >= 0 ) )
				gtk_window_move ( GTK_WINDOW ( Native ), Widget->X, Widget->Y );
			else
				gtk_window_set_position ( GTK_WINDOW ( Native ), GTK_WIN_POS_CENTER );
			gtk_container_add ( GTK_CONTAINER ( Native ), Fixed );
			g_signal_connect ( Native, "delete-event", G_CALLBACK ( OnDelete ), Widget );
			gtk_widget_show_all ( Native );
			Widget->NativeHandle = Native;
			Widget->NativeInnerHandle = Fixed;
			return true;
			}
		case cuiType_Button:
			{
			Native = gtk_button_new_with_label ( Widget->Text );
			g_signal_connect ( Native, "clicked", G_CALLBACK ( OnButtonClicked ), Widget );
			break;
			}

		default:
			return false;
		}

	PlaceWidget ( Widget, Native );
	Widget->NativeHandle = Native;
	Widget->NativeInnerHandle = Native;
	return true;
	}

void cuiBackend_DestroyNativeWidget ( cuiWidget *Widget )
	{
	GtkWidget *Native = GTK_WIDGET ( Widget->NativeHandle );
	if ( Native == NULL )
		return;
	LOG_DEBUG ( "Destroying widget %p as a %s", Widget, Stringify_cuiWidgetType ( Widget->Type ) );
	//if ( Widget->Type == cuiType_Tree )
	//cuiBackend_TreeClear ( Widget );
	Widget->NativeHandle = NULL;
	Widget->NativeInnerHandle = NULL;
	gtk_widget_destroy ( Native );
	}


bool cuiBackend_Update ( bool Wait )
	{
	if ( cuiInternal_GetWidgetCount () == 0 )
		return false;
	gtk_main_iteration_do ( Wait ? TRUE : FALSE );
	if ( Wait == false )
		{
		while ( gtk_events_pending () )
			gtk_main_iteration_do ( FALSE );
		}
	return cuiInternal_GetWidgetCount () > 0;
	}

void cuiBackend_SetVisible ( cuiWidget *Widget, const bool Visible )
	{
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
	GtkWidget *Native = GTK_WIDGET ( Widget->NativeHandle );
	if ( Native == NULL )
		return;
	Widget->Enabled = Enabled;
	gtk_widget_set_sensitive ( Native, Widget->Enabled ? TRUE : FALSE );
	}