#include "GTK3Backend.h"
#include "GTK3Slider.h"
#include <Platform/Platform.h>
#include <Platform/Logger.h>
#include <stdlib.h>
#include <gtk/gtk.h>

static void OnSliderChangedValue ( GtkRange *Range, gpointer Data )
	{
	cuiWidget *Widget = ( cuiWidget * ) Data;
	Widget->SliderData.Value = gtk_range_get_value ( Range );
	if ( Widget->Callbacks.SliderChangedValue )
		Widget->Callbacks.SliderChangedValue ( cuiInternal_WidgetToHandle ( Widget ), Widget->SliderData.Value );
	}

bool cuiBackend_Slider_Create ( cuiWidget *Widget )
	{
	Widget->NativeHandle = NULL;
	Widget->NativeHandle = gtk_scale_new_with_range ( GTK_ORIENTATION_HORIZONTAL, Widget->SliderData.Min, Widget->SliderData.Max, 1 );
	gtk_range_set_value ( GTK_RANGE ( Widget->NativeHandle ), Widget->SliderData.Value );
	gtk_scale_set_draw_value ( GTK_SCALE ( Widget->NativeHandle ), TRUE );
	g_signal_connect ( Widget->NativeHandle, "value-changed", G_CALLBACK ( OnSliderChangedValue ), Widget );
	return true;
	}

void cuiBackend_Slider_SetRange ( cuiWidget *Widget, const float Min, const float Max )
	{
	GtkWidget *Native = GetInnermostWidget ( Widget );
	ASSERT_FAIL ( Widget->Type == cuiType_Slider );
	gtk_range_set_range ( GTK_RANGE ( Native ), Min, Max );
	}

void cuiBackend_Slider_SetValue ( cuiWidget *Widget, const float Value )
	{
	GtkWidget *Native = GetInnermostWidget ( Widget );
	ASSERT_FAIL ( Widget->Type == cuiType_Slider );
	gtk_range_set_value ( GTK_RANGE ( Native ), Value );
	}

float cuiBackend_Slider_GetValue ( cuiWidget *Widget )
	{
	GtkWidget *Native = GetInnermostWidget ( Widget );
	ASSERT_FAIL ( Widget->Type == cuiType_Slider );
	return gtk_range_get_value ( GTK_RANGE ( Native ) );
	}

