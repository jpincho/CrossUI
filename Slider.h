#pragma once
#include "CrossUI.h"

cuiHandle cuiCreateSlider ( const cuiHandle ParentHandle, const int X, const int Y, const unsigned Width, const unsigned Height, const float MinValue, const float MaxValue, const float Value );
void cuiSetSliderValue ( const cuiHandle WidgetHandle, const float Value );
float cuiGetSliderValue ( const cuiHandle WidgetHandle );