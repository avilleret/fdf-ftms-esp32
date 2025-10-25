#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "usb_host_handler.h"

static const char *TAG = "USB_HOST";

// Data callback function
static usb_data_callback_t data_callback = NULL;

// Connection status
static bool connected = false;

bool usb_host_init(void)
{
    ESP_LOGI(TAG, "USB Host initialized (stub implementation)");
    ESP_LOGI(TAG, "TODO: Implement actual USB host functionality");
    return true;
}

void usb_host_register_data_callback(usb_data_callback_t callback)
{
    data_callback = callback;
    ESP_LOGI(TAG, "Data callback registered");
}

bool usb_host_is_connected(void)
{
    return connected;
}

void usb_host_deinit(void)
{
    ESP_LOGI(TAG, "USB Host deinitialized");
    connected = false;
}

// Function to simulate FDF data for testing
void usb_host_simulate_data(void)
{
    if (data_callback) {
        const char* test_data = "STROKES:10 TIME:01:30 DISTANCE:500 RATE:20 POWER:150 CALORIES:25\n";
        data_callback((const uint8_t*)test_data, strlen(test_data));
    }
}