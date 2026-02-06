#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct transport transport_t;

typedef bool (*transport_txrx_fn)(transport_t *t,
                                 const uint8_t *tx, size_t tx_len,
                                 uint8_t *rx, size_t rx_max,
                                 size_t *rx_len,
                                 int timeout_ms);

struct transport {
    transport_txrx_fn txrx;
    void *ctx;
};
