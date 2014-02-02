#include <pebble.h>

#define PEBBLE_WIDTH  144
#define PEBBLE_HEIGHT 168

#define COLOR_SCHEME        1
#define DISPLAY_SECONDS     2
#define DISPLAY_BATTERY     3
#define DISPLAY_BLUETOOTH   4
#define DISPLAY_TRANSITIONS 5
	
Window *window;
TextLayer *text_layer;
InverterLayer *inv_layer;

char buffer[] = "00:00:00 AM";

char format_12_secs[] = "%l:%M:%S %P";
char format_24_secs[] = "%H:%M:%S";

char format_12[] = "%l:%M %P";
char format_24[] = "%H:%M";

// defaults, same as in the js side of things
static int color_scheme        = 0;
static int display_seconds     = 0;
static int display_battery     = 0;
static int display_bluetooth   = 0;
static int display_transitions = 1;

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
	
	if (display_seconds == 1) {
		if (use_24_hour == true) {
			strftime(buffer, sizeof("00:00:00"), format_24_secs, tick_time);
		}
		else {
			strftime(buffer, sizeof("00:00:00 AM"), format_12_secs, tick_time);
		}
	}
	else if (display_seconds == 0) {
		if (use_24_hour == true) {
			strftime(buffer, sizeof("00:00"), format_24, tick_time);
		}
		else {
			strftime(buffer, sizeof("00:00 AM"), format_12, tick_time);
		}
	}
 	
	// update the text on the screen
	text_layer_set_text(text_layer, buffer);
	
	int seconds = tick_time->tm_sec;
	if (seconds == 59 && display_transitions == 1) {
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

void handle_inverter_layer(void) {
	if (color_scheme == 0) {
		layer_set_hidden(inverter_layer_get_layer(inv_layer), false);
	}
	else if (color_scheme == 1) {
		layer_set_hidden(inverter_layer_get_layer(inv_layer), true);
	}
}

void window_load(Window *win) {
	
	// set up text layer
	text_layer = text_layer_create(text_box);
	text_layer_set_background_color(text_layer, GColorClear);
	text_layer_set_text_color(text_layer, GColorBlack);
	text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
	text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
	layer_add_child(window_get_root_layer(window), (Layer*) text_layer);
	
	// set up inverter layer
	inv_layer = inverter_layer_create(GRect(0, 0, PEBBLE_WIDTH, PEBBLE_HEIGHT));
	layer_add_child(window_get_root_layer(window), (Layer*) inv_layer);
	
	handle_inverter_layer();
	
	// load up a value before the first tick
	time_t temp = time(NULL);
	struct tm *t = localtime(&temp);
	tick_handler(t, SECOND_UNIT);
}

void window_unload(Window *win) {
	tick_timer_service_unsubscribe();
}

// pulls in any presisted configs, if they exist
void setup_config(void) {
	if (persist_exists(COLOR_SCHEME)) {
		color_scheme = persist_read_int(COLOR_SCHEME);
	}
	if (persist_exists(DISPLAY_SECONDS)) {
		display_seconds = persist_read_int(DISPLAY_SECONDS);
	}
	if (persist_exists(DISPLAY_BATTERY)) {
		display_battery = persist_read_int(DISPLAY_BATTERY);
	}
	if (persist_exists(DISPLAY_BLUETOOTH)) {
		display_bluetooth = persist_read_int(DISPLAY_BLUETOOTH);
	}
	if (persist_exists(DISPLAY_TRANSITIONS)) {
		display_transitions = persist_read_int(DISPLAY_TRANSITIONS);
	}
}

// callback that gets fired on message recieved from phone
void appmessage_callback(DictionaryIterator *received, void *context) {
	Tuple *tuple = dict_read_first(received);
  	while (tuple) {
    	switch (tuple->key) {
      		case COLOR_SCHEME:
        		color_scheme = tuple->value->int32;
				persist_write_int(COLOR_SCHEME, color_scheme);
        		break;
      		case DISPLAY_SECONDS:
        		display_seconds = tuple->value->int32;
				persist_write_int(DISPLAY_SECONDS, display_seconds);
        		break;
		    case DISPLAY_BATTERY:
        		display_battery = tuple->value->int32;
				persist_write_int(DISPLAY_BATTERY, display_battery);
        		break;
			case DISPLAY_BLUETOOTH:
        		display_bluetooth = tuple->value->int32;
				persist_write_int(DISPLAY_BLUETOOTH, display_bluetooth);
        		break;
			case DISPLAY_TRANSITIONS:
        		display_transitions = tuple->value->int32;
				persist_write_int(DISPLAY_TRANSITIONS, display_transitions);
        		break;
        }
        tuple = dict_read_next(received);
    }
	
	handle_inverter_layer();
}

void handle_init(void) {
	srand(time(NULL));
	text_box = GRect(0, 69, PEBBLE_WIDTH, PEBBLE_HEIGHT);
	
	setup_config();
	
	app_message_register_inbox_received(&appmessage_callback);
  	app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
	
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
