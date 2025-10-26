#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_defs.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_gatt_common_api.h"

#include "ble_ftms.h"

static const char *TAG = "BLE_FTMS";

// Bluetooth connection state
static bool is_connected = false;
static bool bt_initialized = false;

// Current rowing data
static fdf_rowing_data_t current_rowing_data = {0};
static SemaphoreHandle_t data_mutex = NULL;

// GATT interface
static esp_gatt_if_t gatts_if = ESP_GATT_IF_NONE;

/**
 * @brief GAP event handler
 */
static void gap_event_handler(esp_gap_ble_cb_event_t event,
                              esp_ble_gap_cb_param_t *param)
{
    switch (event) {
        case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
            ESP_LOGI(TAG, "Advertisement data set complete");
            break;
        
        case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
            if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
                ESP_LOGE(TAG, "Advertising start failed");
            } else {
                ESP_LOGI(TAG, "Advertising started successfully");
            }
            break;
        
        case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
            ESP_LOGI(TAG, "Advertisement stopped");
            break;
        
        default:
            break;
    }
}

/**
 * @brief GATTS event handler
 */
static void gatts_event_handler(esp_gatts_cb_event_t event,
                                esp_gatt_if_t gatts_if,
                                esp_ble_gatts_cb_param_t *param)
{
    switch (event) {
        case ESP_GATTS_REG_EVT:
            if (param->reg.status == ESP_GATT_OK) {
                gatts_if = gatts_if;
                ESP_LOGI(TAG, "GATTS registered successfully, interface: %d", gatts_if);
            } else {
                ESP_LOGE(TAG, "GATTS registration failed");
            }
            break;
        
        case ESP_GATTS_CONNECT_EVT:
            is_connected = true;
            ESP_LOGI(TAG, "Client connected, conn_id: %d", param->connect.conn_id);
            break;
        
        case ESP_GATTS_DISCONNECT_EVT:
            is_connected = false;
            ESP_LOGI(TAG, "Client disconnected");
            break;
        
        default:
            break;
    }
}

/**
 * @brief Initialize Bluetooth FTMS service
 */
bool ble_ftms_init(void)
{
    esp_err_t ret;
    
    ESP_LOGI(TAG, "Initializing Bluetooth FTMS service");
    
    // Create mutex for data access
    data_mutex = xSemaphoreCreateMutex();
    if (data_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create mutex");
        return false;
    }
    
    // Release classic BT memory for memory optimization
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
    
    // Initialize Bluetooth controller
    ESP_LOGI(TAG, "Initializing Bluetooth controller...");
    
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize Bluetooth controller: %s", esp_err_to_name(ret));
        vSemaphoreDelete(data_mutex);
        return false;
    }
    ESP_LOGI(TAG, "Bluetooth controller initialized successfully");
    
    // Enable Bluetooth controller in BLE mode
    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable Bluetooth controller: %s", esp_err_to_name(ret));
        esp_bt_controller_deinit();
        vSemaphoreDelete(data_mutex);
        return false;
    }
    ESP_LOGI(TAG, "Bluetooth controller enabled in BLE mode");
    
    // Initialize Bluedroid
    ESP_LOGI(TAG, "Initializing Bluedroid...");
    
    ret = esp_bluedroid_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize Bluedroid: %s", esp_err_to_name(ret));
        esp_bt_controller_disable();
        esp_bt_controller_deinit();
        vSemaphoreDelete(data_mutex);
        return false;
    }
    ESP_LOGI(TAG, "Bluedroid initialized successfully");
    
    // Enable Bluedroid
    ret = esp_bluedroid_enable();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable Bluedroid: %s", esp_err_to_name(ret));
        esp_bluedroid_deinit();
        esp_bt_controller_disable();
        esp_bt_controller_deinit();
        vSemaphoreDelete(data_mutex);
        return false;
    }
    ESP_LOGI(TAG, "Bluedroid enabled successfully");
    
    // Register GAP callback
    ESP_LOGI(TAG, "Registering GAP callback...");
    ret = esp_ble_gap_register_callback(gap_event_handler);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register GAP callback: %s", esp_err_to_name(ret));
        esp_bluedroid_disable();
        esp_bluedroid_deinit();
        esp_bt_controller_disable();
        esp_bt_controller_deinit();
        vSemaphoreDelete(data_mutex);
        return false;
    }
    ESP_LOGI(TAG, "GAP callback registered successfully");
    
    // Register GATTS callback
    ESP_LOGI(TAG, "Registering GATTS callback...");
    ret = esp_ble_gatts_register_callback(gatts_event_handler);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register GATTS callback: %s", esp_err_to_name(ret));
        esp_bluedroid_disable();
        esp_bluedroid_deinit();
        esp_bt_controller_disable();
        esp_bt_controller_deinit();
        vSemaphoreDelete(data_mutex);
        return false;
    }
    ESP_LOGI(TAG, "GATTS callback registered successfully");
    
    // TODO: Next steps to complete FTMS:
    // 1. Create application profile and register with esp_ble_gatts_app_register()
    // 2. Create FTMS service (UUID 0x1826) with Indoor Rower Data characteristic (UUID 0x2AD1)
    // 3. Start advertising with esp_ble_gap_start_advertising()
    
    bt_initialized = true;
    ESP_LOGI(TAG, "Bluetooth stack initialized successfully");
    ESP_LOGW(TAG, "TODO: Implement GATT service creation and advertising");
    
    return true;
}

/**
 * @brief Update FTMS data with new rowing metrics
 */
void ble_ftms_update_data(const fdf_rowing_data_t *data)
{
    if (!data || data_mutex == NULL) {
        return;
    }
    
    // Update data with mutex protection
    if (xSemaphoreTake(data_mutex, portMAX_DELAY) == pdTRUE) {
        memcpy(&current_rowing_data, data, sizeof(fdf_rowing_data_t));
        xSemaphoreGive(data_mutex);
        
        ESP_LOGI(TAG, "FTMS data updated - Strokes: %" PRIu16 ", Distance: %" PRIu32 " m, Rate: %" PRIu16 " spm, Power: %" PRIu16 " W", 
                 data->stroke_count, data->distance_m, data->stroke_rate, data->power_watts);
        
        // TODO: Send GATT notification to connected clients with Indoor Rower Data packet
        // Format: Flags(2) + StrokeRate(2) + StrokeCount(2) + AvgStrokeRate(2) + 
        //         Distance(3) + Pace(2) + AvgPace(2) + Power(2) + AvgPower(2) + 
        //         Calories(2) + EnergyPerHr(2) + ElapsedTime(2)
    }
}

/**
 * @brief Check if any clients are connected
 */
bool ble_ftms_is_connected(void)
{
    return is_connected && bt_initialized;
}

/**
 * @brief Start advertising FTMS service
 */
void ble_ftms_start_advertising(void)
{
    ESP_LOGI(TAG, "Starting advertising (TODO: implement esp_ble_gap_start_advertising)");
    
    if (!bt_initialized) {
        ESP_LOGW(TAG, "Bluetooth not initialized, cannot advertise");
        return;
    }
    
    // TODO: Implement advertising
    // esp_ble_adv_data_t adv_data = {0};
    // Set advertisement data with FTMS UUID (0x1826)
    // esp_ble_gap_set_device_name("FDF Rower");
    // esp_ble_gap_config_adv_data(&adv_data);
    // esp_ble_gap_start_advertising(&adv_params);
}

/**
 * @brief Stop advertising FTMS service
 */
void ble_ftms_stop_advertising(void)
{
    ESP_LOGI(TAG, "Stopping advertising (TODO: implement esp_ble_gap_stop_advertising)");
    
    // TODO: Implement stop advertising
    // esp_ble_gap_stop_advertising();
}

/**
 * @brief Deinitialize FTMS service
 */
void ble_ftms_deinit(void)
{
    esp_err_t ret;
    
    ESP_LOGI(TAG, "Deinitializing Bluetooth FTMS service");
    
    if (!bt_initialized) {
        ESP_LOGW(TAG, "Bluetooth not initialized");
        if (data_mutex != NULL) {
            vSemaphoreDelete(data_mutex);
            data_mutex = NULL;
        }
        return;
    }
    
    // Stop advertising
    ble_ftms_stop_advertising();
    
    // Disable Bluedroid
    ret = esp_bluedroid_disable();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to disable Bluedroid: %s", esp_err_to_name(ret));
    }
    
    ret = esp_bluedroid_deinit();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to deinit Bluedroid: %s", esp_err_to_name(ret));
    }
    
    // Disable and deinit BT controller
    ret = esp_bt_controller_disable();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to disable BT controller: %s", esp_err_to_name(ret));
    }
    
    ret = esp_bt_controller_deinit();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to deinit BT controller: %s", esp_err_to_name(ret));
    }
    
    // Delete mutex
    if (data_mutex != NULL) {
        vSemaphoreDelete(data_mutex);
        data_mutex = NULL;
    }
    
    bt_initialized = false;
    ESP_LOGI(TAG, "Bluetooth FTMS service deinitialized");
}