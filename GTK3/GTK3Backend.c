#include "GTK3Backend.h"
#include <Platform/Platform.h>
#include <Platform/Logger.h>
#include <stdlib.h>
#include "GTK3Slider.h"
#include "GTK3ProgressBar.h"
#include "GTK3CheckBox.h"
#include "GTK3Button.h"
#include "GTK3Label.h"

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

GtkWidget *GetContainer ( cuiWidget *Widget )
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

GtkWidget *GetInnermostWidget ( cuiWidget *Widget )
	{
	if ( Widget->NativeInnerHandle != NULL )
		return GTK_WIDGET ( Widget->NativeInnerHandle );
	ASSERT_FAIL ( Widget->NativeHandle != NULL );
	return GTK_WIDGET ( Widget->NativeHandle );
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
	LOG_DEBUG ( "Setting up widget %p as a %s", Widget, Stringify_cuiWidgetType ( Widget->Type ) );

	switch ( Widget->Type )
		{
		case cuiType_Window:
			{
			GtkWidget *Fixed = gtk_fixed_new ();
			GtkWidget *Native = gtk_window_new ( GTK_WINDOW_TOPLEVEL );
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
			if ( cuiBackend_Button_Create ( Widget ) == false )
				return false;
			break;
			}
		case cuiType_Label:
			{
			if ( cuiBackend_Label_Create ( Widget ) == false )
				return false;
			break;
			}
		case cuiType_ProgressBar:
			{
			if ( cuiBackend_ProgressBar_Create ( Widget ) == false )
				return false;
			break;
			}
		case cuiType_Slider:
			{
			if ( cuiBackend_Slider_Create ( Widget ) == false )
				return false;
			break;
			}
		case cuiType_CheckBox:
			{
			if ( cuiBackend_CheckBox_Create ( Widget ) == false )
				return false;
			break;
			}

		default:
			return false;
		}

	PlaceWidget ( Widget, Widget->NativeHandle );
	Widget->NativeInnerHandle = Widget->NativeHandle;
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

void cuiBackend_SetText ( cuiWidget *Widget, const char *Text )
	{
	GtkWidget *Native = GTK_WIDGET ( Widget->NativeHandle );
	if ( Widget->NativeInnerHandle != NULL )
		Native = GTK_WIDGET ( Widget->NativeInnerHandle );
	if ( Native == NULL )
		return;
	static char EmptyString[] = "";
	if ( Text == NULL )
		Text = EmptyString;

	switch ( Widget->Type )
		{
		case cuiType_Window:
			gtk_window_set_title ( GTK_WINDOW ( Widget->NativeHandle ), Text );
			break;
		case cuiType_Label:
			gtk_label_set_text ( GTK_LABEL ( Native ), Text );
			break;
		case cuiType_Button:
			gtk_button_set_label ( GTK_BUTTON ( Native ), Text );
			break;
		case cuiType_CheckBox:
			gtk_button_set_label ( GTK_BUTTON ( Native ), Text );
			break;
		case cuiType_TextBox:
			gtk_entry_set_text ( GTK_ENTRY ( Native ), Text );
			break;
		case cuiType_TextArea:
			{
			GtkTextBuffer *Buffer = gtk_text_view_get_buffer ( GTK_TEXT_VIEW ( Native ) );
			gtk_text_buffer_set_text ( Buffer, Text, -1 );
			break;
			}
		case cuiType_GroupBox:
			gtk_frame_set_label ( GTK_FRAME ( Widget->NativeHandle ), Text );
			break;
		default:
			break;
		}
	}

void cuiBackend_GetText ( cuiWidget *Widget, char *Buffer, const unsigned BufferSize )
	{
	GtkWidget *Native = GetInnermostWidget ( Widget );
	const char *Text = "";
	if ( Native == NULL )
		{
		if ( Widget->Text == NULL )
			{
			Buffer[0] = 0;
			return;
			}
		strncpy ( Buffer, Widget->Text, BufferSize );
		return;
		}
	switch ( Widget->Type )
		{
		case cuiType_Window:
			Text = gtk_window_get_title ( GTK_WINDOW ( Widget->NativeHandle ) );
			strncpy ( Buffer, Text, BufferSize );
			break;
		case cuiType_Label:
			Text = gtk_label_get_text ( GTK_LABEL ( Native ) );
			strncpy ( Buffer, Text, BufferSize );
			break;
		case cuiType_Button:
		case cuiType_CheckBox:
			Text = gtk_button_get_label ( GTK_BUTTON ( Native ) );
			strncpy ( Buffer, Text, BufferSize );
			break;
		case cuiType_TextBox:
			Text = gtk_entry_get_text ( GTK_ENTRY ( Native ) );
			strncpy ( Buffer, Text, BufferSize );
			break;
		case cuiType_TextArea:
			{
			GtkTextBuffer *TextBuffer = gtk_text_view_get_buffer ( GTK_TEXT_VIEW ( Native ) );
			GtkTextIter Start, End;
			char *Allocated;
			gtk_text_buffer_get_bounds ( TextBuffer, &Start, &End );
			Allocated = gtk_text_buffer_get_text ( TextBuffer, &Start, &End, FALSE );
			strncpy ( Buffer, Allocated, BufferSize );
			g_free ( Allocated );
			break;
			}
		case cuiType_GroupBox:
			Text = gtk_frame_get_label ( GTK_FRAME ( Widget->NativeHandle ) );
			strncpy ( Buffer, Text, BufferSize );
			break;
		default:
			Text = Widget->Text;
			strncpy ( Buffer, Text, BufferSize );
			break;
		}
	}

void cuiBackend_SetFocus ( cuiWidget *Widget )
	{
	GtkWidget *Native = GetInnermostWidget ( Widget );
	gtk_widget_grab_focus ( Native );
	}

