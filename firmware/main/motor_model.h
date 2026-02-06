#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    // “físicas”
    float voltage_v;   // V
    float current_a;   // A
    float temp_c;      // °C
    uint16_t rpm;      // RPM

    // meta
    uint32_t uptime_s;
    bool comm_ok;
} motor_telemetry_t;

bool motor_model_from_regs(const uint16_t *regs, uint16_t n_regs, motor_telemetry_t *out);
