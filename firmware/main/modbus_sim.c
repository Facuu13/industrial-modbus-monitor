#include "modbus_sim.h"
#include "modbus_crc.h"
#include <string.h>

static uint16_t holding_regs[32] = {
    // 0..31 (hardcodeado)
    2300,  512,  1450,  123,  // ejemplo: volt*10, current*100, rpm, temp*10
    0,0,0,0,
};

static bool crc_ok(const uint8_t *frame, size_t len)
{
    if (!frame || len < 4) return false;
    uint16_t crc_calc = modbus_crc16(frame, len - 2);
    uint16_t crc_rx = (uint16_t)frame[len - 2] | ((uint16_t)frame[len - 1] << 8);
    return crc_calc == crc_rx;
}

bool modbus_sim_handle_request(const uint8_t *req,
                               size_t req_len,
                               uint8_t *out_resp,
                               size_t out_resp_len,
                               size_t *out_resp_used)
{
    if (!req || !out_resp || !out_resp_used) return false;
    *out_resp_used = 0;

    if (req_len != 8) return false;          // request 0x03 fijo = 8 bytes
    if (!crc_ok(req, req_len)) return false;

    uint8_t slave = req[0];
    uint8_t func  = req[1];
    if (func != 0x03) return false;

    uint16_t start = ((uint16_t)req[2] << 8) | req[3];
    uint16_t qty   = ((uint16_t)req[4] << 8) | req[5];
    if (qty == 0 || qty > 125) return false;

    if ((size_t)(start + qty) > (sizeof(holding_regs)/sizeof(holding_regs[0]))) {
        // Exception: illegal data address (0x02)
        if (out_resp_len < 5) return false;
        out_resp[0] = slave;
        out_resp[1] = (uint8_t)(func | 0x80);
        out_resp[2] = 0x02;

        uint16_t crc = modbus_crc16(out_resp, 3);
        out_resp[3] = (uint8_t)(crc & 0xFF);
        out_resp[4] = (uint8_t)(crc >> 8);
        *out_resp_used = 5;
        return true;
    }

    uint8_t byte_count = (uint8_t)(qty * 2);
    size_t resp_len = 3 + byte_count + 2;
    if (out_resp_len < resp_len) return false;

    out_resp[0] = slave;
    out_resp[1] = 0x03;
    out_resp[2] = byte_count;

    for (uint16_t i = 0; i < qty; i++) {
        uint16_t v = holding_regs[start + i];
        out_resp[3 + i*2]     = (uint8_t)(v >> 8);
        out_resp[3 + i*2 + 1] = (uint8_t)(v & 0xFF);
    }

    uint16_t crc = modbus_crc16(out_resp, 3 + byte_count);
    out_resp[3 + byte_count] = (uint8_t)(crc & 0xFF);
    out_resp[4 + byte_count] = (uint8_t)(crc >> 8);

    *out_resp_used = resp_len;
    return true;
}
