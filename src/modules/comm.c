#include "comm.h"

static bool s_busy;

static void out_failed_handler(DictionaryIterator *iter, AppMessageResult result, void *context) {
  APP_LOG(APP_LOG_LEVEL_WARNING, "out_failed_handler %d", (int)result);
}

static void outbox_sent_handler(DictionaryIterator *iterator, void *context) {
  s_busy = false;
}

void comm_init() {
  app_message_register_outbox_sent(outbox_sent_handler);
  app_message_register_outbox_failed(out_failed_handler);
  app_comm_set_sniff_interval(SNIFF_INTERVAL_REDUCED);
  app_message_open(COMM_SIZE_INBOX, COMM_SIZE_OUTBOX);
}

bool comm_is_busy() {
  return s_busy;
}

bool comm_send_data(AccelData *data, uint32_t num_samples) {
  DictionaryIterator *out;
  AppMessageResult result = app_message_outbox_begin(&out);
  if(result == APP_MSG_OK) {
    for(uint32_t i = 0; i < num_samples; i++) {
      DictionaryResult dict_result;
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Timestamp: %ju", data[i].timestamp);
      dict_result = dict_write_uint32(out, (COMM_NUM_PACKET_ELEMENTS * i) + 0, data[i].timestamp/1000);
      if(dict_result != DICT_OK) {
        APP_LOG(APP_LOG_LEVEL_WARNING, "dict_write_int16 failed on item %lu", (long)i);
      }
      dict_result = dict_write_uint16(out, (COMM_NUM_PACKET_ELEMENTS * i) + 0, data[i].timestamp%1000);
      if(dict_result != DICT_OK) {
        APP_LOG(APP_LOG_LEVEL_WARNING, "dict_write_int16 failed on item %lu", (long)i);
      }
      dict_result = dict_write_int16(out, (COMM_NUM_PACKET_ELEMENTS * i) + 1, data[i].x);
      if(dict_result != DICT_OK) {
        APP_LOG(APP_LOG_LEVEL_WARNING, "dict_write_int16 failed on item %lu", (long)i);
      }
      dict_result = dict_write_int16(out, (COMM_NUM_PACKET_ELEMENTS * i) + 2, data[i].y);
      if(dict_result != DICT_OK) {
        APP_LOG(APP_LOG_LEVEL_WARNING, "dict_write_int16 failed on item %lu", (long)i);
      }
      dict_result = dict_write_int16(out, (COMM_NUM_PACKET_ELEMENTS * i) + 3, data[i].z);
      if(dict_result != DICT_OK) {
        APP_LOG(APP_LOG_LEVEL_WARNING, "dict_write_int16 failed on item %lu", (long)i);
      }
    }

    if(app_message_outbox_send() == APP_MSG_OK) {
      s_busy = true;
      return true;
    } else {
      APP_LOG(APP_LOG_LEVEL_WARNING, "AppMessage send failed.");
      return false;
    }
  } else {
    APP_LOG(APP_LOG_LEVEL_WARNING, "AppMessage not ready: %d", (int)result);
    return false;
  }
}
