#include "Slider.h"
#include "CrossUI_Internal.h"
#include "CrossUI_BackendFunctions.h"
#include <stdlib.h>

cuiHandle cuiCreateSlider ( const cuiHandle ParentHandle, const int X, const int Y, const unsigned Width, const unsigned Height, const float MinValue, const float MaxValue, const float Value )
	{
	cuiWidget *NewWidget = cuiInternal_CreateWidgetEntry ( cuiType_Slider, cuiInternal_HandleToWidget ( ParentHandle ), NULL, X, Y, Width, Height );
	if ( NewWidget == NULL )
		return cuiHandle_Invalid;
	NewWidget->SliderData.Min = MinValue;
	NewWidget->SliderData.Max = MaxValue;
	NewWidget->SliderData.Value = Value;
	if ( cuiBackend_CreateNativeWidget ( NewWidget ) == false )
		{
		free ( NewWidget );
		return cuiHandle_Invalid;
		}
	return cuiInternal_WidgetToHandle ( NewWidget );
	}

void cuiSetSliderValue ( const cuiHandle WidgetHandle, const float Value )
	{
	cuiWidget *Widget = cuiInternal_HandleToWidget ( WidgetHandle );
	if ( Widget == NULL )
		return;
	Widget->SliderData.Value = Value;
	cuiBackend_Slider_SetValue ( Widget, Value );
	}

float cuiGetSliderValue ( const cuiHandle WidgetHandle )
	{
	cuiWidget *Widget = cuiInternal_HandleToWidget ( WidgetHandle );
	if ( Widget == NULL )
		return 0.0f;
	return Widget->SliderData.Value;
	}