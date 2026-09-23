#include "GTK3BackendInternal.h"

cuiResult cuiMessageBox ( const cuiWidget *Parent, const char *Title, const char *Message, const cuiMessageType Type )
	{
	GtkWindow *ParentWindow = NULL;
	GtkMessageType GtkType = GTK_MESSAGE_INFO;
	GtkButtonsType Buttons = GTK_BUTTONS_OK;
	GtkWidget *Dialog;
	gint Response;

	if ( ( Parent != NULL ) && ( Parent->Type == cuiType_Window ) )
		ParentWindow = GTK_WINDOW ( Parent->NativeHandle );

	switch ( Type )
		{
		case cuiMessage_Warning:
			GtkType = GTK_MESSAGE_WARNING;
			break;
		case cuiMessage_Error:
			GtkType = GTK_MESSAGE_ERROR;
			break;
		case cuiMessage_Question:
			GtkType = GTK_MESSAGE_QUESTION;
			Buttons = GTK_BUTTONS_YES_NO;
			break;
		default:
			break;
		}

	Dialog = gtk_message_dialog_new ( ParentWindow, GTK_DIALOG_MODAL, GtkType, Buttons, "%s", ( Message != NULL ) ? Message : "" );
	if ( Title != NULL )
		gtk_window_set_title ( GTK_WINDOW ( Dialog ), Title );
	Response = gtk_dialog_run ( GTK_DIALOG ( Dialog ) );
	gtk_widget_destroy ( Dialog );

	if ( Response == GTK_RESPONSE_YES )
		return cuiResult_Yes;
	if ( Response == GTK_RESPONSE_NO )
		return cuiResult_No;
	if ( Response == GTK_RESPONSE_OK )
		return cuiResult_OK;
	return cuiResult_Cancel;
	}

static void ApplyFileFilters ( GtkFileChooser *Chooser, const char *Filter )
	{
	char *Copy;
	char *Token;
	char *Save = NULL;
	char *Name = NULL;
	if ( ( Filter == NULL ) || ( Filter[0] == 0 ) )
		return;
	Copy = strdup ( Filter );
	if ( Copy == NULL )
		return;
	for ( Token = strtok_r ( Copy, "|", &Save ); Token != NULL; Token = strtok_r ( NULL, "|", &Save ) )
		{
		if ( Name == NULL )
			Name = Token;
		else
			{
			GtkFileFilter *GtkFilter = gtk_file_filter_new ();
			char *Pattern = Token;
			char *PatternSave = NULL;
			char *OnePattern;
			gtk_file_filter_set_name ( GtkFilter, Name );
			for ( OnePattern = strtok_r ( Pattern, ";", &PatternSave ); OnePattern != NULL; OnePattern = strtok_r ( NULL, ";", &PatternSave ) )
				gtk_file_filter_add_pattern ( GtkFilter, OnePattern );
			gtk_file_chooser_add_filter ( Chooser, GtkFilter );
			Name = NULL;
			}
		}
	free ( Copy );
	}

char *cuiFileDialog ( cuiWidget *Parent, const char *Title, const char *Filter, const bool Save, const bool Folder )
	{
	GtkWindow *ParentWindow = NULL;
	GtkFileChooserAction Action = GTK_FILE_CHOOSER_ACTION_OPEN;
	GtkWidget *Dialog;
	char *Result = NULL;
	const char *Accept = "_Open";

	if ( ( Parent != NULL ) && ( Parent->Type == cuiType_Window ) )
		ParentWindow = GTK_WINDOW ( Parent->NativeHandle );
	if ( Folder )
		{
		Action = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;
		Accept = "_Select";
		}
	else if ( Save )
		{
		Action = GTK_FILE_CHOOSER_ACTION_SAVE;
		Accept = "_Save";
		}

	Dialog = gtk_file_chooser_dialog_new ( ( Title != NULL ) ? Title : "", ParentWindow, Action,
	                                       "_Cancel", GTK_RESPONSE_CANCEL,
	                                       Accept, GTK_RESPONSE_ACCEPT,
	                                       NULL );
	if ( Save )
		gtk_file_chooser_set_do_overwrite_confirmation ( GTK_FILE_CHOOSER ( Dialog ), TRUE );
	ApplyFileFilters ( GTK_FILE_CHOOSER ( Dialog ), Filter );
	if ( gtk_dialog_run ( GTK_DIALOG ( Dialog ) ) == GTK_RESPONSE_ACCEPT )
		{
		char *Filename = gtk_file_chooser_get_filename ( GTK_FILE_CHOOSER ( Dialog ) );
		Result = strdup ( Filename );
		g_free ( Filename );
		}
	gtk_widget_destroy ( Dialog );
	return Result;
	}
