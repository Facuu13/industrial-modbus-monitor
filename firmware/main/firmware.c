#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "rs485.h"
#include "modbus_crc.h"
#include "modbus_rtu.h"
#include "modbus_sim.h"

#include "modbus_poll.h"

#include "transport_sim.h"

#include "motor_status.h"




static const char *TAG = "main";

// ⚠️ Elegí pines que tengas disponibles en tu ESP32-C3 devkit
#define UART_NUM   1
#define GPIO_TX    4
#define GPIO_RX    5
#define GPIO_DERE  6

#define COMM_FAIL_THRESHOLD 3
static int s_comm_fail_streak = 0;


static void modbus_crc_selftest(void)
{
    // Test clásico Modbus: 01 03 00 00 00 0A -> CRC esperado en frame: C5 CD
    // Como entero (high<<8|low) se imprime como 0xCDC5
    const uint8_t test[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x0A};
    uint16_t crc = modbus_crc16(test, sizeof(test));
    printf("CRC16 = 0x%04X (esperado 0xCDC5)\n", crc);

    uint8_t req[8];
    size_t req_used = 0;

    bool ok = modbus_rtu_build_read_holding(0x01, 0x0000, 4, req, sizeof(req), &req_used);
    printf("build req ok=%d len=%u\n", ok, (unsigned)req_used);

    uint8_t resp[64];
    size_t resp_used = 0;

    ok = modbus_sim_handle_request(req, req_used, resp, sizeof(resp), &resp_used);
    printf("sim resp ok=%d len=%u\n", ok, (unsigned)resp_used);

    uint16_t regs[16];
    size_t regs_used = 0;

    ok = modbus_rtu_parse_read_holding_resp(resp, resp_used, 0x01, regs, 16, &regs_used);
    printf("parse ok=%d regs=%u\n", ok, (unsigned)regs_used);

    for (size_t i = 0; i < regs_used; i++) {
        printf("reg[%u]=%u\n", (unsigned)i, (unsigned)regs[i]);
}

}


void app_main(void)
{
    ESP_LOGI(TAG, "Etapa 2.2 - Modelo + Poll (sim)");

    transport_t tr = transport_sim_create();

    while (1) {
        motor_telemetry_t t = {0};
        poll_status_t st = poll_motor(&tr, 0x01, &t);

        if (st == POLL_OK) {
            s_comm_fail_streak = 0;
            t.comm_ok = true;
        } else {
            s_comm_fail_streak++;
            t.comm_ok = (s_comm_fail_streak < COMM_FAIL_THRESHOLD);
        }

        // Evaluar estado SIEMPRE (incluso en falla)
        motor_status_t ms = motor_eval_status(&t);

        if (st == POLL_OK) {
            printf("[POLL_OK] up=%us V=%.1f A=%.2f rpm=%u temp=%.1fC | status=%s flags=0x%08X\n",
                (unsigned)t.uptime_s, t.voltage_v, t.current_a, (unsigned)t.rpm, t.temp_c,
                motor_level_str(ms.level), (unsigned)ms.flags);
        } else {
            printf("[POLL_ERR] st=%d fail_streak=%d | status=%s flags=0x%08X\n",
                (int)st, s_comm_fail_streak, motor_level_str(ms.level), (unsigned)ms.flags);
        }



        vTaskDelay(pdMS_TO_TICKS(1000));
    }

}

