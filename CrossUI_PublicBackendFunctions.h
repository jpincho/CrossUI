#pragma once
#include <stdbool.h>

typedef struct cuiWidget cuiWidget;
#define cuiWidget_Invalid NULL

typedef enum
	{
	cuiType_Dead = -1,
	cuiType_Window = 0,
	cuiType_Button,
	cuiType_CheckBox,
	cuiType_ComboBox,
	cuiType_Slider,
	cuiType_ProgressBar,
	cuiType_Label,
	cuiType_TextArea,
	cuiType_TextBox,
	cuiType_ListBox
	} cuiWidgetType;

// Callbacks
typedef void ( *cuiResizedCallback ) ( const cuiWidget *Widget, const unsigned NewWidth, const unsigned NewHeight );
typedef void ( *cuiMovedCallback ) ( const cuiWidget *Widget, const int X, const int Y );
typedef void ( *cuiDestroyedCallback ) ( const cuiWidget *Widget );
typedef void ( *cuiClickedCallback ) ( const cuiWidget *Widget );
typedef void ( *cuiSliderChangedValue ) ( const cuiWidget *Widget, const float Value );
typedef void ( *cuiCheckBoxChanged ) ( const cuiWidget *Widget, const bool Value );
typedef void ( *cuiComboBoxChanged ) ( const cuiWidget *Widget, const int Value );
typedef void ( *cuiTextBoxChanged ) ( const cuiWidget *Widget );
typedef void ( *cuiTextAreaChanged ) ( const cuiWidget *Widget );
typedef void ( *cuiListBoxSelectionChanged ) ( const cuiWidget *Widget, const int Value );

typedef struct
	{
	cuiResizedCallback Resized;
	cuiMovedCallback Moved;
	cuiDestroyedCallback Destroyed;
	cuiClickedCallback Clicked;
	cuiSliderChangedValue SliderChangedValue;
	cuiCheckBoxChanged CheckChanged;
	cuiComboBoxChanged ComboBoxChanged;
	cuiTextBoxChanged TextBoxChanged;
	cuiTextAreaChanged TextAreaChanged;
	cuiListBoxSelectionChanged ListBoxSelectionChanged;
	} cuiWidgetCallbacks;

const char *cuiGetBackendName ( void );
bool cuiUpdate ( const bool Wait );

void cuiSetWidgetText ( cuiWidget *Widget, const char *Text );
void cuiGetWidgetText ( const cuiWidget *Widget, char *Buffer, const unsigned BufferSize );
void cuiSetWidgetBounds ( cuiWidget *Widget, const int X, const int Y, const unsigned Width, const unsigned Height );
bool cuiGetWidgetBounds ( cuiWidget *Widget, int *X, int *Y, unsigned *Width, unsigned *Height );
void cuiSetVisible ( cuiWidget *Widget, const bool Visible );
bool cuiIsVisible ( const cuiWidget *Widget );
void cuiSetEnabled ( cuiWidget *Widget, const bool Enabled );
bool cuiIsEnabled ( const cuiWidget *Widget );
void cuiSetFocus ( const cuiWidget *Widget );

// Specific widget functions
cuiWidget *cuiCreateWindow ( const char *NewTitle, const int NewX, const int NewY, const unsigned NewWidth, const unsigned NewHeight );

cuiWidget *cuiCreateButton ( cuiWidget *ParentWidget, const char *Text, const int X, const int Y, const unsigned Width, const unsigned Height );

cuiWidget *cuiCreateCheckbox ( cuiWidget *ParentWidget, const char *Text, const int X, const int Y, const unsigned Width, const unsigned Height, const bool State );
void cuiSetCheckboxState ( cuiWidget *Widget, const bool State );
bool cuiGetCheckboxState ( const cuiWidget *Widget );

cuiWidget *cuiCreateLabel ( cuiWidget *ParentWidget, const char *Text, const int X, const int Y, const unsigned Width, const unsigned Height );

cuiWidget *cuiCreateProgressBar ( cuiWidget *ParentWidget, const int X, const int Y, const unsigned Width, const unsigned Height );
void cuiSetProgressBarValue ( cuiWidget *Widget, const float Value );

cuiWidget *cuiCreateSlider ( cuiWidget *ParentWidget, const int X, const int Y, const unsigned Width, const unsigned Height, const float MinValue, const float MaxValue, const float Value );
void cuiSetSliderRange ( cuiWidget *Widget, const float MinValue, const float MaxValue );
void cuiSetSliderValue ( cuiWidget *Widget, const float Value );
float cuiGetSliderValue ( const cuiWidget *Widget );

cuiWidget *cuiCreateComboBox ( cuiWidget *ParentWidget, const int X, const int Y, const unsigned Width, const unsigned Height );
void cuiClearComboBox ( const cuiWidget *Widget );
int cuiAddItemToComboBox ( cuiWidget *Widget, const char *Text );
void cuiRemoveItemFromComboBox ( cuiWidget *Widget, const int Index );
void cuiSetSelectedItemInComboBox ( const cuiWidget *Widget, const int Index );
int cuiGetSelectedItemInComboBox ( const cuiWidget *Widget );
int cuiGetEntryCountInComboBox ( const cuiWidget *Widget );

cuiWidget *cuiCreateTextBox ( cuiWidget *ParentWidget, const char *Text, const int X, const int Y, const unsigned Width, const unsigned Height );
cuiWidget *cuiCreateTextArea ( cuiWidget *ParentWidget, const char *Text, const int X, const int Y, const unsigned Width, const unsigned Height );
cuiWidget *cuiCreateListBox ( cuiWidget *ParentWidget, const int X, const int Y, const unsigned Width, const unsigned Height );

void cuiClearListBoxItems ( const cuiWidget *Widget );
int cuiAddItemToListBox ( const cuiWidget *Widget, const char *Text );
void cuiRemoveItemFromListBox ( const cuiWidget *Widget, const int Index );
void cuiSetSelectedItemInListBox ( const cuiWidget *Widget, const int Index );
int cuiGetSelectedItemInListBox ( const cuiWidget *Widget );
int cuiGetEntryCountInListBox ( const cuiWidget *Widget );

/*
GUIHandle cuiCreateListBox ( const GUIHandle Parent, const int X, const int Y, const unsigned Width, const unsigned Height );
GUIHandle cuiCreateTree ( const GUIHandle Parent, const int X, const int Y, const unsigned Width, const unsigned Height );
GUIHandle cuiCreateGroupBox ( const GUIHandle Parent, const char *Text, const int X, const int Y, const unsigned Width, const unsigned Height );
GUIHandle cuiCreatePanel ( const GUIHandle Parent, const int X, const int Y, const unsigned Width, const unsigned Height );
*/