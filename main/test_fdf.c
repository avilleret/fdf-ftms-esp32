#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "fdf_protocol.h"

static const char *TAG = "FDF_TEST";

// Test data callback
static void test_data_callback(const fdf_rowing_data_t *data)
{
    ESP_LOGI(TAG, "Test Data Update:");
    ESP_LOGI(TAG, "  Strokes: %d", data->stroke_count);
    ESP_LOGI(TAG, "  Time: %d ms", data->elapsed_time_ms);
    ESP_LOGI(TAG, "  Distance: %d m", data->distance_m);
    ESP_LOGI(TAG, "  Stroke Rate: %d spm", data->stroke_rate);
    ESP_LOGI(TAG, "  Power: %d W", data->power_watts);
    ESP_LOGI(TAG, "  Calories: %d", data->calories);
    ESP_LOGI(TAG, "  Session Active: %s", data->session_active ? "Yes" : "No");
}

void test_fdf_protocol(void)
{
    ESP_LOGI(TAG, "Testing FDF Protocol Parser...");
    
    // Initialize protocol parser
    if (!fdf_protocol_init()) {
        ESP_LOGE(TAG, "Failed to initialize FDF protocol");
        return;
    }
    
    // Register test callback
    fdf_protocol_register_callback(test_data_callback);
    
    // Test data samples (simulating FDF console output)
    const char* test_lines[] = {
        "STROKES:0 TIME:00:00 DISTANCE:0 RATE:0 POWER:0 CALORIES:0",
        "STROKES:1 TIME:00:05 DISTANCE:25 RATE:12 POWER:80 CALORIES:2",
        "STROKES:5 TIME:00:25 DISTANCE:125 RATE:15 POWER:120 CALORIES:8",
        "STROKES:10 TIME:00:50 DISTANCE:250 RATE:18 POWER:150 CALORIES:15",
        "STROKES:20 TIME:01:40 DISTANCE:500 RATE:20 POWER:180 CALORIES:30"
    };
    
    // Process test data
    for (int i = 0; i < sizeof(test_lines) / sizeof(test_lines[0]); i++) {
        ESP_LOGI(TAG, "Processing test line %d: %s", i + 1, test_lines[i]);
        fdf_protocol_process_data((const uint8_t*)test_lines[i], strlen(test_lines[i]));
        vTaskDelay(pdMS_TO_TICKS(1000)); // Wait 1 second between updates
    }
    
    ESP_LOGI(TAG, "FDF Protocol test completed");
}
