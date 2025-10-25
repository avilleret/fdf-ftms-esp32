#ifndef USB_HOST_HANDLER_H
#define USB_HOST_HANDLER_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// USB Host callback function type for data received
typedef void (*usb_data_callback_t)(const uint8_t *data, size_t length);

// USB Host status
typedef enum {
    USB_HOST_STATUS_DISCONNECTED,
    USB_HOST_STATUS_CONNECTED,
    USB_HOST_STATUS_ERROR
} usb_host_status_t;

/**
 * @brief Initialize USB host and CDC-ACM driver
 * @param callback Function to call when data is received
 * @return ESP_OK if successful, error code otherwise
 */
esp_err_t usb_host_init(usb_data_callback_t callback);

/**
 * @brief Check if FDF console is connected
 * @return true if connected, false otherwise
 */
bool usb_host_is_connected(void);

/**
 * @brief Get current USB host status
 * @return USB host status
 */
usb_host_status_t usb_host_get_status(void);

/**
 * @brief Send data to USB device (if needed)
 * @param data Data to send
 * @param len Length of data
 * @return ESP_OK if successful, error code otherwise
 */
esp_err_t usb_host_send_data(const uint8_t *data, size_t len);

/**
 * @brief Deinitialize USB host
 */
void usb_host_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // USB_HOST_HANDLER_H
