#pragma once
#include <stdbool.h>
#include "CrossUI.h"

typedef struct cuiWidget cuiWidget;

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
	int SliderMin, SliderMax, SliderValue;
	/*
	void *UserData;
	char **Items;
	unsigned ItemCount;
	unsigned ItemCapacity;
	int ProgressValue;
	int SelectedIndex;
	bool Checked;
	cuiCallback OnClick;
	void *OnClickUserData;
	cuiCallback OnChange;
	void *OnChangeUserData;
	cuiCallback OnClose;
	void *OnCloseUserData;*/
	cuiWidget *Previous, *Next;
	} cuiWidget;


cuiWidget *cuiInternal_CreateWidgetEntry ( cuiWidgetType Type, cuiWidget *ParentWidget, const char *Text, const int X, const int Y, const unsigned Width, const unsigned Height );
void cuiInternal_DestroyWidgetEntry ( cuiWidget *Widget );
void cuiInternal_DestroyAllWidgets ( void );
unsigned cuiInternal_GetWidgetCount ( void );
cuiHandle cuiInternal_WidgetToHandle ( const cuiWidget *Widget );
cuiWidget *cuiInternal_HandleToWidget ( const cuiHandle Handle );
const char *Stringify_cuiWidgetType(const cuiWidgetType Value );
