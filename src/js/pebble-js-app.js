var DEBUG_MODE = 0;

function debug_log(msg) {
	if (DEBUG_MODE == 1) {
		console.log(msg);
	}
}

var COLOR_SCHEME_BLACK      = 0,
	COLOR_SCHEME_WHITE      = 1,
	DISPLAY_SECONDS_OFF     = 0,
	DISPLAY_SECONDS_ON      = 1,
	DISPLAY_BATTERY_OFF     = 0,
	DISPLAY_BATTERY_ON      = 1,
	DISPLAY_BLUETOOTH_OFF   = 0,
	DISPLAY_BLUETOOTH_ON    = 1,
	DISPLAY_TRANSITIONS_OFF = 0,
	DISPLAY_TRANSITIONS_ON  = 1,
	ALWAYS_SHOW_INFO_OFF    = 0,
	ALWAYS_SHOW_INFO_ON     = 1;

var version = "1.1";
var config = {};
var default_config = {
	color_scheme        : COLOR_SCHEME_BLACK,
	display_seconds     : DISPLAY_SECONDS_OFF,
	display_battery     : DISPLAY_BATTERY_ON,
	display_bluetooth   : DISPLAY_BLUETOOTH_ON,
	display_transitions : DISPLAY_TRANSITIONS_ON,
	always_show_info    : ALWAYS_SHOW_INFO_OFF
};

var settings_name = "simple_clock_settings_v1";
var send_in_progress = false;

function send_config() {
    if (send_in_progress == true) {
        return debug_log("Sending config already in progress.");
    }
	
	send_in_progress = true;
	debug_log("Sending config: " + JSON.stringify(config)); 

    Pebble.sendAppMessage(config,
        function ack(e) {
			debug_log("Successfully delivered message: " + JSON.stringify(e.data));
            send_in_progress = false;
        },
        function nack(e) {
			debug_log("Unable to deliver message: " + JSON.stringify(e));
            send_in_progress = false;
        });
}

function merge(set1, set2) {
  	for (var key in set2){
    	if (set2.hasOwnProperty(key))
      		set1[key] = set2[key];
  	}
  	return set1
}

Pebble.addEventListener("appmessage",
    function(e) {
		debug_log("Got message: " + JSON.stringify(e.payload));
		if (e.payload.get_config_values && e.payload.get_config_values == 1) {
			send_config();
		}
	}
);

Pebble.addEventListener("ready", function() {
	debug_log("In ready event.");
	var json = window.localStorage.getItem(settings_name);
	
	if (typeof json === 'string') {
		config = merge(default_config, JSON.parse(json));
		debug_log("Loaded config: " + JSON.stringify(config));
    }
});

Pebble.addEventListener("showConfiguration", function() {
	debug_log("In showConfiguration.");

	var url = 'http://subtly.me/pebble/simple-clock-v' + version + '.html';
	
	if (DEBUG_MODE == 1) {
		url = 'http://subtly.me/pebble/simple-clock-v' + version + '_debug.html';
	}
	if (config) {
		url = url + "#" + encodeURIComponent(JSON.stringify(config));
	}
	debug_log("Opening url: " + url);
	Pebble.openURL(url);
});

Pebble.addEventListener("webviewclosed", function(e) {
	var json = decodeURIComponent(e.response);
	config = JSON.parse(json);
	
	debug_log("Storing config: " + json);
	window.localStorage.setItem(settings_name, json);
	
	send_config();
});