#pragma once
#include <stdint.h>
#include "motor_model.h"
#include "transport.h"

typedef enum {
    POLL_OK = 0,
    POLL_ERR_BUILD_REQ,
    POLL_ERR_TXRX,
    POLL_ERR_PARSE,
    POLL_ERR_MODEL,
} poll_status_t;

poll_status_t poll_motor(transport_t *tr, uint8_t slave_addr, motor_telemetry_t *out);
