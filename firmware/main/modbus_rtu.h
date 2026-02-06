#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Construye request RTU para función 0x03 (Read Holding Registers)
// out debe tener >= 8 bytes
bool modbus_rtu_build_read_holding(uint8_t slave_addr,
                                  uint16_t start_addr,
                                  uint16_t quantity,
                                  uint8_t *out,
                                  size_t out_len,
                                  size_t *out_used);

// Parsea respuesta RTU 0x03: devuelve cantidad de registros en out_regs
// out_regs_len = capacidad de out_regs en cantidad de uint16_t
bool modbus_rtu_parse_read_holding_resp(const uint8_t *frame,
                                       size_t len,
                                       uint8_t expected_slave,
                                       uint16_t *out_regs,
                                       size_t out_regs_len,
                                       size_t *out_regs_used);
