#pragma once
#include <stdbool.h>

typedef void *cuiHandle;
#define cuiHandle_Invalid NULL

typedef enum
	{
	cuiType_Dead = -1,
	cuiType_Window = 0,
	cuiType_Label,
	cuiType_Button,
	cuiType_CheckBox,
	cuiType_TextBox,
	cuiType_TextArea,
	cuiType_ComboBox,
	cuiType_ListBox,
	cuiType_Tree,
	cuiType_Slider,
	cuiType_ProgressBar,
	cuiType_GroupBox,
	cuiType_Panel
	} cuiWidgetType;

// Callbacks
typedef void ( *cuiResizedCallback ) ( const cuiHandle WidgetHandle, const unsigned NewWidth, const unsigned NewHeight );
typedef void ( *cuiMovedCallback ) ( const cuiHandle WidgetHandle, const int X, const int Y );
typedef void ( *cuiDestroyedCallback ) ( const cuiHandle WidgetHandle );
typedef void ( *cuiclickedCallback ) ( const cuiHandle WidgetHandle );
typedef struct
	{
	cuiResizedCallback Resized;
	cuiMovedCallback Moved;
	cuiDestroyedCallback Destroyed;
	cuiclickedCallback Clicked;
	} cuiWidgetCallbacks;

bool cuiInitialize ( void );
void cuiShutdown ( void );
const char *cuiGetBackendName ( void );
bool cuiUpdate ( const bool Wait );

cuiHandle cuiCreateWindow ( const char *NewTitle, const int NewX, const int NewY, const unsigned NewWidth, const unsigned NewHeight );
void cuiDestroyWindow ( const cuiHandle WindowHandle );

cuiWidgetCallbacks cuiGetWidgetCallbacks ( const cuiHandle WidgetHandle );
void cuiSetWidgetCallbacks ( const cuiHandle WidgetHandle, cuiWidgetCallbacks Callbacks );

cuiHandle cuiCreateButton ( const cuiHandle ParentHandle, const char *Text, const int X, const int Y, const unsigned Width, const unsigned Height );
#if 0
cuiHandle cuiCreateWindow ( const char *NewTitle, const int NewX, const int NewY, const unsigned NewWidth, const unsigned NewHeight );
void cuiDestroyWindow ( const cuiHandle WindowHandle );
void cuiSetWindowTitle ( const cuiHandle WindowHandle, const char *Title );
unsigned cuiGetWindowCount ( void );
bool cuiUpdate ( void );
#endif