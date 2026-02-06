#include "transport_sim.h"
#include "modbus_sim.h"

static bool sim_txrx(transport_t *t,
                     const uint8_t *tx, size_t tx_len,
                     uint8_t *rx, size_t rx_max,
                     size_t *rx_len,
                     int timeout_ms)
{
    (void)t;
    (void)timeout_ms;

    size_t used = 0;
    bool ok = modbus_sim_handle_request(tx, tx_len, rx, rx_max, &used);
    if (!ok) {
        if (rx_len) *rx_len = 0;
        return false;
    }
    if (rx_len) *rx_len = used;
    return true;
}

transport_t transport_sim_create(void)
{
    transport_t t = {
        .txrx = sim_txrx,
        .ctx = NULL,
    };
    return t;
}
