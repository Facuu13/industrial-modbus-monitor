#include "motor_status.h"

static motor_level_t max_level(motor_level_t a, motor_level_t b)
{
    return (a > b) ? a : b;
}

const char* motor_level_str(motor_level_t lvl)
{
    switch (lvl) {
        case MOTOR_OK:   return "OK";
        case MOTOR_WARN: return "WARN";
        case MOTOR_CRIT: return "CRIT";
        default:         return "?";
    }
}

motor_status_t motor_eval_status(const motor_telemetry_t *t)
{
    motor_status_t s = {.level = MOTOR_OK, .flags = 0};
    if (!t) return s;

    if (!t->comm_ok) {
        s.flags |= FLG_COMM_LOSS;
        s.level = MOTOR_CRIT;
        return s;
    }

    // Thresholds (simples para MVP)
    // Voltage
    if (t->voltage_v < 200.0f) { s.flags |= FLG_UNDERVOLT; s.level = max_level(s.level, MOTOR_WARN); }
    if (t->voltage_v > 250.0f) { s.flags |= FLG_OVERVOLT;  s.level = max_level(s.level, MOTOR_WARN); }

    // Current
    if (t->current_a > 12.0f)  { s.flags |= FLG_OVERCURR;  s.level = max_level(s.level, MOTOR_WARN); }
    if (t->current_a > 18.0f)  { s.flags |= FLG_OVERCURR;  s.level = max_level(s.level, MOTOR_CRIT); }

    // Temperature
    if (t->temp_c > 70.0f)     { s.flags |= FLG_OVERTEMP;  s.level = max_level(s.level, MOTOR_WARN); }
    if (t->temp_c > 85.0f)     { s.flags |= FLG_OVERTEMP;  s.level = max_level(s.level, MOTOR_CRIT); }

    return s;
}
