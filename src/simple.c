#include "pebble.h"

Window *window;

TextLayer *time_layer; 
TextLayer *battery_layer;


//test
void on_animation_stopped(Animation *anim, bool finished, void *context)
{
    //Free the memoery used by the Animation
    property_animation_destroy((PropertyAnimation*) anim);
}
 
void animate_layer(Layer *layer, GRect *start, GRect *finish, int duration, int delay)
{
    //Declare animation
    PropertyAnimation *anim = property_animation_create_layer_frame(layer, start, finish);
 
    //Set characteristics
    animation_set_duration((Animation*) anim, duration);
    animation_set_delay((Animation*) anim, delay);
 
    //Set stopped handler to free memory
    AnimationHandlers handlers = {
        //The reference to the stopped handler is the only one in the array
        .stopped = (AnimationStoppedHandler) on_animation_stopped
    };
    animation_set_handlers((Animation*) anim, handlers, NULL);
 
    //Start animation!
    animation_schedule((Animation*) anim);
}



static void handle_battery(BatteryChargeState charge_state) {
  static char battery_text[] = "100%";

  if (charge_state.is_charging) {
    snprintf(battery_text, sizeof(battery_text), "%d%%", charge_state.charge_percent);
  } else {
    snprintf(battery_text, sizeof(battery_text), "%d%%", charge_state.charge_percent);
  }
  text_layer_set_text(battery_layer, battery_text);
}

// Called once per second
static void handle_second_tick(struct tm* tick_time, TimeUnits units_changed) {
  static char time_text[] = "00:00"; // Needs to be static because it's used by the system later.

  
  strftime(time_text, sizeof(time_text), clock_is_24h_style()?"%T":"%I:%M", tick_time);
  text_layer_set_text(time_layer, time_text);

  handle_battery(battery_state_service_peek());
}

void tap_handler(AccelAxisType accel, int32_t direction) {
  
  GRect start = GRect(110, 134, 144, 34);
  GRect finish = GRect(0, 134, 144, 34);
  animate_layer(text_layer_get_layer(battery_layer), &start, &finish, 200, 500);
  
  GRect start1 = GRect(0, 134, 144, 34);
  GRect finish1 = GRect(110, 134, 144, 34);
  animate_layer(text_layer_get_layer(battery_layer), &start1, &finish1, 200, 1700);
  
}

// Handle the start-up of the app
static void do_init(void) {

  // Create our app's base window
  window = window_create();
  window_stack_push(window, true);
  window_set_background_color(window, GColorBlack);

  Layer *root_layer = window_get_root_layer(window);
  GRect frame = layer_get_frame(root_layer);
  
  

  // Init the text layer used to show the time
  ResHandle font_handle = resource_get_handle(RESOURCE_ID_FONT_OSTRICH_33);
  ResHandle font_handle2 = resource_get_handle(RESOURCE_ID_FONT_OSTRICH_25);
  
  time_layer = text_layer_create(GRect(0, 60, frame.size.w /* width */, 100/* height */));
  text_layer_set_text_color(time_layer, GColorWhite);
  text_layer_set_background_color(time_layer, GColorClear);
  text_layer_set_font(time_layer, fonts_load_custom_font(font_handle));
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);

  battery_layer = text_layer_create(GRect(110, 134, /*width */ frame.size.w, 34 /* height */));
  text_layer_set_text_color(battery_layer, GColorBlack);
  text_layer_set_background_color(battery_layer, GColorWhite);
  text_layer_set_font(battery_layer, fonts_load_custom_font(font_handle2));
  text_layer_set_text_alignment(battery_layer, GTextAlignmentCenter);
  text_layer_set_text(battery_layer, "100%");

  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);
  handle_second_tick(current_time, MINUTE_UNIT);

  tick_timer_service_subscribe(MINUTE_UNIT, &handle_second_tick);
  battery_state_service_subscribe(&handle_battery);

  layer_add_child(root_layer, text_layer_get_layer(time_layer));
  layer_add_child(root_layer, text_layer_get_layer(battery_layer));
  
  accel_tap_service_subscribe(tap_handler);
}

static void do_deinit(void) {
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  text_layer_destroy(time_layer);
  text_layer_destroy(battery_layer);
  window_destroy(window);
  
}

// The main event/run loop for our app
int main(void) {
  do_init();
  app_event_loop();
  do_deinit();
}