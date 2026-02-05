#pragma once
#include <stddef.h>
#include <stdint.h>
#include "esp_err.h"

typedef struct {
    int uart_num;        // UART_NUM_0/1
    int tx_gpio;
    int rx_gpio;
    int de_re_gpio;      // pin que controla DE+RE (unidos)
    int baudrate;        // ej 9600
} rs485_cfg_t;

esp_err_t rs485_init(const rs485_cfg_t *cfg);

// Envía bytes (habilita TX, manda, espera TX done, vuelve a RX)
esp_err_t rs485_write(const uint8_t *data, size_t len);

// Lee bytes disponibles (timeout_ms). Devuelve cantidad leída en *out_len
esp_err_t rs485_read(uint8_t *out, size_t out_max, int timeout_ms, size_t *out_len);
