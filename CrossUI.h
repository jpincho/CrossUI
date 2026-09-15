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
typedef void ( *cuiClickedCallback ) ( const cuiHandle WidgetHandle );
typedef void ( *cuiSliderChangedValue ) ( const cuiHandle WidgetHandle, const float Value );
typedef void ( *cuiCheckBoxChanged ) ( const cuiHandle WidgetHandle, const bool Value );

typedef struct
	{
	cuiResizedCallback Resized;
	cuiMovedCallback Moved;
	cuiDestroyedCallback Destroyed;
	cuiClickedCallback Clicked;
	cuiSliderChangedValue SliderChangedValue;
	cuiCheckBoxChanged CheckChanged;
	} cuiWidgetCallbacks;

bool cuiInitialize ( void );
void cuiShutdown ( void );
const char *cuiGetBackendName ( void );
bool cuiUpdate ( const bool Wait );


cuiHandle cuiCreateWidget ( const cuiWidgetType Type, const cuiHandle ParentHandle, const char *Text, const int X, const int Y, const unsigned Width, const unsigned Height );
void cuiDestroyWidget ( const cuiHandle WidgetHandle );

void cuiSetWidgetCallbacks ( const cuiHandle WidgetHandle, cuiWidgetCallbacks Callbacks );
cuiWidgetCallbacks cuiGetWidgetCallbacks ( const cuiHandle WidgetHandle );

void cuiSetWidgetText ( const cuiHandle WidgetHandle, const char *Text );
void cuiGetWidgetText ( const cuiHandle WidgetHandle, char *Buffer, const unsigned BufferSize );

// Specific widget functions
cuiHandle cuiCreateWindow ( const char *NewTitle, const int NewX, const int NewY, const unsigned NewWidth, const unsigned NewHeight );
cuiHandle cuiCreateButton ( const cuiHandle ParentHandle, const char *Text, const int X, const int Y, const unsigned Width, const unsigned Height );
cuiHandle cuiCreateLabel ( const cuiHandle ParentHandle, const char *Text, const int X, const int Y, const unsigned Width, const unsigned Height );
cuiHandle cuiCreateCheckbox ( const cuiHandle ParentHandle, const char *Text, const int X, const int Y, const unsigned Width, const unsigned Height );
cuiHandle cuiCreateTextBox ( const cuiHandle ParentHandle, const char *Text, const int X, const int Y, const unsigned Width, const unsigned Height );
cuiHandle cuiCreateTextArea ( const cuiHandle ParentHandle, const char *Text, const int X, const int Y, const unsigned Width, const unsigned Height );
cuiHandle cuiCreateComboBox ( const cuiHandle ParentHandle, const char *Text, const int X, const int Y, const unsigned Width, const unsigned Height );
cuiHandle cuiCreateListBox ( const cuiHandle ParentHandle, const char *Text, const int X, const int Y, const unsigned Width, const unsigned Height );
cuiHandle cuiCreateTree ( const cuiHandle ParentHandle, const int X, const int Y, const unsigned Width, const unsigned Height );
cuiHandle cuiCreateGroupBox ( const cuiHandle ParentHandle, const char *Text, const int X, const int Y, const unsigned Width, const unsigned Height );
cuiHandle cuiCreatePanel ( const cuiHandle ParentHandle, const char *Text, const int X, const int Y, const unsigned Width, const unsigned Height );

#include "ProgressBar.h"
#include "Slider.h"