#ifndef FDF_PROTOCOL_H
#define FDF_PROTOCOL_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// FDF rowing metrics structure
typedef struct {
    uint16_t stroke_count;        // Total strokes
    uint32_t elapsed_time_ms;    // Elapsed time in milliseconds
    uint32_t distance_m;         // Distance in meters
    uint16_t stroke_rate;        // Current stroke rate (strokes per minute)
    uint16_t avg_stroke_rate;    // Average stroke rate
    uint16_t power_watts;        // Current power in watts
    uint16_t avg_power_watts;    // Average power in watts
    uint16_t calories;            // Total calories burned
    uint16_t pace_500m_ms;       // Pace per 500m in milliseconds
    uint16_t avg_pace_500m_ms;   // Average pace per 500m in milliseconds
    bool session_active;          // Whether a rowing session is active
} fdf_rowing_data_t;

// Callback function type for updated rowing data
typedef void (*fdf_data_callback_t)(const fdf_rowing_data_t *data);

/**
 * @brief Initialize FDF protocol parser
 * @return true if successful, false otherwise
 */
bool fdf_protocol_init(void);

/**
 * @brief Register callback for updated rowing data
 * @param callback Function to call when data is updated
 */
void fdf_protocol_register_callback(fdf_data_callback_t callback);

/**
 * @brief Process incoming data from FDF console
 * @param data Raw data received from console
 * @param length Length of data
 */
void fdf_protocol_process_data(const uint8_t *data, size_t length);

/**
 * @brief Get current rowing data
 * @param data Pointer to structure to fill with current data
 * @return true if data is valid, false otherwise
 */
bool fdf_protocol_get_current_data(fdf_rowing_data_t *data);

/**
 * @brief Reset session data
 */
void fdf_protocol_reset_session(void);

#ifdef __cplusplus
}
#endif

#endif // FDF_PROTOCOL_H
