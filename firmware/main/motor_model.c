#include "motor_model.h"
#include "esp_timer.h"

static uint32_t uptime_s(void)
{
    return (uint32_t)(esp_timer_get_time() / 1000000ULL);
}

bool motor_model_from_regs(const uint16_t *regs, uint16_t n_regs, motor_telemetry_t *out)
{
    if (!regs || !out) return false;
    if (n_regs < 4) return false;

    // Mapeo definido por vos (simulado)
    // reg0: volt*10, reg1: current*100, reg2: rpm, reg3: temp*10
    out->voltage_v = regs[0] / 10.0f;
    out->current_a = regs[1] / 100.0f;
    out->rpm       = regs[2];
    out->temp_c    = regs[3] / 10.0f;

    out->uptime_s  = uptime_s();
    out->comm_ok   = true;

    return true;
}
