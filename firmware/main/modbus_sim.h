#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Simula un slave Modbus RTU que responde a 0x03.
// Devuelve respuesta RTU en out_resp.
bool modbus_sim_handle_request(const uint8_t *req,
                               size_t req_len,
                               uint8_t *out_resp,
                               size_t out_resp_len,
                               size_t *out_resp_used);
