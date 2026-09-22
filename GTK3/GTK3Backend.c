#include "GTK3BackendInternal.h"
#include <string.h>

GtkWidget *GetOutmostWidgetHandle ( const cuiWidget *Widget )
	{
	if ( Widget == NULL )
		return NULL;
	switch ( Widget->Type )
		{
		case cuiType_ListBox:
		case cuiType_Tree:
			return Widget->ScrollHandle;
		default:
			return Widget->NativeHandle;
		}
	}

const char *cuiGetBackendName ( void )
	{
	return "GTK3";
	}

bool cuiUpdate ( const bool Wait )
	{
	if ( ( Initialized == false ) || ( cuiInternal_GetWidgetCount () == 0 ) )
		return false;
	gtk_main_iteration_do ( Wait ? TRUE : FALSE );
	if ( Wait == false )
		{
		while ( gtk_events_pending () )
			gtk_main_iteration_do ( FALSE );
		}
	return cuiInternal_GetWidgetCount () > 0;
	}

void cuiSetWidgetText ( cuiWidget *Widget, const char *Text )
	{
	if ( ( Widget == NULL ) || ( Widget->BeingDestroyed == true ) || ( Widget->NativeHandle == NULL ) )
		return;
	char *Copy = NULL;
	if ( Text != NULL )
		{
		Copy = strdup ( Text );
		if ( Copy == NULL )
			return;
		}
	SAFE_DEL_C ( Widget->Text );
	Widget->Text = Copy;
	switch ( Widget->Type )
		{
		case cuiType_Window:
			gtk_window_set_title ( GTK_WINDOW ( Widget->NativeHandle ), EMPTY_STRING_IF_NULL ( Widget->Text ) );
			break;
		case cuiType_Button:
			gtk_button_set_label ( GTK_BUTTON ( Widget->NativeHandle ), EMPTY_STRING_IF_NULL ( Widget->Text ) );
			break;
		case cuiType_CheckBox:
			gtk_button_set_label ( GTK_BUTTON ( Widget->NativeHandle ), EMPTY_STRING_IF_NULL ( Widget->Text ) );
			break;
		case cuiType_Label:
			gtk_label_set_label ( GTK_LABEL ( Widget->NativeHandle ), EMPTY_STRING_IF_NULL ( Widget->Text ) );
			break;
		case cuiType_TextBox:
			gtk_entry_set_text ( GTK_ENTRY ( Widget->NativeHandle ), EMPTY_STRING_IF_NULL ( Text ) );
			break;
		case cuiType_TextArea:
			{
			GtkTextBuffer *Buffer = gtk_text_view_get_buffer ( GTK_TEXT_VIEW ( Widget->TextAreaBufferView ) );
			gtk_text_buffer_set_text ( Buffer, EMPTY_STRING_IF_NULL ( Text ), -1 );
			break;
			}
		case cuiType_GroupBox:
			gtk_frame_set_label ( GTK_FRAME ( Widget->NativeHandle ), EMPTY_STRING_IF_NULL ( Widget->Text ) );
			break;

		default:
			break;
		}
	}

void cuiGetWidgetText ( const cuiWidget *Widget, char *Buffer, const unsigned BufferSize )
	{
	const char *Text = "";
	if ( ( Widget == NULL ) || ( Buffer == NULL ) || ( BufferSize == 0 ) )
		return;
	if ( Widget->NativeHandle == NULL )
		return;

	Buffer[0] = 0;
	switch ( Widget->Type )
		{
		case cuiType_Window:
			{
			Text = gtk_window_get_title ( GTK_WINDOW ( Widget->NativeHandle ) );
			break;
			}
		case cuiType_Button:
		case cuiType_CheckBox:
			Text = gtk_button_get_label ( GTK_BUTTON ( Widget->NativeHandle ) );
			break;
		case cuiType_Label:
			Text = gtk_label_get_label ( GTK_LABEL ( Widget->NativeHandle ) );
			break;
		case cuiType_TextBox:
			Text = gtk_entry_get_text ( GTK_ENTRY ( Widget->NativeHandle ) );
			break;
		case cuiType_TextArea:
			{
			GtkTextBuffer *TextBuffer = gtk_text_view_get_buffer ( GTK_TEXT_VIEW ( Widget->TextAreaBufferView ) );
			GtkTextIter Start, End;
			char *Allocated;
			gtk_text_buffer_get_bounds ( TextBuffer, &Start, &End );
			Allocated = gtk_text_buffer_get_text ( TextBuffer, &Start, &End, FALSE );
			strncpy ( Buffer, Allocated, BufferSize - 1 );
			Buffer[BufferSize - 1] = 0;
			g_free ( Allocated );
			return;
			}
		default:
			Text = Widget->Text;
			break;
		}
	if ( Text == NULL )
		return;
	strncpy ( Buffer, Text, BufferSize - 1 );
	Buffer[BufferSize - 1] = 0;
	}

void cuiSetWidgetBounds ( cuiWidget *Widget, const int X, const int Y, const unsigned Width, const unsigned Height )
	{
	if ( ( Widget == NULL ) || ( Widget->BeingDestroyed == true ) || ( Widget->NativeHandle == NULL ) )
		return;

	Widget->Width = Width;
	Widget->Height = Height;
	Widget->X = X;
	Widget->Y = Y;

	if ( Widget->Type == cuiType_Window )
		{
		gtk_widget_set_size_request ( GTK_WIDGET ( Widget->NativeHandle ), ( gint ) Width, ( gint ) Height );
		gtk_window_resize ( GTK_WINDOW ( Widget->NativeHandle ), ( gint ) Width, ( gint ) Height );
		if ( ( Widget->X >= 0 ) && ( Widget->Y >= 0 ) )
			gtk_window_move ( GTK_WINDOW ( Widget->NativeHandle ), X, Y );
		}
	else
		{
		GtkWidget *OutmostWidget = GetOutmostWidgetHandle ( Widget );
		GtkWidget *Container = GetContainer ( Widget );
		gtk_widget_set_size_request ( OutmostWidget, ( gint ) Width, ( gint ) Height );
		if ( Container != NULL )
			gtk_fixed_move ( GTK_FIXED ( Container ), OutmostWidget, X, Y );
		}
	}

bool cuiGetWidgetBounds ( cuiWidget *Widget, int *X, int *Y, unsigned *Width, unsigned *Height )
	{
	if ( ( Widget == NULL ) || ( Widget->NativeHandle == NULL ) )
		return false;

	if ( ( Width != NULL ) || ( Height != NULL ) )
		{
		gint NativeWidth = 0;
		gint NativeHeight = 0;
		if ( Widget->Type == cuiType_Window )
			gtk_window_get_size ( GTK_WINDOW ( Widget->NativeHandle ), &NativeWidth, &NativeHeight );
		else
			gtk_widget_get_size_request ( GetOutmostWidgetHandle ( Widget ), &NativeWidth, &NativeHeight );
		if ( NativeWidth < 0 )
			NativeWidth = 0;
		if ( NativeHeight < 0 )
			NativeHeight = 0;
		Widget->Width = ( unsigned ) NativeWidth;
		Widget->Height = ( unsigned ) NativeHeight;
		if ( Width != NULL )
			*Width = Widget->Width;
		if ( Height != NULL )
			*Height = Widget->Height;
		}
	if ( ( X != NULL ) || ( Y != NULL ) )
		{
		gint NativeX = 0;
		gint NativeY = 0;
		if ( Widget->Type == cuiType_Window )
			gtk_window_get_position ( GTK_WINDOW ( Widget->NativeHandle ), &NativeX, &NativeY );
		Widget->X = NativeX;
		Widget->Y = NativeY;
		if ( X != NULL )
			*X = Widget->X;
		if ( Y != NULL )
			*Y = Widget->Y;
		}
	return true;
	}

void cuiSetVisible ( cuiWidget *Widget, const bool Visible )
	{
	if ( ( Widget == NULL ) || ( Widget->BeingDestroyed == true ) || ( Widget->NativeHandle == NULL ) )
		return;
	if ( Widget->ScrollHandle != NULL )
		gtk_widget_set_visible ( GTK_WIDGET ( Widget->ScrollHandle ), Visible );
	if ( Widget->NativeHandle != NULL )
		gtk_widget_set_visible ( GTK_WIDGET ( Widget->NativeHandle ), Visible );
	Widget->Visible = Visible;
	}

bool cuiIsVisible ( const cuiWidget *Widget )
	{
	if ( ( Widget == NULL ) || ( Widget->NativeHandle == NULL ) )
		return false;
	GtkWidget *WidgetToConsider = GetOutmostWidgetHandle ( Widget );
	return gtk_widget_get_visible ( WidgetToConsider );
	}

void cuiSetEnabled ( cuiWidget *Widget, const bool Enabled )
	{
	if ( ( Widget == NULL ) || ( Widget->BeingDestroyed == true ) || ( Widget->NativeHandle == NULL ) )
		return;
	if ( Widget->ScrollHandle != NULL )
		gtk_widget_set_sensitive ( GTK_WIDGET ( Widget->ScrollHandle ), Enabled );
	if ( Widget->NativeHandle != NULL )
		gtk_widget_set_sensitive ( GTK_WIDGET ( Widget->NativeHandle ), Enabled );
	Widget->Enabled = Enabled;
	}

bool cuiIsEnabled ( const cuiWidget *Widget )
	{
	if ( ( Widget == NULL ) || ( Widget->NativeHandle == NULL ) )
		return false;
	GtkWidget *WidgetToConsider = GetOutmostWidgetHandle ( Widget );
	return gtk_widget_get_sensitive ( WidgetToConsider );
	}

void cuiSetFocus ( const cuiWidget *Widget )
	{
	if ( ( Widget == NULL ) || ( Widget->BeingDestroyed == true ) || ( Widget->NativeHandle == NULL ) )
		return;
	gtk_widget_grab_focus ( GTK_WIDGET ( Widget->NativeHandle ) );
	}