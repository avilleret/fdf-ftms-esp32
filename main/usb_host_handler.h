#ifndef USB_HOST_HANDLER_H
#define USB_HOST_HANDLER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// USB Host callback function type for data received
typedef void (*usb_data_callback_t)(const uint8_t *data, size_t length);

/**
 * @brief Initialize USB host and CDC-ACM driver
 * @return true if successful, false otherwise
 */
bool usb_host_init(void);

/**
 * @brief Register callback for data received from FDF console
 * @param callback Function to call when data is received
 */
void usb_host_register_data_callback(usb_data_callback_t callback);

/**
 * @brief Check if FDF console is connected
 * @return true if connected, false otherwise
 */
bool usb_host_is_connected(void);

/**
 * @brief Deinitialize USB host
 */
void usb_host_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // USB_HOST_HANDLER_H
