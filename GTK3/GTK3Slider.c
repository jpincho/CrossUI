#include "GTK3BackendInternal.h"

static void OnSliderChangedValue ( GtkRange *Range, gpointer Data )
	{
	cuiWidget *Widget = ( cuiWidget * ) Data;
	if ( Widget->Callbacks.SliderChangedValue )
		Widget->Callbacks.SliderChangedValue ( Widget, gtk_range_get_value ( Range ) );
	}

cuiWidget *cuiCreateSlider ( cuiWidget *ParentWidget, const int X, const int Y, const unsigned Width, const unsigned Height, const float MinValue, const float MaxValue, const float Value )
	{
	cuiWidget *Widget = cuiInternal_CreateWidgetEntry ( cuiType_Slider, ParentWidget, NULL, X, Y, Width, Height );
	if ( Widget == NULL )
		return NULL;
	Widget->NativeHandle = gtk_scale_new_with_range ( GTK_ORIENTATION_HORIZONTAL, MIN ( MinValue, MaxValue ), MAX ( MinValue, MaxValue ), 1 );
	gtk_range_set_value ( GTK_RANGE ( Widget->NativeHandle ), Value );
	gtk_scale_set_draw_value ( GTK_SCALE ( Widget->NativeHandle ), TRUE );
	g_signal_connect ( Widget->NativeHandle, "value-changed", G_CALLBACK ( OnSliderChangedValue ), Widget );
	FinishedCreatingNewWidget ( Widget );
	return Widget;
	}

void cuiSetSliderRange ( cuiWidget *Widget, const float MinValue, const float MaxValue )
	{
	ASSERT_FAIL ( Widget != NULL );
	ASSERT_FAIL ( Widget->Type == cuiType_Slider );
	GtkWidget *Native = GetInnermostWidget ( Widget );
	gtk_range_set_range ( GTK_RANGE ( Native ), MIN ( MinValue, MaxValue ), MAX ( MinValue, MaxValue ) );
	}

void cuiSetSliderValue ( cuiWidget *Widget, const float Value )
	{
	ASSERT_FAIL ( Widget != NULL );
	ASSERT_FAIL ( Widget->Type == cuiType_Slider );
	GtkWidget *Native = GetInnermostWidget ( Widget );
	gtk_range_set_value ( GTK_RANGE ( Native ), Value );
	}

float cuiGetSliderValue ( const cuiWidget *Widget )
	{
	ASSERT_FAIL ( Widget != NULL );
	ASSERT_FAIL ( Widget->Type == cuiType_Slider );
	GtkWidget *Native = GetInnermostWidget ( Widget );
	return gtk_range_get_value ( GTK_RANGE ( Native ) );
	}