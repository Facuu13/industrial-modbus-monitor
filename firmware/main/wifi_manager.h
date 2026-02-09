#pragma once
#include "esp_err.h"
#include <stdbool.h>

esp_err_t wifi_manager_init_sta(const char *ssid, const char *pass);

// Bloquea hasta estar conectado y con IP (timeout_ms). true si OK.
bool wifi_manager_wait_connected(int timeout_ms);
