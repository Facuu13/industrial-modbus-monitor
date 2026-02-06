#include "modbus_rtu.h"
#include "modbus_crc.h"

static bool crc_ok(const uint8_t *frame, size_t len)
{
    if (!frame || len < 4) return false;
    uint16_t crc_calc = modbus_crc16(frame, len - 2);
    uint16_t crc_rx = (uint16_t)frame[len - 2] | ((uint16_t)frame[len - 1] << 8);
    return crc_calc == crc_rx;
}

bool modbus_rtu_build_read_holding(uint8_t slave_addr,
                                  uint16_t start_addr,
                                  uint16_t quantity,
                                  uint8_t *out,
                                  size_t out_len,
                                  size_t *out_used)
{
    if (!out || out_len < 8 || !out_used) return false;
    if (quantity == 0 || quantity > 125) return false; // Modbus limit

    out[0] = slave_addr;
    out[1] = 0x03;
    out[2] = (uint8_t)(start_addr >> 8);
    out[3] = (uint8_t)(start_addr & 0xFF);
    out[4] = (uint8_t)(quantity >> 8);
    out[5] = (uint8_t)(quantity & 0xFF);

    uint16_t crc = modbus_crc16(out, 6);
    out[6] = (uint8_t)(crc & 0xFF);       // CRC low
    out[7] = (uint8_t)(crc >> 8);         // CRC high

    *out_used = 8;
    return true;
}

bool modbus_rtu_parse_read_holding_resp(const uint8_t *frame,
                                       size_t len,
                                       uint8_t expected_slave,
                                       uint16_t *out_regs,
                                       size_t out_regs_len,
                                       size_t *out_regs_used)
{
    if (!frame || len < 5 || !out_regs || !out_regs_used) return false;
    if (!crc_ok(frame, len)) return false;

    uint8_t slave = frame[0];
    uint8_t func  = frame[1];

    if (slave != expected_slave) return false;

    // Exception response: function | 0x80, byte2 = exception code
    if (func & 0x80) return false;

    if (func != 0x03) return false;

    uint8_t byte_count = frame[2];
    if (byte_count % 2 != 0) return false;

    size_t regs = byte_count / 2;
    if (regs > out_regs_len) return false;

    // Frame mínimo esperado: 3 + byte_count + 2(CRC)
    if (len != (size_t)(3 + byte_count + 2)) return false;

    for (size_t i = 0; i < regs; i++) {
        uint8_t hi = frame[3 + i * 2];
        uint8_t lo = frame[3 + i * 2 + 1];
        out_regs[i] = ((uint16_t)hi << 8) | (uint16_t)lo;
    }

    *out_regs_used = regs;
    return true;
}
