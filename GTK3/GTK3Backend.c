#include "GTK3BackendInternal.h"

const char *cuiGetBackendName ( void )
	{
	return "GTK";
	}

bool cuiUpdate ( const bool Wait )
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

void cuiSetWidgetText ( cuiWidget *Widget, const char *Text )
	{
	if ( Widget == NULL )
		return;
	GtkWidget *Native = GetInnermostWidget ( Widget );
	SAFE_DEL_C ( Widget->Text );
	if ( Text != NULL )
		Widget->Text = strdup ( Text );
	switch ( Widget->Type )
		{
		case cuiType_Window:
			gtk_window_set_title ( GTK_WINDOW ( Widget->NativeHandle ), EMPTY_STRING_IF_NULL ( Widget->Text ) );
			break;
		case cuiType_Button:
			gtk_button_set_label ( GTK_BUTTON ( Native ), EMPTY_STRING_IF_NULL ( Widget->Text ) );
			break;
		case cuiType_CheckBox:
			gtk_button_set_label ( GTK_BUTTON ( Native ), EMPTY_STRING_IF_NULL ( Widget->Text ) );
			break;
		case cuiType_Label:
			gtk_label_set_label ( GTK_LABEL ( Native ), EMPTY_STRING_IF_NULL ( Widget->Text ) );
			break;
		default:
			break;
		}
	}

void cuiGetWidgetText ( const cuiWidget *Widget, char *Buffer, const unsigned BufferSize )
	{
	if ( Widget == NULL )
		return;
	if ( BufferSize == 0 )
		return;
	if ( Buffer == NULL )
		return;
	GtkWidget *Native = GetInnermostWidget ( Widget );
	const char *Text = "";
	Buffer[BufferSize - 1] = 0;
	switch ( Widget->Type )
		{
		case cuiType_Window:
			Text = gtk_window_get_title ( GTK_WINDOW ( Widget->NativeHandle ) );
			break;
		case cuiType_Button:
		case cuiType_CheckBox:
			Text = gtk_button_get_label ( GTK_BUTTON ( Native ) );
			break;
		case cuiType_Label:
			Text = gtk_label_get_label ( GTK_LABEL ( Native ) );
			break;
		default:
			if ( Widget->Text != NULL )
				Text = Widget->Text;
			break;
		}
	if ( Text == NULL )
		{
		Buffer[0] = 0;
		return;
		}
	strncpy ( Buffer, Text, BufferSize - 1 );
	}

void cuiSetWidgetBounds ( cuiWidget *Widget, const int X, const int Y, const unsigned Width, const unsigned Height )
	{
	if ( Widget == NULL )
		return;
	GtkWidget *Native = GetInnermostWidget ( Widget );

	Widget->Width = Width;
	Widget->Height = Height;
	Widget->X = X;
	Widget->Y = Y;

	gtk_widget_set_size_request ( Native, ( gint ) Width, ( gint ) Height );
	if ( Widget->Type == cuiType_Window )
		{
		gtk_window_resize ( GTK_WINDOW ( Native ), ( gint ) Width, ( gint ) Height );
		if ( ( Widget->X >= 0 ) && ( Widget->Y >= 0 ) )
			gtk_window_move ( GTK_WINDOW ( Native ), X, Y );
		return;
		}
	GtkWidget *Container = GetContainer ( Widget );
	if ( Container != NULL )
		gtk_fixed_move ( GTK_FIXED ( Container ), Native, X, Y );
	}

bool cuiGetWidgetBounds ( cuiWidget *Widget, int *X, int *Y, unsigned *Width, unsigned *Height )
	{
	if ( Widget == NULL )
		return false;
	GtkWidget *Native = GetInnermostWidget ( Widget );
	if ( ( Width != NULL ) && ( Height != NULL ) )
		{
		if ( Widget->Type == cuiType_Window )
			{
			gtk_window_get_size ( GTK_WINDOW ( Native ), Width, Height );
			}
		else
			{
			gtk_widget_get_size_request ( Native, Width, Height );
			}
		Widget->Width = *Width;
		Widget->Height = *Height;
		}
	if ( ( X != NULL ) && ( Y != NULL ) )
		{
		if ( Widget->Type == cuiType_Window )
			{
			gtk_window_get_position ( GTK_WINDOW ( Native ), X, Y );
			}
		else
			{
			}
		Widget->X = *X;
		Widget->Y = *Y;
		}
	return true;
	}

void cuiSetVisible ( cuiWidget *Widget, const bool Visible )
	{
	if ( Widget == NULL )
		return;

	GtkWidget *Native = GTK_WIDGET ( Widget->NativeHandle );
	if ( Native == NULL )
		return;
	gtk_widget_set_visible ( Native, Visible );
	Widget->Visible = Visible;
	}

bool cuiIsVisible ( const cuiWidget *Widget )
	{
	if ( Widget == NULL )
		return false;

	GtkWidget *Native = GTK_WIDGET ( Widget->NativeHandle );
	if ( Native == NULL )
		return false;
	return gtk_widget_get_visible ( Native );
	}

void cuiSetEnabled ( cuiWidget *Widget, const bool Enabled )
	{
	if ( Widget == NULL )
		return;

	GtkWidget *Native = GTK_WIDGET ( Widget->NativeHandle );
	if ( Native == NULL )
		return;
	gtk_widget_set_sensitive ( Native, Enabled );
	Widget->Enabled = Enabled;
	}

bool cuiIsEnabled ( const cuiWidget *Widget )
	{
	if ( Widget == NULL )
		return false;

	GtkWidget *Native = GTK_WIDGET ( Widget->NativeHandle );
	if ( Native == NULL )
		return false;
	return gtk_widget_get_sensitive ( Native );
	}

void cuiSetFocus ( const cuiWidget *Widget )
	{
	if ( Widget == NULL )
		return;

	GtkWidget *Native = GetInnermostWidget ( Widget );
	if ( Native == NULL )
		return;
	gtk_widget_grab_focus ( Native );
	}