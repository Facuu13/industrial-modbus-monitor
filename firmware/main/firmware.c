#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "rs485.h"
#include "modbus_crc.h"
#include "modbus_rtu.h"
#include "modbus_sim.h"

static const char *TAG = "main";

// ⚠️ Elegí pines que tengas disponibles en tu ESP32-C3 devkit
#define UART_NUM   1
#define GPIO_TX    4
#define GPIO_RX    5
#define GPIO_DERE  6

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
    ESP_LOGI(TAG, "Etapa 1.0 - RS485 raw");

    modbus_crc_selftest();

    rs485_cfg_t cfg = {
        .uart_num = UART_NUM,
        .tx_gpio = GPIO_TX,
        .rx_gpio = GPIO_RX,
        .de_re_gpio = GPIO_DERE,
        .baudrate = 9600,
    };
    ESP_ERROR_CHECK(rs485_init(&cfg));

    // Enviar bytes de prueba (no Modbus todavía)
    const uint8_t test_frame[] = {0xAA, 0x55, 0x01, 0x02, 0x03};

    while (1) {
        ESP_LOGI(TAG, "TX raw frame...");
        rs485_write(test_frame, sizeof(test_frame));

        uint8_t rxbuf[128];
        size_t n = 0;
        rs485_read(rxbuf, sizeof(rxbuf), 200, &n);

        if (n > 0) {
            printf("RX (%u): ", (unsigned)n);
            for (size_t i = 0; i < n; i++) printf("%02X ", rxbuf[i]);
            printf("\n");
        } else {
            ESP_LOGI(TAG, "RX: (no data)");
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
