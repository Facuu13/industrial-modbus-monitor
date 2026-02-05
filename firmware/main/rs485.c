#include "rs485.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "rs485";

static rs485_cfg_t s_cfg;

static void rs485_set_tx(bool tx_en)
{
    // DE=1 -> driver habilitado (TX), RE normalmente es activo en bajo, pero si los unís:
    // TX: DE/RE=1, RX: DE/RE=0
    gpio_set_level(s_cfg.de_re_gpio, tx_en ? 1 : 0);
}

esp_err_t rs485_init(const rs485_cfg_t *cfg)
{
    if (!cfg) return ESP_ERR_INVALID_ARG;
    s_cfg = *cfg;

    // DE/RE pin
    gpio_config_t io = {
        .pin_bit_mask = 1ULL << s_cfg.de_re_gpio,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = 0,
        .pull_down_en = 0,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&io));
    rs485_set_tx(false); // start in RX

    uart_config_t uart_cfg = {
        .baud_rate = s_cfg.baudrate,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    ESP_ERROR_CHECK(uart_param_config(s_cfg.uart_num, &uart_cfg));
    ESP_ERROR_CHECK(uart_set_pin(s_cfg.uart_num, s_cfg.tx_gpio, s_cfg.rx_gpio, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(s_cfg.uart_num, 1024, 1024, 0, NULL, 0));

    // Importante para half-duplex: vaciar buffers
    uart_flush_input(s_cfg.uart_num);

    ESP_LOGI(TAG, "init ok uart=%d tx=%d rx=%d de_re=%d baud=%d",
             s_cfg.uart_num, s_cfg.tx_gpio, s_cfg.rx_gpio, s_cfg.de_re_gpio, s_cfg.baudrate);

    return ESP_OK;
}

esp_err_t rs485_write(const uint8_t *data, size_t len)
{
    if (!data || len == 0) return ESP_ERR_INVALID_ARG;

    rs485_set_tx(true);

    int written = uart_write_bytes(s_cfg.uart_num, (const char *)data, len);
    if (written < 0) {
        rs485_set_tx(false);
        return ESP_FAIL;
    }

    // Esperar que termine de salir por UART
    ESP_ERROR_CHECK(uart_wait_tx_done(s_cfg.uart_num, pdMS_TO_TICKS(200)));

    rs485_set_tx(false);
    return ESP_OK;
}

esp_err_t rs485_read(uint8_t *out, size_t out_max, int timeout_ms, size_t *out_len)
{
    if (!out || out_max == 0 || !out_len) return ESP_ERR_INVALID_ARG;

    int r = uart_read_bytes(s_cfg.uart_num, out, out_max, pdMS_TO_TICKS(timeout_ms));
    if (r < 0) return ESP_FAIL;

    *out_len = (size_t)r;
    return ESP_OK;
}
