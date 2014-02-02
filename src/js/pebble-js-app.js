var COLOR_SCHEME_BLACK      = 0,
	COLOR_SCHEME_WHITE      = 1,
	DISPLAY_SECONDS_OFF     = 0,
	DISPLAY_SECONDS_ON      = 1,
	DISPLAY_BATTERY_OFF     = 0,
	DISPLAY_BATTERY_ON      = 1,
	DISPLAY_BLUETOOTH_OFF   = 0,
	DISPLAY_BLUETOOTH_ON    = 1,
	DISPLAY_TRANSITIONS_OFF = 0,
	DISPLAY_TRANSITIONS_ON  = 1;

var config = {
	color_scheme        : COLOR_SCHEME_BLACK,
	display_seconds     : DISPLAY_SECONDS_OFF,
	display_battery     : DISPLAY_BATTERY_OFF,
	display_bluetooth   : DISPLAY_BLUETOOTH_OFF,
	display_transitions : DISPLAY_TRANSITIONS_ON
};

var settings_name = "simple_clock_settings_v1";
var send_in_progress = false;

function send_config() {
    if (send_in_progress == true) {
        return console.log("Sending config already in progress.");
    }
	
	send_in_progress = true;
	console.log("Sending config: " + JSON.stringify(config)); 

    Pebble.sendAppMessage(config,
        function ack(e) {
			console.log("Successfully delivered message: " + JSON.stringify(e.data));
            send_in_progress = false;
        },
        function nack(e) {
			console.log("Unable to deliver message: " + JSON.stringify(e));
            send_in_progress = false;
        });
}

Pebble.addEventListener("ready", function() {
	var json = window.localStorage.getItem(settings_name);
	console.log("In ready event.");
	
	if (typeof json === 'string') {
		config = JSON.parse(json);
		console.log("Loaded config: " + JSON.stringify(config));
    }
});

Pebble.addEventListener("showConfiguration", function() {
	console.log("In showConfiguration.");
	var url = 'http://subtly.me/pebble/simple-clock.html';
	if (config) {
		url = url + "#" + encodeURIComponent(JSON.stringify(config));
	}
	console.log("Opening url: " + url);
	Pebble.openURL(url);
});

Pebble.addEventListener("webviewclosed", function(e) {
	var json = decodeURIComponent(e.response);
	config = JSON.parse(json);
	
	console.log("Storing config: " + json);
	window.localStorage.setItem(settings_name, json);
	
	send_config();
});