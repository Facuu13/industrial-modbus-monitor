#include "motor_status.h"
#include "sdkconfig.h"


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

        // Voltage
    if (t->voltage_v < (float)CONFIG_THRESH_UNDERVOLT_V) {
        s.flags |= FLG_UNDERVOLT;
        s.level = max_level(s.level, MOTOR_WARN);
    }
    if (t->voltage_v > (float)CONFIG_THRESH_OVERVOLT_V) {
        s.flags |= FLG_OVERVOLT;
        s.level = max_level(s.level, MOTOR_WARN);
    }

    // Current thresholds en A
    float oc_warn = CONFIG_THRESH_OVERCURR_WARN_A_X100 / 100.0f;
    float oc_crit = CONFIG_THRESH_OVERCURR_CRIT_A_X100 / 100.0f;

    if (t->current_a > oc_warn) {
        s.flags |= FLG_OVERCURR;
        s.level = max_level(s.level, MOTOR_WARN);
    }
    if (t->current_a > oc_crit) {
        s.flags |= FLG_OVERCURR;
        s.level = max_level(s.level, MOTOR_CRIT);
    }

    // Temperature thresholds en C
    float ot_warn = CONFIG_THRESH_OVERTEMP_WARN_C_X10 / 10.0f;
    float ot_crit = CONFIG_THRESH_OVERTEMP_CRIT_C_X10 / 10.0f;

    if (t->temp_c > ot_warn) {
        s.flags |= FLG_OVERTEMP;
        s.level = max_level(s.level, MOTOR_WARN);
    }
    if (t->temp_c > ot_crit) {
        s.flags |= FLG_OVERTEMP;
        s.level = max_level(s.level, MOTOR_CRIT);
    }


    return s;
}
