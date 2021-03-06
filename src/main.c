#include <pebble.h>

#define DEBUG_MODE 0
	
#define PEBBLE_WIDTH  144
#define PEBBLE_HEIGHT 168

#define COLOR_SCHEME        1
#define DISPLAY_SECONDS     2
#define DISPLAY_BATTERY     3
#define DISPLAY_BLUETOOTH   4
#define DISPLAY_TRANSITIONS 5
#define ALWAYS_SHOW_INFO    6
#define SHOW_DATE_FORMAT    7
#define GET_CONFIG_VALUES   100

Window *window;
TextLayer *time_layer;
TextLayer *date_layer;
TextLayer *battery_layer;
TextLayer *bluetooth_layer;
InverterLayer *inv_layer;

char format_12_secs[] = "%l:%M:%S %P";
char format_24_secs[] = "%H:%M:%S";

char format_12[] = "%l:%M %P";
char format_24[] = "%H:%M";

char time_buffer[] = "00:00:00 AM";
char date_buffer[] = "00/00/0000";
char batt_buffer[] = "100%+";

char date_format_1[] = "%m/%d/%Y";
char date_format_2[] = "%d/%m/%Y";
char date_format_3[] = "%Y/%m/%d";

// defaults, same as in the js side of things
static int color_scheme        = 0;
static int display_seconds     = 0;
static int display_battery     = 1;
static int display_bluetooth   = 1;
static int display_transitions = 1;
static int always_show_info    = 0;
static int show_date_format    = 0;

GRect text_box;
static int showing_info = 0;

void debug_log(char *data) {
	if (DEBUG_MODE == 1) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, data);
	}
}

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
    animate_layer(text_layer_get_layer(time_layer), &text_box, &finish, 500, 500);
	
	GRect start = GRect(144, 69, 144, 168);
    animate_layer(text_layer_get_layer(time_layer), &start, &text_box, 500, 1000);
}

void animate_right(void) {
    GRect finish = GRect(144, 69, 144, 168);
    animate_layer(text_layer_get_layer(time_layer), &text_box, &finish, 500, 500);
	
	GRect start = GRect(-144, 69, 144, 168);
    animate_layer(text_layer_get_layer(time_layer), &start, &text_box, 500, 1000);
}

void animate_up(void) {
    GRect finish = GRect(0, -99, 144, 168);
    animate_layer(text_layer_get_layer(time_layer), &text_box, &finish, 500, 500);
	
	GRect start = GRect(0, 237, 144, 168);
    animate_layer(text_layer_get_layer(time_layer), &start, &text_box, 500, 1000);
}

void animate_down(void) {
    GRect finish = GRect(0, 237, 144, 168);
    animate_layer(text_layer_get_layer(time_layer), &text_box, &finish, 500, 500);
	
	GRect start = GRect(0, -99, 144, 168);
    animate_layer(text_layer_get_layer(time_layer), &start, &text_box, 500, 1000);
}

void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	bool use_24_hour = clock_is_24h_style();
	
	if (display_seconds == 1) {
		if (use_24_hour == true) {
			strftime(time_buffer, sizeof("00:00:00"), format_24_secs, tick_time);
		}
		else {
			strftime(time_buffer, sizeof("00:00:00 AM"), format_12_secs, tick_time);
		}
	}
	else if (display_seconds == 0) {
		if (use_24_hour == true) {
			strftime(time_buffer, sizeof("00:00"), format_24, tick_time);
		}
		else {
			strftime(time_buffer, sizeof("00:00 AM"), format_12, tick_time);
		}
	}
 	
	// update the time text on the screen
	text_layer_set_text(time_layer, time_buffer);
	
	// update the date text on the screen
	if (show_date_format == 0) {
		strftime(date_buffer, sizeof("00/00/0000"), date_format_1, tick_time);
	}
	else if (show_date_format == 1) {
		strftime(date_buffer, sizeof("00/00/0000"), date_format_2, tick_time);
	}
	else if (show_date_format == 2) {
		strftime(date_buffer, sizeof("00/00/0000"), date_format_3, tick_time);
	}
	
	text_layer_set_text(date_layer, date_buffer);
	
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

void update_inverter_layer(void) {
	debug_log("In update_inverter_layer().");
	if (color_scheme == 0) {
		layer_set_hidden(inverter_layer_get_layer(inv_layer), false);
	}
	else if (color_scheme == 1) {
		layer_set_hidden(inverter_layer_get_layer(inv_layer), true);
	}
}

void update_battery_layer(BatteryChargeState charge_state) {
	debug_log("In update_battery_layer().");
	if (display_battery == 1) {
		snprintf(batt_buffer, sizeof(batt_buffer), "%d%%%s", charge_state.charge_percent, charge_state.is_charging ? "+" : "");
		text_layer_set_text(battery_layer, batt_buffer);
		layer_set_hidden(text_layer_get_layer(battery_layer), false);
	}
	else if (display_battery == 0) {
		layer_set_hidden(text_layer_get_layer(battery_layer), true);
	}
}

void update_bluetooth_layer(bool connected) {
	debug_log("In update_bluetooth_layer().");
	if (display_bluetooth == 1) {
		text_layer_set_text(bluetooth_layer, connected ? "BT" : "--");
		layer_set_hidden(text_layer_get_layer(bluetooth_layer), false);
	}
	else if (display_bluetooth == 0) {
		layer_set_hidden(text_layer_get_layer(bluetooth_layer), true);
	}
}

void hide_info_layers(void *data) {
	debug_log("In hide_info_layers().");
	
	// hide the date layer
	GRect start = GRect(0, 143,           PEBBLE_WIDTH, PEBBLE_HEIGHT);
	GRect end   = GRect(0, PEBBLE_HEIGHT, PEBBLE_WIDTH, PEBBLE_HEIGHT);
    animate_layer(text_layer_get_layer(date_layer), &start, &end, 500, 0);
	
	// hide the battery layer
	GRect start2 = GRect(5,   0, 50, 50);
	GRect end2   = GRect(5, -50, 50, 50);
    animate_layer(text_layer_get_layer(battery_layer), &start2, &end2, 500, 0);
	
	// hide the bluetooth layer
	GRect start3 = GRect(89,   0, 50, 50);
	GRect end3   = GRect(89, -50, 50, 50);
    animate_layer(text_layer_get_layer(bluetooth_layer), &start3, &end3, 500, 0);
	
	showing_info = 0;
}

void show_info_layers(void) {
	debug_log("In showing_info_layers().");
	
	// show the date layer
	GRect start = GRect(0, PEBBLE_HEIGHT, PEBBLE_WIDTH, PEBBLE_HEIGHT);
	GRect end   = GRect(0,           143, PEBBLE_WIDTH, PEBBLE_HEIGHT);
    animate_layer(text_layer_get_layer(date_layer), &start, &end, 500, 0);
	
	// show the battery layer
	GRect start2 = GRect(5, -50, 50, 50);
	GRect end2   = GRect(5,   0, 50, 50);
    animate_layer(text_layer_get_layer(battery_layer), &start2, &end2, 500, 0);
	
	// show the bluetooth layer
	GRect start3 = GRect(89, -50, 50, 50);
	GRect end3   = GRect(89,   0, 50, 50);
    animate_layer(text_layer_get_layer(bluetooth_layer), &start3, &end3, 500, 0);
	
	// in 3 seconds, make the layers slide away if it's not pinned
	if (always_show_info == 0) {
		app_timer_register(3000, hide_info_layers, NULL);
	}
}

void accel_tap_handler(AccelAxisType axis, int32_t direction) {
	debug_log("In accel_tap_handler().");
	
	// don't do anything if it's always going to show the info
	if (always_show_info == 1) {
		return;
	}
	
	if (showing_info == 1) {
		debug_log("Not reshowing info layers.");
		return;
	}
	showing_info = 1;
	
	show_info_layers();
}

void window_load(Window *win) {
	
	// set up time layer
	time_layer = text_layer_create(text_box);
	text_layer_set_background_color(time_layer, GColorClear);
	text_layer_set_text_color(time_layer, GColorBlack);
	text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
	text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
	layer_add_child(window_get_root_layer(window), (Layer*) time_layer);
	
	// set up date layer
	GRect date_box = GRect(0, PEBBLE_HEIGHT, PEBBLE_WIDTH, PEBBLE_HEIGHT);
	date_layer = text_layer_create(date_box);
	text_layer_set_background_color(date_layer, GColorClear);
	text_layer_set_text_color(date_layer, GColorBlack);
	text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);
	text_layer_set_font(date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
	layer_add_child(window_get_root_layer(window), (Layer*) date_layer);
	
	// set up battery layer
	GRect battery_box = GRect(5, -50, 50, 50);
	battery_layer = text_layer_create(battery_box);
	text_layer_set_background_color(battery_layer, GColorClear);
	text_layer_set_text_color(battery_layer, GColorBlack);
	text_layer_set_text_alignment(battery_layer, GTextAlignmentLeft);
	text_layer_set_font(battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
	layer_add_child(window_get_root_layer(window), (Layer*) battery_layer);
	
	// set up bluetooth layer
	GRect bluetooth_box = GRect(89, -50, 50, 50);
	bluetooth_layer = text_layer_create(bluetooth_box);
	text_layer_set_background_color(bluetooth_layer, GColorClear);
	text_layer_set_text_color(bluetooth_layer, GColorBlack);
	text_layer_set_text_alignment(bluetooth_layer, GTextAlignmentRight);
	text_layer_set_font(bluetooth_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
	layer_add_child(window_get_root_layer(window), (Layer*) bluetooth_layer);
	
	// set up inverter layer
	inv_layer = inverter_layer_create(GRect(0, 0, PEBBLE_WIDTH, PEBBLE_HEIGHT));
	layer_add_child(window_get_root_layer(window), (Layer*) inv_layer);
	
	update_battery_layer(battery_state_service_peek());
	update_bluetooth_layer(bluetooth_connection_service_peek());
	update_inverter_layer();
	
	// load up a value before the first tick
	time_t temp = time(NULL);
	struct tm *t = localtime(&temp);
	tick_handler(t, SECOND_UNIT);
	
	if (always_show_info == 1) {
		show_info_layers();
	} 
}

void window_unload(Window *win) {
	tick_timer_service_unsubscribe();
}

void request_config_values(void *data) {
	debug_log("In request_config_values().");
	
	Tuplet request_tuple = TupletInteger(GET_CONFIG_VALUES, 1);
	
  	DictionaryIterator *iter;
  	app_message_outbox_begin(&iter);
  	if (!iter) return;
	
  	dict_write_tuplet(iter, &request_tuple);
  	dict_write_end(iter);
  	app_message_outbox_send();
}

// pulls in any persisted configs, if they exist
void setup_config(void) {
	debug_log("In setup_config().");
	
	bool has_config_values = true;
	
	if (persist_exists(COLOR_SCHEME)) {
		color_scheme = persist_read_int(COLOR_SCHEME);
	}
	else {
		has_config_values = false;
	}
	
	if (persist_exists(DISPLAY_SECONDS)) {
		display_seconds = persist_read_int(DISPLAY_SECONDS);
	}
	else {
		has_config_values = false;
	}
	
	if (persist_exists(DISPLAY_BATTERY)) {
		display_battery = persist_read_int(DISPLAY_BATTERY);
	}
	else {
		has_config_values = false;
	}
	if (persist_exists(DISPLAY_BLUETOOTH)) {
		display_bluetooth = persist_read_int(DISPLAY_BLUETOOTH);
	}
	else {
		has_config_values = false;
	}
	
	if (persist_exists(DISPLAY_TRANSITIONS)) {
		display_transitions = persist_read_int(DISPLAY_TRANSITIONS);
	}
	else {
		has_config_values = false;
	}
	
	if (persist_exists(ALWAYS_SHOW_INFO)) {
		always_show_info = persist_read_int(ALWAYS_SHOW_INFO);
	}
	else {
		has_config_values = false;
	}
	
	if (persist_exists(SHOW_DATE_FORMAT)) {
		show_date_format = persist_read_int(SHOW_DATE_FORMAT);
	}
	else {
		has_config_values = false;
	}
	
	if (has_config_values == false) {
		app_timer_register(1000, request_config_values, NULL);
	}
}

// callback that gets fired on message recieved from phone
void appmessage_callback(DictionaryIterator *received, void *context) {
	debug_log("In appmessage_callback().");
	
	int old_always_show_info = always_show_info;
	
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
			case ALWAYS_SHOW_INFO:
        		always_show_info = tuple->value->int32;
				persist_write_int(ALWAYS_SHOW_INFO, always_show_info);
        		break;
			case SHOW_DATE_FORMAT:
        		show_date_format = tuple->value->int32;
				persist_write_int(SHOW_DATE_FORMAT, show_date_format);
        		break;
        }
        tuple = dict_read_next(received);
    }
	
	// tick, in case the display seconds toggle was changed
	time_t temp = time(NULL);
	struct tm *t = localtime(&temp);
	tick_handler(t, SECOND_UNIT);
	
	update_battery_layer(battery_state_service_peek());
	update_bluetooth_layer(bluetooth_connection_service_peek());
	update_inverter_layer();
	
	if (old_always_show_info != always_show_info) {
		if (always_show_info == 1) {
			show_info_layers();
		}
		else if (always_show_info == 0) {
			hide_info_layers(NULL);
		}
	}
}

void handle_init(void) {
	srand(time(NULL));
	text_box = GRect(0, 69, PEBBLE_WIDTH, PEBBLE_HEIGHT);
	
	// get config values first, so things are set up right when they get displayed
	setup_config();
	
	// set up listener for messages from the phone
	app_message_register_inbox_received(&appmessage_callback);
  	app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
	
	window = window_create();
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
    });
	window_stack_push(window, true);
	
	battery_state_service_subscribe(&update_battery_layer);
  	update_battery_layer(battery_state_service_peek());
	
	bluetooth_connection_service_subscribe(&update_bluetooth_layer);
  	update_bluetooth_layer(bluetooth_connection_service_peek());
	
	accel_tap_service_subscribe(&accel_tap_handler);
	
	update_inverter_layer();
	
	tick_timer_service_subscribe(SECOND_UNIT, (TickHandler) tick_handler);
}

void handle_deinit(void) {
	battery_state_service_unsubscribe();
	bluetooth_connection_service_unsubscribe();
	accel_tap_service_unsubscribe();
	
	inverter_layer_destroy(inv_layer);
	text_layer_destroy(bluetooth_layer);
	text_layer_destroy(battery_layer);
	text_layer_destroy(date_layer);
	text_layer_destroy(time_layer);
	window_destroy(window);
}

int main(void) {
	  handle_init();
	  app_event_loop();
	  handle_deinit();
}
