#pragma once

#include <pebble.h>

#include "../windows/main_window.h"

#define COMM_SIZE_INBOX          256
#define COMM_SIZE_OUTBOX         COMM_SIZE_INBOX
#define COMM_NUM_PACKET_ELEMENTS 4

void comm_init();

bool comm_is_busy();

bool comm_send_data(AccelData *data, uint32_t num_samples);
