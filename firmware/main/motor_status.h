#pragma once
#include <stdint.h>
#include "motor_model.h"

typedef enum {
    MOTOR_OK = 0,
    MOTOR_WARN,
    MOTOR_CRIT
} motor_level_t;

enum {
    FLG_COMM_LOSS  = 1u << 0,
    FLG_UNDERVOLT  = 1u << 1,
    FLG_OVERVOLT   = 1u << 2,
    FLG_OVERCURR   = 1u << 3,
    FLG_OVERTEMP   = 1u << 4,
};

typedef struct {
    motor_level_t level;
    uint32_t flags;
} motor_status_t;

motor_status_t motor_eval_status(const motor_telemetry_t *t);
const char* motor_level_str(motor_level_t lvl);
