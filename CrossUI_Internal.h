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

	union
		{
		struct
			{
			float Min, Max, Value;
			} SliderData;
		struct
			{
			bool Checked;
			} CheckBoxData;
		struct
			{
			float Value;
			} ProgressBarData;

		};

	/*

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

void cuiInternal_SetWidgetText ( cuiWidget *Widget, const char *Text );
void cuiInternal_GetText ( cuiWidget *Widget, char *Buffer, const unsigned BufferSize );

cuiWidget *cuiInternal_CreateWidgetEntry ( cuiWidgetType Type, cuiWidget *ParentWidget, const char *Text, const int X, const int Y, const unsigned Width, const unsigned Height );
void cuiInternal_DestroyWidgetEntry ( cuiWidget *Widget );
void cuiInternal_DestroyAllWidgets ( void );
unsigned cuiInternal_GetWidgetCount ( void );
cuiHandle cuiInternal_WidgetToHandle ( const cuiWidget *Widget );
cuiWidget *cuiInternal_HandleToWidget ( const cuiHandle Handle );
const char *Stringify_cuiWidgetType ( const cuiWidgetType Value );
