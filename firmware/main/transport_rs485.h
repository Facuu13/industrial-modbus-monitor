#pragma once
#include "transport.h"
#include "rs485.h"

transport_t transport_rs485_create(const rs485_cfg_t *cfg);
