#include <pebble.h>

#define PEBBLE_WIDTH 144
#define PEBBLE_HEIGHT 168
	
Window *window;
TextLayer *text_layer;
InverterLayer *inv_layer;

char buffer[] = "00:00:00 AM";

char format_12_secs[] = "%l:%M:%S %P";
char format_24_secs[] = "%H:%M:%S";

char format_12[] = "%l:%M %P";
char format_24[] = "%H:%M";

GRect text_box;

void on_animation_stopped(Animation *anim, bool finished, void *context) {
    property_animation_destroy((PropertyAnimation*) anim);
}
 
void animate_layer(Layer *layer, GRect *start, GRect *finish, int duration, int delay) {
    // declare animation
    PropertyAnimation *anim = property_animation_create_layer_frame(layer, start, finish);
     
    // set characteristics
    animation_set_duration((Animation*) anim, duration);
    animation_set_delay((Animation*) anim, delay);
     
    // set stopped handler to free memory
    AnimationHandlers handlers = {
        // the reference to the stopped handler is the only one in the array
        .stopped = (AnimationStoppedHandler) on_animation_stopped
    };
    animation_set_handlers((Animation*) anim, handlers, NULL);
     
    // start animation!
    animation_schedule((Animation*) anim);
}

void animate_left(void) {
    GRect finish = GRect(-144, 69, 144, 168);
    animate_layer(text_layer_get_layer(text_layer), &text_box, &finish, 500, 500);
	
	GRect start = GRect(144, 69, 144, 168);
    animate_layer(text_layer_get_layer(text_layer), &start, &text_box, 500, 1000);
}

void animate_right(void) {
    GRect finish = GRect(144, 69, 144, 168);
    animate_layer(text_layer_get_layer(text_layer), &text_box, &finish, 500, 500);
	
	GRect start = GRect(-144, 69, 144, 168);
    animate_layer(text_layer_get_layer(text_layer), &start, &text_box, 500, 1000);
}

void animate_up(void) {
    GRect finish = GRect(0, -99, 144, 168);
    animate_layer(text_layer_get_layer(text_layer), &text_box, &finish, 500, 500);
	
	GRect start = GRect(0, 237, 144, 168);
    animate_layer(text_layer_get_layer(text_layer), &start, &text_box, 500, 1000);
}

void animate_down(void) {
    GRect finish = GRect(0, 237, 144, 168);
    animate_layer(text_layer_get_layer(text_layer), &text_box, &finish, 500, 500);
	
	GRect start = GRect(0, -99, 144, 168);
    animate_layer(text_layer_get_layer(text_layer), &start, &text_box, 500, 1000);
}

void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	bool use_24_hour = clock_is_24h_style();
	
	if (use_24_hour == true) {
		strftime(buffer, sizeof("00:00:00 am"), format_24, tick_time);
	}
	else {
		strftime(buffer, sizeof("00:00:00 am"), format_12, tick_time);
	}
 	
	// update the text on the screen
	text_layer_set_text(text_layer, buffer);
	
	int seconds = tick_time->tm_sec;
	if (seconds == 59) {
		int direction = rand() % 4;
		
		if (direction == 0) {
			animate_left();
		}
		else if (direction == 1) {
			animate_right();
		}
		else if (direction == 2) {
			animate_up();
		}
		else if (direction == 3) {
			animate_down();
		}
	}
}

void window_load(Window *win) {
	text_layer = text_layer_create(text_box);
	text_layer_set_background_color(text_layer, GColorClear);
	text_layer_set_text_color(text_layer, GColorBlack);
	text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
	text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
	layer_add_child(window_get_root_layer(window), (Layer*) text_layer);
	
	// inverter layer
	inv_layer = inverter_layer_create(GRect(0, 0, PEBBLE_WIDTH, PEBBLE_HEIGHT));
	layer_add_child(window_get_root_layer(window), (Layer*) inv_layer);
	
	// load up a value before the first tick
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
	srand(time(NULL));
	text_box = GRect(0, 69, PEBBLE_WIDTH, PEBBLE_HEIGHT);
	
	window = window_create();
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
    });
	
	window_stack_push(window, true);
	
	tick_timer_service_subscribe(SECOND_UNIT, (TickHandler) tick_handler);
}

void handle_deinit(void) {
	inverter_layer_destroy(inv_layer);
	text_layer_destroy(text_layer);
	window_destroy(window);
}

int main(void) {
	  handle_init();
	  app_event_loop();
	  handle_deinit();
}
