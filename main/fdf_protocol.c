#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"

#include "fdf_protocol.h"

static const char *TAG = "FDF_PROTOCOL";

// Global rowing data
static fdf_rowing_data_t current_data = {0};
static fdf_data_callback_t data_callback = NULL;

// Data buffer for incoming serial data
#define MAX_BUFFER_SIZE 1024
static char data_buffer[MAX_BUFFER_SIZE];
static size_t buffer_pos = 0;

// Session start time
static int64_t session_start_time = 0;

// Helper function to parse integer from string
static int parse_int(const char *str, int *value)
{
    char *endptr;
    *value = strtol(str, &endptr, 10);
    return (endptr != str && *endptr == '\0') ? 1 : 0;
}

// Helper function to parse float from string (unused for now)
// static int parse_float(const char *str, float *value)
// {
//     char *endptr;
//     *value = strtof(str, &endptr);
//     return (endptr != str && *endptr == '\0') ? 1 : 0;
// }

// Parse a line of data from FDF console
// Based on typical rowing machine formats, expecting something like:
// "STROKES:123 TIME:12:34 DISTANCE:5000 RATE:24 POWER:150 CALORIES:200"
static void parse_data_line(const char *line)
{
    ESP_LOGD(TAG, "Parsing line: %s", line);
    
    char *line_copy = strdup(line);
    char *token;
    char *saveptr;
    
    // Tokenize by spaces
    token = strtok_r(line_copy, " \t\r\n", &saveptr);
    while (token != NULL) {
        // Look for key-value pairs
        char *colon = strchr(token, ':');
        if (colon != NULL) {
            *colon = '\0';
            char *key = token;
            char *value = colon + 1;
            
            // Parse different metrics based on key
            if (strcmp(key, "STROKES") == 0 || strcmp(key, "STROKE") == 0) {
                parse_int(value, (int*)&current_data.stroke_count);
            }
            else if (strcmp(key, "TIME") == 0) {
                // Parse time in MM:SS format
                int minutes, seconds;
                if (sscanf(value, "%d:%d", &minutes, &seconds) == 2) {
                    current_data.elapsed_time_ms = (minutes * 60 + seconds) * 1000;
                }
            }
            else if (strcmp(key, "DISTANCE") == 0 || strcmp(key, "DIST") == 0) {
                parse_int(value, (int*)&current_data.distance_m);
            }
            else if (strcmp(key, "RATE") == 0 || strcmp(key, "SPM") == 0) {
                parse_int(value, (int*)&current_data.stroke_rate);
            }
            else if (strcmp(key, "AVGRATE") == 0 || strcmp(key, "AVG_RATE") == 0) {
                parse_int(value, (int*)&current_data.avg_stroke_rate);
            }
            else if (strcmp(key, "POWER") == 0 || strcmp(key, "WATTS") == 0) {
                parse_int(value, (int*)&current_data.power_watts);
            }
            else if (strcmp(key, "AVGPOWER") == 0 || strcmp(key, "AVG_POWER") == 0) {
                parse_int(value, (int*)&current_data.avg_power_watts);
            }
            else if (strcmp(key, "CALORIES") == 0 || strcmp(key, "CAL") == 0) {
                parse_int(value, (int*)&current_data.calories);
            }
            else if (strcmp(key, "PACE") == 0) {
                // Parse pace in MM:SS format for 500m
                int minutes, seconds;
                if (sscanf(value, "%d:%d", &minutes, &seconds) == 2) {
                    current_data.pace_500m_ms = (minutes * 60 + seconds) * 1000;
                }
            }
            else if (strcmp(key, "AVGPACE") == 0 || strcmp(key, "AVG_PACE") == 0) {
                // Parse average pace in MM:SS format for 500m
                int minutes, seconds;
                if (sscanf(value, "%d:%d", &minutes, &seconds) == 2) {
                    current_data.avg_pace_500m_ms = (minutes * 60 + seconds) * 1000;
                }
            }
        }
        
        token = strtok_r(NULL, " \t\r\n", &saveptr);
    }
    
    free(line_copy);
    
    // Mark session as active if we have any data
    if (current_data.stroke_count > 0 || current_data.distance_m > 0) {
        current_data.session_active = true;
        
        // If this is the first data, record session start time
        if (session_start_time == 0) {
            session_start_time = esp_timer_get_time();
        }
    }
    
    // Notify callback if registered
    if (data_callback) {
        data_callback(&current_data);
    }
}

bool fdf_protocol_init(void)
{
    ESP_LOGI(TAG, "Initializing FDF protocol parser...");
    
    // Initialize data structure
    memset(&current_data, 0, sizeof(fdf_rowing_data_t));
    buffer_pos = 0;
    session_start_time = 0;
    
    ESP_LOGI(TAG, "FDF protocol parser initialized");
    return true;
}

void fdf_protocol_register_callback(fdf_data_callback_t callback)
{
    data_callback = callback;
}

void fdf_protocol_process_data(const uint8_t *data, size_t length)
{
    if (!data || length == 0) {
        return;
    }
    
    // Process each byte
    for (size_t i = 0; i < length; i++) {
        char c = (char)data[i];
        
        // Handle different line endings
        if (c == '\n' || c == '\r') {
            if (buffer_pos > 0) {
                // Null terminate the buffer
                data_buffer[buffer_pos] = '\0';
                
                // Parse the line
                parse_data_line(data_buffer);
                
                // Reset buffer
                buffer_pos = 0;
            }
        }
        else if (buffer_pos < MAX_BUFFER_SIZE - 1) {
            // Add character to buffer
            data_buffer[buffer_pos++] = c;
        }
        else {
            // Buffer overflow, reset
            ESP_LOGW(TAG, "Data buffer overflow, resetting");
            buffer_pos = 0;
        }
    }
}

bool fdf_protocol_get_current_data(fdf_rowing_data_t *data)
{
    if (!data) {
        return false;
    }
    
    memcpy(data, &current_data, sizeof(fdf_rowing_data_t));
    return current_data.session_active;
}

void fdf_protocol_reset_session(void)
{
    ESP_LOGI(TAG, "Resetting FDF session data");
    
    memset(&current_data, 0, sizeof(fdf_rowing_data_t));
    buffer_pos = 0;
    session_start_time = 0;
}
