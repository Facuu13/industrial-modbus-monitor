#include "modbus_poll.h"
#include "modbus_rtu.h"
#include "modbus_sim.h"

poll_status_t poll_motor_sim(uint8_t slave_addr, motor_telemetry_t *out)
{
    if (!out) return POLL_ERR_MODEL;

    // 1) build request (leer 4 regs desde 0)
    uint8_t req[8];
    size_t req_used = 0;
    if (!modbus_rtu_build_read_holding(slave_addr, 0x0000, 4, req, sizeof(req), &req_used)) {
        return POLL_ERR_BUILD_REQ;
    }

    // 2) transport (sim)
    uint8_t resp[64];
    size_t resp_used = 0;
    if (!modbus_sim_handle_request(req, req_used, resp, sizeof(resp), &resp_used)) {
        return POLL_ERR_SIM;
    }

    // 3) parse
    uint16_t regs[8];
    size_t regs_used = 0;
    if (!modbus_rtu_parse_read_holding_resp(resp, resp_used, slave_addr, regs, 8, &regs_used)) {
        return POLL_ERR_PARSE;
    }

    // 4) model
    if (!motor_model_from_regs(regs, (uint16_t)regs_used, out)) {
        return POLL_ERR_MODEL;
    }

    return POLL_OK;
}
