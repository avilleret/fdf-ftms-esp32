#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"

#include "usb_host_handler.h"
#include "fdf_protocol.h"
#include "ble_ftms.h"

static const char *TAG = "FDF_BRIDGE";

// Global data callback to bridge USB data to protocol parser
static void usb_data_received(const uint8_t *data, size_t length)
{
    ESP_LOGD(TAG, "Received %d bytes from USB", length);
    fdf_protocol_process_data(data, length);
}

// Global callback to bridge protocol data to FTMS
static void fdf_data_updated(const fdf_rowing_data_t *data)
{
    ESP_LOGI(TAG, "Rowing data updated - Strokes: %" PRIu16 ", Distance: %" PRIu32 " m, Rate: %" PRIu16 " spm, Power: %" PRIu16 " W", 
             data->stroke_count, data->distance_m, data->stroke_rate, data->power_watts);
    
    ble_ftms_update_data(data);
}

void app_main(void)
{
    ESP_LOGI(TAG, "FDF Bluetooth Bridge starting...");

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize FDF protocol parser
    if (!fdf_protocol_init()) {
        ESP_LOGE(TAG, "Failed to initialize FDF protocol parser");
        return;
    }
    fdf_protocol_register_callback(fdf_data_updated);

    // Initialize Bluetooth FTMS service
    if (!ble_ftms_init()) {
        ESP_LOGE(TAG, "Failed to initialize Bluetooth FTMS service");
        return;
    }

    // Initialize USB host
    if (!usb_host_init()) {
        ESP_LOGE(TAG, "Failed to initialize USB host");
        return;
    }
    usb_host_register_data_callback(usb_data_received);

    ESP_LOGI(TAG, "FDF Bluetooth Bridge initialized successfully");
    ESP_LOGI(TAG, "Connect your FDF console via USB and pair with 'FDF Rower' device");

    // Start advertising
    ble_ftms_start_advertising();

    // Main loop - monitor system status
    while (1) {
        // Check USB connection status
        if (!usb_host_is_connected()) {
            ESP_LOGW(TAG, "FDF console disconnected");
        }

        // Check Bluetooth connection status
        if (!ble_ftms_is_connected()) {
            ESP_LOGW(TAG, "No Bluetooth clients connected");
        }

        vTaskDelay(pdMS_TO_TICKS(5000)); // Check every 5 seconds
    }
}
