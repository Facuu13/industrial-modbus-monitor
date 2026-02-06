#include "transport_rs485.h"
#include <string.h>

typedef struct {
    rs485_cfg_t cfg;
    bool inited;
} rs485_ctx_t;

static rs485_ctx_t s_ctx;

static bool rs485_txrx(transport_t *t,
                       const uint8_t *tx, size_t tx_len,
                       uint8_t *rx, size_t rx_max,
                       size_t *rx_len,
                       int timeout_ms)
{
    (void)t;
    if (!s_ctx.inited) return false;

    if (rs485_write(tx, tx_len) != ESP_OK) {
        if (rx_len) *rx_len = 0;
        return false;
    }

    size_t n = 0;
    if (rs485_read(rx, rx_max, timeout_ms, &n) != ESP_OK) {
        if (rx_len) *rx_len = 0;
        return false;
    }

    if (rx_len) *rx_len = n;
    return (n > 0);
}

transport_t transport_rs485_create(const rs485_cfg_t *cfg)
{
    transport_t t = {.txrx = NULL, .ctx = NULL};

    if (!cfg) return t;

    s_ctx.cfg = *cfg;
    if (rs485_init(&s_ctx.cfg) == ESP_OK) {
        s_ctx.inited = true;
        t.txrx = rs485_txrx;
        t.ctx = &s_ctx;
    }

    return t;
}
