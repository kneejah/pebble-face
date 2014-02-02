#include <pebble.h>

#define PEBBLE_WIDTH 144
#define PEBBLE_HEIGHT 168
	
Window *window;
TextLayer *text_layer;
InverterLayer *inv_layer;

char buffer[] = "00:00:00 AM";

void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    strftime(buffer, sizeof("00:00:00 am"), "%l:%M:%S %P", tick_time);
 
    text_layer_set_text(text_layer, buffer);
}

void window_load(Window *win) {
	text_layer = text_layer_create(GRect(0, 53, 132, 168));
	text_layer_set_background_color(text_layer, GColorClear);
	text_layer_set_text_color(text_layer, GColorBlack);
	text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
	text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
	
	layer_add_child(window_get_root_layer(window), (Layer*) text_layer);
	
	struct tm *t;
	time_t temp;
	temp = time(NULL);
	t = localtime(&temp);
 
	tick_handler(t, MINUTE_UNIT);
}

void window_unload(Window *win) {
	tick_timer_service_unsubscribe();
}

void handle_init(void) {

	window = window_create();
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
    });
	
	window_stack_push(window, true);
	
	tick_timer_service_subscribe(SECOND_UNIT, (TickHandler) tick_handler);
}

void handle_deinit(void) {
	text_layer_destroy(text_layer);
	window_destroy(window);
}

int main(void) {
	  handle_init();
	  app_event_loop();
	  handle_deinit();
}
