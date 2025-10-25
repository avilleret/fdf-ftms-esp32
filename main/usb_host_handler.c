#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_err.h"

// USB Host includes
#include "usb/usb_host.h"
#include "usb/cdc_acm_host.h"

#include "usb_host_handler.h"

static const char *TAG = "USB_HOST";

// USB Host configuration
#define USB_HOST_PRIORITY 20
#define USB_HOST_TASK_STACK_SIZE 4096
#define USB_HOST_EVENT_QUEUE_SIZE 10

// CDC-ACM configuration
#define CDC_ACM_RX_BUFFER_SIZE 1024
#define CDC_ACM_TX_BUFFER_SIZE 1024

// Global variables
static usb_data_callback_t data_callback = NULL;
static usb_host_status_t host_status = USB_HOST_STATUS_DISCONNECTED;
static cdc_acm_dev_hdl_t cdc_acm_device = NULL;
static usb_host_client_handle_t client_handle = NULL;
static TaskHandle_t usb_host_task_handle = NULL;
static QueueHandle_t usb_event_queue = NULL;

// Forward declarations
static void usb_host_task(void *arg);
static void usb_event_callback(const usb_host_client_event_msg_t *event_msg, void *arg);
static bool cdc_acm_data_callback(const uint8_t *data, size_t data_len, void *user_arg);
static void cdc_acm_event_callback(const cdc_acm_host_dev_event_data_t *event, void *user_ctx);

/**
 * @brief USB Host task to handle USB events
 */
static void usb_host_task(void *arg)
{
    usb_host_client_event_msg_t event_msg;
    bool client_gone = false;
    
    ESP_LOGI(TAG, "USB Host task started");
    
    while (1) {
        // Wait for USB events
        if (xQueueReceive(usb_event_queue, &event_msg, portMAX_DELAY) == pdTRUE) {
            switch (event_msg.event) {
                case USB_HOST_CLIENT_EVENT_NEW_DEV:
                    ESP_LOGI(TAG, "New USB device detected");
                    if (event_msg.new_dev.address != 0) {
                        // Configure CDC-ACM device
                        const cdc_acm_host_device_config_t dev_config = {
                            .connection_timeout_ms = 5000,
                            .out_buffer_size = CDC_ACM_TX_BUFFER_SIZE,
                            .in_buffer_size = CDC_ACM_RX_BUFFER_SIZE,
                            .event_cb = cdc_acm_event_callback,
                            .data_cb = cdc_acm_data_callback,
                            .user_arg = NULL,
                        };
                        
                        // Try to open CDC-ACM device (using VID/PID 0 for any CDC-ACM device)
                        esp_err_t ret = cdc_acm_host_open(CDC_HOST_ANY_VID, CDC_HOST_ANY_PID, 0, &dev_config, &cdc_acm_device);
                        if (ret == ESP_OK) {
                            ESP_LOGI(TAG, "CDC-ACM device opened successfully");
                            host_status = USB_HOST_STATUS_CONNECTED;
                        } else {
                            ESP_LOGE(TAG, "Failed to open CDC-ACM device: %s", esp_err_to_name(ret));
                            host_status = USB_HOST_STATUS_ERROR;
                        }
                    }
                    break;
                    
                case USB_HOST_CLIENT_EVENT_DEV_GONE:
                    ESP_LOGI(TAG, "USB device disconnected");
                    if (cdc_acm_device != NULL) {
                        cdc_acm_host_close(cdc_acm_device);
                        cdc_acm_device = NULL;
                    }
                    host_status = USB_HOST_STATUS_DISCONNECTED;
                    client_gone = true;
                    break;
                    
                default:
                    ESP_LOGW(TAG, "Unhandled USB event: %d", event_msg.event);
                    break;
            }
            
            // If client is gone, exit the task
            if (client_gone) {
                break;
            }
        }
    }
    
    ESP_LOGI(TAG, "USB Host task ended");
    vTaskDelete(NULL);
}

/**
 * @brief USB event callback
 */
static void usb_event_callback(const usb_host_client_event_msg_t *event_msg, void *arg)
{
    if (xQueueSend(usb_event_queue, event_msg, 0) != pdTRUE) {
        ESP_LOGW(TAG, "USB event queue full, dropping event");
    }
}

/**
 * @brief CDC-ACM data callback
 */
static bool cdc_acm_data_callback(const uint8_t *data, size_t data_len, void *user_arg)
{
    ESP_LOGD(TAG, "Received %d bytes from CDC-ACM", data_len);
    if (data_callback != NULL) {
        data_callback(data, data_len);
    }
    return true; // Data processed, flush RX buffer
}

/**
 * @brief CDC-ACM event callback
 */
static void cdc_acm_event_callback(const cdc_acm_host_dev_event_data_t *event, void *user_ctx)
{
    switch (event->type) {
        case CDC_ACM_HOST_ERROR:
            ESP_LOGE(TAG, "CDC-ACM error: %d", event->data.error);
            host_status = USB_HOST_STATUS_ERROR;
            break;
            
        case CDC_ACM_HOST_SERIAL_STATE:
            ESP_LOGI(TAG, "CDC-ACM serial state changed");
            break;
            
        case CDC_ACM_HOST_NETWORK_CONNECTION:
            ESP_LOGI(TAG, "CDC-ACM network connection: %s", event->data.network_connected ? "connected" : "disconnected");
            break;
            
        case CDC_ACM_HOST_DEVICE_DISCONNECTED:
            ESP_LOGI(TAG, "CDC-ACM device disconnected");
            cdc_acm_device = NULL;
            host_status = USB_HOST_STATUS_DISCONNECTED;
            break;
            
        default:
            ESP_LOGW(TAG, "Unhandled CDC-ACM event: %d", event->type);
            break;
    }
}

/**
 * @brief Initialize USB host and CDC-ACM driver
 */
esp_err_t usb_host_init(usb_data_callback_t callback)
{
    esp_err_t ret;
    
    ESP_LOGI(TAG, "Initializing USB Host");
    
    // Store callback
    data_callback = callback;
    
    // Create event queue
    usb_event_queue = xQueueCreate(USB_HOST_EVENT_QUEUE_SIZE, sizeof(usb_host_client_event_msg_t));
    if (usb_event_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create USB event queue");
        return ESP_ERR_NO_MEM;
    }
    
    // Initialize USB Host
    const usb_host_config_t host_config = {
        .intr_flags = ESP_INTR_FLAG_LEVEL1,
    };
    
    ret = usb_host_install(&host_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to install USB host: %s", esp_err_to_name(ret));
        vQueueDelete(usb_event_queue);
        return ret;
    }
    
    // Install USB Host client
    const usb_host_client_config_t client_config = {
        .is_synchronous = false,
        .max_num_event_msg = USB_HOST_EVENT_QUEUE_SIZE,
        .async = {
            .client_event_callback = usb_event_callback,
            .callback_arg = NULL
        }
    };
    
    ret = usb_host_client_register(&client_config, &client_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register USB host client: %s", esp_err_to_name(ret));
        usb_host_uninstall();
        vQueueDelete(usb_event_queue);
        return ret;
    }
    
    // Initialize CDC-ACM host
    const cdc_acm_host_driver_config_t acm_config = {
        .driver_task_stack_size = 4096,
        .driver_task_priority = 5,
        .xCoreID = 0,
        .new_dev_cb = NULL,
    };
    
    ret = cdc_acm_host_install(&acm_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to install CDC-ACM host: %s", esp_err_to_name(ret));
        usb_host_client_deregister(client_handle);
        usb_host_uninstall();
        vQueueDelete(usb_event_queue);
        return ret;
    }
    
    // Create USB host task
    BaseType_t task_ret = xTaskCreate(usb_host_task, "usb_host_task", 
                                     USB_HOST_TASK_STACK_SIZE, NULL, 
                                     USB_HOST_PRIORITY, &usb_host_task_handle);
    if (task_ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create USB host task");
        cdc_acm_host_uninstall();
        usb_host_client_deregister(client_handle);
        usb_host_uninstall();
        vQueueDelete(usb_event_queue);
        return ESP_ERR_NO_MEM;
    }
    
    // Start handling USB events
    ret = usb_host_client_handle_events(client_handle, portMAX_DELAY);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to handle USB events: %s", esp_err_to_name(ret));
    }
    
    ESP_LOGI(TAG, "USB Host initialized successfully");
    return ESP_OK;
}

/**
 * @brief Check if FDF console is connected
 */
bool usb_host_is_connected(void)
{
    return (host_status == USB_HOST_STATUS_CONNECTED) && (cdc_acm_device != NULL);
}

/**
 * @brief Get current USB host status
 */
usb_host_status_t usb_host_get_status(void)
{
    return host_status;
}

/**
 * @brief Send data to USB device
 */
esp_err_t usb_host_send_data(const uint8_t *data, size_t len)
{
    if (cdc_acm_device == NULL) {
        ESP_LOGW(TAG, "CDC-ACM device not connected");
        return ESP_ERR_INVALID_STATE;
    }
    
    esp_err_t ret = cdc_acm_host_data_tx_blocking(cdc_acm_device, data, len, 1000);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send data: %s", esp_err_to_name(ret));
    }
    
    return ret;
}

/**
 * @brief Deinitialize USB host
 */
void usb_host_deinit(void)
{
    ESP_LOGI(TAG, "Deinitializing USB Host");
    
    // Close CDC-ACM device if open
    if (cdc_acm_device != NULL) {
        cdc_acm_host_close(cdc_acm_device);
        cdc_acm_device = NULL;
    }
    
    // Delete USB host task
    if (usb_host_task_handle != NULL) {
        vTaskDelete(usb_host_task_handle);
        usb_host_task_handle = NULL;
    }
    
    // Uninstall CDC-ACM host
    cdc_acm_host_uninstall();
    
    // Deregister USB host client
    if (client_handle != NULL) {
        usb_host_client_deregister(client_handle);
        client_handle = NULL;
    }
    
    // Uninstall USB host
    usb_host_uninstall();
    
    // Delete event queue
    if (usb_event_queue != NULL) {
        vQueueDelete(usb_event_queue);
        usb_event_queue = NULL;
    }
    
    host_status = USB_HOST_STATUS_DISCONNECTED;
    ESP_LOGI(TAG, "USB Host deinitialized");
}