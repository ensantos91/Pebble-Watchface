#include <pebble.h>
  
static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static TextLayer *s_bluetooth_layer;
static TextLayer *s_battery_layer;
static BitmapLayer *s_background_layer;
static BitmapLayer *s_background_night_layer;
static BitmapLayer *s_background_lowBat_layer;
static BitmapLayer *s_background_charge_layer;
static BitmapLayer *s_message_layer;
static GBitmap *s_background_bitmap;
static GBitmap *s_background_night_bitmap;
static GBitmap *s_background_lowBat_bitmap;
static GBitmap *s_background_charge_bitmap;
static GBitmap *s_message_bitmap;
static bool is_night;
static bool is_start;
static bool is_disconnected;
static bool is_lowBat;

// STILL TODO: Make it so percentage and bluetooth disconnect don't show at same time

static void battery_handler(BatteryChargeState charge_state) {
  static char s_battery_buffer[] = "00%";

  if (charge_state.is_charging) {
    
    is_lowBat = false;
    
    // set to charging face
    layer_set_hidden(bitmap_layer_get_layer(s_background_layer), true);
    layer_set_hidden(bitmap_layer_get_layer(s_background_night_layer), true);
    layer_set_hidden(bitmap_layer_get_layer(s_background_lowBat_layer), true);
    layer_set_hidden(bitmap_layer_get_layer(s_background_charge_layer), false);
    
  } else if (charge_state.charge_percent <= 15){
    
    is_lowBat = true;
    
    // set battery percentage and make visable
    snprintf(s_battery_buffer, sizeof(s_battery_buffer), "%d%%", charge_state.charge_percent);
    text_layer_set_text(s_battery_layer, s_battery_buffer);
    if(!is_disconnected){
      layer_set_hidden(text_layer_get_layer(s_battery_layer), false);  
    }
    
    // set to low battery face
    layer_set_hidden(bitmap_layer_get_layer(s_background_layer), true);
    layer_set_hidden(bitmap_layer_get_layer(s_background_night_layer), true);
    layer_set_hidden(bitmap_layer_get_layer(s_background_lowBat_layer), false);
    layer_set_hidden(bitmap_layer_get_layer(s_background_charge_layer), true);
    
  } else{
    
    is_lowBat = false;
    
    if(is_night){
      // set face to night face
      layer_set_hidden(bitmap_layer_get_layer(s_background_layer), true);
      layer_set_hidden(bitmap_layer_get_layer(s_background_night_layer), false);
      layer_set_hidden(bitmap_layer_get_layer(s_background_lowBat_layer), true);
      layer_set_hidden(bitmap_layer_get_layer(s_background_charge_layer), true);
    }
    else{
      // set face to day face
      layer_set_hidden(bitmap_layer_get_layer(s_background_layer), false);
      layer_set_hidden(bitmap_layer_get_layer(s_background_night_layer), true);
      layer_set_hidden(bitmap_layer_get_layer(s_background_lowBat_layer), true);
      layer_set_hidden(bitmap_layer_get_layer(s_background_charge_layer), true);
    }
    bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
    layer_set_hidden(text_layer_get_layer(s_battery_layer), true);
  }
}

static void bt_handler(bool connected) {
  // Show current connection state
  if (connected) {
    // hide message
    layer_set_hidden(bitmap_layer_get_layer(s_message_layer), true);
    layer_set_hidden(text_layer_get_layer(s_bluetooth_layer), true);
    is_disconnected = false;
    
    if(is_lowBat){
      layer_set_hidden(text_layer_get_layer(s_battery_layer), false);
    }
  } else {
    // show message and vibrate
    layer_set_hidden(bitmap_layer_get_layer(s_message_layer), false);
    layer_set_hidden(text_layer_get_layer(s_bluetooth_layer), false);
    is_disconnected = true;
    
    // hide persentage (for when low battery)
    layer_set_hidden(text_layer_get_layer(s_battery_layer), true);
    
    // vibrate if not start up
    if(!is_start){
      vibes_long_pulse();      
    }
  }
}

static void update_time(){
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  // Create a long-lived buffer
  static char buffer[] = "00:00";
  static char dBuffer[] = "Aaa Aaa 00 0000";
  
  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true){
    // Use 24 hour format
    strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  }
  else{
    strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
  }
  
  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, buffer);
  
  // Format date
  strftime(dBuffer, sizeof("Aaa Aaa 00 0000"), "%a %b %d %Y", tick_time);
  text_layer_set_text(s_date_layer, dBuffer);  
  
  // Set GBitMap to day/night
  if(tick_time->tm_hour >= 22 || tick_time->tm_hour <= 5){ 
    layer_set_hidden(bitmap_layer_get_layer(s_background_layer), true);
    layer_set_hidden(bitmap_layer_get_layer(s_background_night_layer), false);
    is_night = true;
  }
  else{
    layer_set_hidden(bitmap_layer_get_layer(s_background_layer), false);
    layer_set_hidden(bitmap_layer_get_layer(s_background_night_layer), true);
    is_night = false;
  }
}

static void main_window_load(Window *window){
  
  // Create GBitmap, then set to created BitmapLayer
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_PIKACHU_1);
  s_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));
  
  // Create GBitmap for Night face and set hidden
  s_background_night_bitmap = gbitmap_create_with_resource(RESOURCE_ID_PIKACHU_SLEEP);
  s_background_night_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(s_background_night_layer, s_background_night_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_night_layer));
  layer_set_hidden(bitmap_layer_get_layer(s_background_night_layer), true);
  
  // Create GBitmap for Low_Bat face and set hidden
  s_background_lowBat_bitmap = gbitmap_create_with_resource(RESOURCE_ID_PIKACHU_LOW_BAT);
  s_background_lowBat_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(s_background_lowBat_layer, s_background_lowBat_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_lowBat_layer));
  layer_set_hidden(bitmap_layer_get_layer(s_background_lowBat_layer), true);
  
  // Create GBitmap for Charging face and set hidden
  s_background_charge_bitmap = gbitmap_create_with_resource(RESOURCE_ID_PIKACHU_SHOCK);
  s_background_charge_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(s_background_charge_layer, s_background_charge_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_charge_layer));
  layer_set_hidden(bitmap_layer_get_layer(s_background_charge_layer), true);
  
  // Create message GBitmap and set hidden
  s_message_bitmap = gbitmap_create_with_resource(RESOURCE_ID_NOTIFICATION);
  s_message_layer = bitmap_layer_create(GRect(0, 147, 144, 21));
  bitmap_layer_set_bitmap(s_message_layer, s_message_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_message_layer));
  layer_set_hidden(bitmap_layer_get_layer(s_message_layer), true);
  
  // Create time TextLayer
  s_time_layer = text_layer_create(GRect(0, 4, 144, 50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  
  // Create date TextLayer
  s_date_layer = text_layer_create(GRect(0, 55, 144, 50));
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorBlack);
  
  // Create bluetooth TextLayer
  s_bluetooth_layer = text_layer_create(GRect(0,148, 144, 50));
  text_layer_set_background_color(s_bluetooth_layer, GColorClear);
  text_layer_set_text_color(s_bluetooth_layer, GColorBlack);
  text_layer_set_text(s_bluetooth_layer, "DISCONNECTED");
  
  // Create battery TextLayer
  s_battery_layer = text_layer_create(GRect(0,148, 144, 50));
  text_layer_set_background_color(s_battery_layer, GColorClear);
  text_layer_set_text_color(s_battery_layer, GColorBlack);
  
  // Improve the time layout to be more like a watchface
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  // Improve the date layout to be more like a watchface
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  
  // Improve the bluetooth layout to be more like a watchface
  text_layer_set_font(s_bluetooth_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
  text_layer_set_text_alignment(s_bluetooth_layer, GTextAlignmentCenter);
  
  // Improve the battery layout to be more like a watchface
  text_layer_set_font(s_battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentCenter);
  
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
  
  // Add bluetooth text to message layer and set hidden
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_bluetooth_layer));
  layer_set_hidden(text_layer_get_layer(s_bluetooth_layer), true);
  
  // Add battery text to window's root layer and set hidden
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_battery_layer));
  layer_set_hidden(text_layer_get_layer(s_battery_layer), true);
  
  // Make sure the time is displayed from the start
  update_time();
  
  // Check bluetooth status on startup
  is_start = true;
  bt_handler(bluetooth_connection_service_peek());
  is_start = false;
  
  // Check battery status on startup
  battery_handler(battery_state_service_peek());
}  

static void main_window_unload(Window *window){
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_bluetooth_layer);
  text_layer_destroy(s_battery_layer);
  
  // Destroy GBitmap
  gbitmap_destroy(s_background_bitmap);
  gbitmap_destroy(s_message_bitmap);
  gbitmap_destroy(s_background_night_bitmap);
  gbitmap_destroy(s_background_charge_bitmap);
  gbitmap_destroy(s_background_lowBat_bitmap);
  
  // Destroy BitmapLayer
  bitmap_layer_destroy(s_background_layer);
  bitmap_layer_destroy(s_message_layer);
  bitmap_layer_destroy(s_background_night_layer);
  bitmap_layer_destroy(s_background_charge_layer);
  bitmap_layer_destroy(s_background_lowBat_layer);
}  

static void tick_handler(struct tm *tick_time, TimeUnits units_changed){
  update_time();
}

static void init(){
  // Set initial value to is_night to false
  is_night = false;
  
  // Set initial value to is_lowBat to false
  is_lowBat = false; // would get overridden latter
  
  // Create main Window element and assign to pointer
  s_main_window = window_create();
  
  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers){
    .load = main_window_load, 
    .unload = main_window_unload
  });
  
  // Show the Window on the watch, with animated = true
  window_stack_push(s_main_window, true);
  
  //Register with TickTimeService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // Subscribe to Bluetooth updates
  bluetooth_connection_service_subscribe(bt_handler);
  
  // Subscribe to Battery updates
  battery_state_service_subscribe(battery_handler);

}

static void deinit(){
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void){
  init();
  app_event_loop();
  deinit();
}