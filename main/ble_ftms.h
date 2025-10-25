#ifndef BLE_FTMS_H
#define BLE_FTMS_H

#include <stdint.h>
#include <stdbool.h>
#include "fdf_protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

// FTMS Indoor Rower Data flags
#define FTMS_FLAG_MORE_DATA                   0x01
#define FTMS_FLAG_AVG_SPEED_PRESENT           0x02
#define FTMS_FLAG_TOTAL_DISTANCE_PRESENT       0x04
#define FTMS_FLAG_INCLINATION_PRESENT          0x08
#define FTMS_FLAG_ELEVATION_GAIN_PRESENT       0x10
#define FTMS_FLAG_INSTANTANEOUS_PACE_PRESENT   0x20
#define FTMS_FLAG_AVERAGE_PACE_PRESENT         0x40
#define FTMS_FLAG_EXPANDED_ENERGY_PRESENT      0x80
#define FTMS_FLAG_HEART_RATE_PRESENT           0x100
#define FTMS_FLAG_METABOLIC_EQUIVALENT_PRESENT 0x200
#define FTMS_FLAG_ELAPSED_TIME_PRESENT         0x400
#define FTMS_FLAG_REMAINING_TIME_PRESENT       0x800
#define FTMS_FLAG_FORCE_ON_BELT_PRESENT        0x1000
#define FTMS_FLAG_POWER_OUTPUT_PRESENT         0x2000
#define FTMS_FLAG_SPEED_PRESENT                0x4000
#define FTMS_FLAG_STEP_RATE_PRESENT            0x8000

// FTMS Indoor Rower Data flags for rowing
#define FTMS_INDOOR_ROWER_FLAGS (FTMS_FLAG_TOTAL_DISTANCE_PRESENT | \
                                 FTMS_FLAG_INSTANTANEOUS_PACE_PRESENT | \
                                 FTMS_FLAG_AVERAGE_PACE_PRESENT | \
                                 FTMS_FLAG_EXPANDED_ENERGY_PRESENT | \
                                 FTMS_FLAG_ELAPSED_TIME_PRESENT | \
                                 FTMS_FLAG_POWER_OUTPUT_PRESENT | \
                                 FTMS_FLAG_STEP_RATE_PRESENT)

/**
 * @brief Initialize Bluetooth FTMS service
 * @return true if successful, false otherwise
 */
bool ble_ftms_init(void);

/**
 * @brief Update FTMS data with new rowing metrics
 * @param data Pointer to rowing data structure
 */
void ble_ftms_update_data(const fdf_rowing_data_t *data);

/**
 * @brief Check if any clients are connected
 * @return true if connected, false otherwise
 */
bool ble_ftms_is_connected(void);

/**
 * @brief Start advertising FTMS service
 */
void ble_ftms_start_advertising(void);

/**
 * @brief Stop advertising FTMS service
 */
void ble_ftms_stop_advertising(void);

/**
 * @brief Deinitialize FTMS service
 */
void ble_ftms_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // BLE_FTMS_H
