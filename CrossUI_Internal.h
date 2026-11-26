#pragma once
#include <stdbool.h>
#include "CrossUI.h"
#include <Platform/PointerArray.h>

typedef struct cuiWidget
	{
	cuiWidgetType Type;
	char *Text;
	int X, Y;
	unsigned Width, Height;

	bool Visible;
	bool Enabled;
	bool BeingDestroyed;

	void *NativeHandle;
	void *NativeLayoutHandle;
	void *TextAreaBuffer;
	void *TextAreaBufferView;
	void *ScrollHandle;
	cuiWidget *Parent;
	PointerArray Children;

	cuiWidgetCallbacks Callbacks;

	/*
	void *UserData;
	char **Items;
	unsigned ItemCount;
	unsigned ItemCapacity;
	int ProgressValue;
	int SelectedIndex;
	bool Checked;
	void *OnClickUserData;
	cuiCallback OnChange;
	void *OnChangeUserData;
	void *OnCloseUserData;*/
	} cuiWidget;

extern bool Initialized;

bool cuiBackend_Initialize ( int argc, char *argv[] );
void cuiBackend_Shutdown ( void );

cuiWidget *cuiInternal_CreateWidgetEntry ( cuiWidgetType Type, cuiWidget *ParentWidget, const char *Text, const int X, const int Y, const unsigned Width, const unsigned Height );
void cuiInternal_DestroyWidgetEntry ( cuiWidget *Widget );
void cuiInternal_DestroyAllWidgets ( void );
unsigned cuiInternal_GetWidgetCount ( void );
const char *Stringify_cuiWidgetType ( const cuiWidgetType Value );

#define EMPTY_STRING_IF_NULL(STRING) (STRING==NULL?"":STRING)
