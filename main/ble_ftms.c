#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "ble_ftms.h"

static const char *TAG = "BLE_FTMS";

// Current rowing data
static fdf_rowing_data_t current_rowing_data = {0};

bool ble_ftms_init(void)
{
    ESP_LOGI(TAG, "Bluetooth FTMS service initialized (stub implementation)");
    ESP_LOGI(TAG, "TODO: Implement actual Bluetooth FTMS functionality");
    return true;
}

void ble_ftms_update_data(const fdf_rowing_data_t *data)
{
    if (!data) {
        return;
    }
    
    // Store current data
    memcpy(&current_rowing_data, data, sizeof(fdf_rowing_data_t));
    
    ESP_LOGI(TAG, "FTMS data updated - Strokes: %" PRIu16 ", Distance: %" PRIu32 " m, Rate: %" PRIu16 " spm, Power: %" PRIu16 " W", 
             data->stroke_count, data->distance_m, data->stroke_rate, data->power_watts);
}

bool ble_ftms_is_connected(void)
{
    return false; // Stub implementation
}

void ble_ftms_start_advertising(void)
{
    ESP_LOGI(TAG, "Starting advertising (stub implementation)");
    ESP_LOGI(TAG, "TODO: Implement actual Bluetooth advertising");
}

void ble_ftms_stop_advertising(void)
{
    ESP_LOGI(TAG, "Stopping advertising (stub implementation)");
}

void ble_ftms_deinit(void)
{
    ESP_LOGI(TAG, "Bluetooth FTMS service deinitialized (stub implementation)");
}