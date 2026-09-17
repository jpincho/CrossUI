#pragma once
#include <stdbool.h>
#include "CrossUI.h"

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
	void *NativeInnerHandle;

	cuiWidget *Parent;
	cuiWidget **Children;
	unsigned ChildCount;
	unsigned ChildCapacity;

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
	cuiWidget *Previous, *Next;
	} cuiWidget;

extern bool Initialized;

bool cuiBackend_Initialize ( void );
void cuiBackend_Shutdown ( void );

cuiWidget *cuiInternal_CreateWidgetEntry ( cuiWidgetType Type, cuiWidget *ParentWidget, const char *Text, const int X, const int Y, const unsigned Width, const unsigned Height );
void cuiInternal_DestroyWidgetEntry ( cuiWidget *Widget );
void cuiInternal_DestroyAllWidgets ( void );
unsigned cuiInternal_GetWidgetCount ( void );
const char *Stringify_cuiWidgetType ( const cuiWidgetType Value );

#define EMPTY_STRING_IF_NULL(STRING) (STRING==NULL?"":STRING)