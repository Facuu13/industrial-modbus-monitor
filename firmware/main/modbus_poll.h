#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "motor_model.h"

typedef enum {
    POLL_OK = 0,
    POLL_ERR_BUILD_REQ,
    POLL_ERR_SIM,
    POLL_ERR_PARSE,
    POLL_ERR_MODEL,
} poll_status_t;

// Hace un ciclo completo: request -> transport(sim) -> parse -> model
poll_status_t poll_motor_sim(uint8_t slave_addr, motor_telemetry_t *out);
