#include <pebble.h>  
#include <stdlib.h>

#define BFSIZE 25 //max 57
static const uint32_t AXIS_LOG_TAG = 0x5; 

static Window *window;

static TextLayer *text_layer;
DictionaryIterator *iter;
Tuple *text_tuple;
char text_buffer[100];
char data_buffer[BFSIZE];
char *p;
char *ctime(const time_t *timep); 
int x[10];
int y[10];
int z[10];

//Define data structure for sending data. 
typedef struct {  
  uint32_t tag; //tag of the log
  DataLoggingSessionRef logging_session; // logging session
  char value[]; //string to write into the log
} AxisData;
static AxisData axis_datas; 

//Writing data to loggign session
static void count_axis(AxisData *axis_data, char v[]) {  
  strcpy(axis_data->value, v);
  data_logging_log(axis_data->logging_session, &axis_data->value,1);
}

//collect data from accelerometer
int i=1;
static void data_handler(AccelData *data, uint32_t num_samples) {

    int16_t angle = atan2_lookup(data[0].z, data[0].x) * 360 / TRIG_MAX_ANGLE - 270; // Calculate arm angle  
    //Change millisecond to readable data time string
    time_t t = data[0].timestamp/1000;
    struct tm * timeinfo=gmtime(&t);
    char timebuffer[BFSIZE];
    char timebuf[15];
    strftime( timebuffer,BFSIZE,"%H:%M:%S %Y/%m/%d", timeinfo); //"%Y/%m/%d %H:%M:%S"
    strftime( timebuf,15,"%Y%m%d%H%M%S", timeinfo); //"%Y/%m/%d %H:%M:%S"
  
    //Text buffer for  showing on the screen
    snprintf(text_buffer, 100, "Time: %s \n \n \n Acc_X: %d mG \n Acc_Y: %d mG \n Acc_Z: %d mG", timebuffer, data[0].x, data[0].y, data[0].z); 
    //snprintf(text_buffer, sizeof(text_buffer), "N X,Y,Z\nxxxxxxxxxxx");
    //snprintf(data_buffer, BFSIZE, "%d,%d,%d,%d,", (int) data[0].timestamp, data[0].x, data[0].y, data[0].z);
    snprintf(data_buffer, BFSIZE, "%lu %d %d %d ", (long) data[0].timestamp, data[0].x, data[0].y, data[0].z);
    //snprintf(text_buffer, 100, "%lu %d %d %d", (long)data[0].timestamp, data[0].x, data[0].y, data[0].z); 
  
    APP_LOG(APP_LOG_LEVEL_DEBUG, data_buffer, window);


  
    AxisData *axis_data0 = &axis_datas;          
    count_axis(axis_data0,data_buffer); //Writing data to logging session  
    text_layer_set_text(text_layer, text_buffer);   
    if (i==300){
      //Finish a logging session and creat a new one
      AxisData *axis_data = &axis_datas;
      data_logging_finish(axis_data->logging_session); //Finish the old session
      axis_data->logging_session = data_logging_create(AXIS_LOG_TAG, DATA_LOGGING_BYTE_ARRAY, BFSIZE, false); //Create a new session every one minute
  
      i=1;
    }else{
      i++;
    }
    
}

static void init_axis_datas(Window *window) { 
    AxisData *axis_data = &axis_datas;   
    axis_data->logging_session = data_logging_create(AXIS_LOG_TAG, DATA_LOGGING_BYTE_ARRAY, BFSIZE, false);   

}

static void deinit_axis_datas(void) {  
  AxisData *axis_data = &axis_datas;
  data_logging_finish(axis_data->logging_session);  
  
}

static void window_load(Window *window) {
  init_axis_datas(window);
  //accel_data_service_subscribe(0, update_ui_from_accel);
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  text_layer = text_layer_create((GRect) { .origin = { 5, 5 }, .size = { bounds.size.w-0, 100 } });
  //timer = app_timer_register(300, timer_callback, NULL);   //Timer for updating watch face and writing data to logging session
  //timerl = app_timer_register(30000, timer_sendlog, NULL);  //Timer for sending logging session every minute
  text_layer_set_overflow_mode(text_layer, GTextOverflowModeWordWrap);
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));    
}

static void window_unload(Window *window) {
 // app_timer_cancel(timer); 
  //app_timer_cancel(timerl); 
  text_layer_destroy(text_layer);
  deinit_axis_datas();
}

static void init(void) {
  window = window_create(); 
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);
   app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  
   // Subscribe to the accelerometer data service
   int num_samples = 1;
   accel_data_service_subscribe(num_samples, data_handler);
   // Choose update rate
   accel_service_set_sampling_rate(ACCEL_SAMPLING_25HZ);
}

static void deinit(void) {
  accel_data_service_unsubscribe();  
}

int main(void) {
  init();
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);
  app_event_loop();
  deinit();
}