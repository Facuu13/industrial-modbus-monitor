#include "mqtt_manager.h"
#include <string.h>

#include "esp_log.h"
#include "mqtt_client.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

static const char *TAG = "mqtt";

static esp_mqtt_client_handle_t s_client = NULL;
static EventGroupHandle_t s_evt = NULL;
static const int MQTT_CONNECTED_BIT = BIT0;

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    (void)handler_args; (void)base; (void)event_id;

    esp_mqtt_event_handle_t e = (esp_mqtt_event_handle_t)event_data;

    switch (e->event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "connected");
        xEventGroupSetBits(s_evt, MQTT_CONNECTED_BIT);
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGW(TAG, "disconnected");
        xEventGroupClearBits(s_evt, MQTT_CONNECTED_BIT);
        break;

    case MQTT_EVENT_ERROR:
        ESP_LOGE(TAG, "event error");
        break;

    default:
        break;
    }
}

esp_err_t mqtt_manager_start(const mqtt_manager_cfg_t *cfg)
{
    if (!cfg || !cfg->broker_uri || !cfg->client_id) return ESP_ERR_INVALID_ARG;

    if (!s_evt) s_evt = xEventGroupCreate();

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = cfg->broker_uri,
        .credentials.client_id = cfg->client_id,
        .network.disable_auto_reconnect = false,
    };

    s_client = esp_mqtt_client_init(&mqtt_cfg);
    if (!s_client) return ESP_FAIL;

    ESP_ERROR_CHECK(esp_mqtt_client_register_event(s_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL));
    esp_err_t err = esp_mqtt_client_start(s_client);

    ESP_LOGI(TAG, "start uri=%s client_id=%s", cfg->broker_uri, cfg->client_id);
    return err;
}

bool mqtt_manager_is_connected(void)
{
    if (!s_evt) return false;
    EventBits_t bits = xEventGroupGetBits(s_evt);
    return (bits & MQTT_CONNECTED_BIT) != 0;
}

esp_err_t mqtt_manager_publish(const char *topic, const char *payload, int qos, int retain)
{
    if (!s_client || !topic || !payload) return ESP_ERR_INVALID_ARG;

    int msg_id = esp_mqtt_client_publish(s_client, topic, payload, 0, qos, retain);
    if (msg_id < 0) return ESP_FAIL;

    return ESP_OK;
}
