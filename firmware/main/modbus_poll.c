#include "modbus_poll.h"
#include "modbus_rtu.h"

poll_status_t poll_motor(transport_t *tr, uint8_t slave_addr, motor_telemetry_t *out)
{
    if (!tr || !tr->txrx || !out) return POLL_ERR_MODEL;

    uint8_t req[8];
    size_t req_used = 0;
    if (!modbus_rtu_build_read_holding(slave_addr, 0x0000, 4, req, sizeof(req), &req_used)) {
        return POLL_ERR_BUILD_REQ;
    }

    uint8_t resp[128];
    size_t resp_used = 0;
    if (!tr->txrx(tr, req, req_used, resp, sizeof(resp), &resp_used, 300)) {
        return POLL_ERR_TXRX;
    }

    uint16_t regs[8];
    size_t regs_used = 0;
    if (!modbus_rtu_parse_read_holding_resp(resp, resp_used, slave_addr, regs, 8, &regs_used)) {
        return POLL_ERR_PARSE;
    }

    if (!motor_model_from_regs(regs, (uint16_t)regs_used, out)) {
        return POLL_ERR_MODEL;
    }

    return POLL_OK;
}
