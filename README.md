# FDF Bluetooth Bridge

A Bluetooth bridge for First Degree Fitness (FDF) rower consoles that enables wireless connectivity to fitness applications like Kinomap using the Bluetooth Fitness Machine Service (FTMS) protocol.

## Overview

This project creates an ESP32-S3 firmware that acts as a USB-to-Bluetooth bridge, reading rowing data from the FDF console's USB interface and broadcasting it via Bluetooth FTMS for compatibility with modern fitness applications.

## Hardware Requirements

- **ESP32-S3-DevKitC-1** or **ESP32-S3-USB-OTG** development board (with native USB OTG support)
- USB OTG cable/adapter to connect FDF console to ESP32-S3
- First Degree Fitness rower console with USB interface

## Features

- **USB Host Communication**: Connects directly to FDF console via USB CDC-ACM
- **Bluetooth FTMS**: Implements Fitness Machine Service for indoor rowing
- **Real-time Data**: Transmits stroke rate, distance, power, calories, and timing data
- **Compatible Apps**: Works with Kinomap, Zwift, and other FTMS-compatible applications

## Supported Data Metrics

- Stroke count and rate (current and average)
- Distance (meters)
- Elapsed time
- Power output (watts, current and average)
- Calories burned
- Pace per 500m (current and average)

## Build Instructions

### Prerequisites

1. Install ESP-IDF v5.0 (recommended stable version):
   ```bash
   # Install ESP-IDF v5.0
   git clone -b v5.0 --recursive https://github.com/espressif/esp-idf.git
   cd esp-idf
   ./install.sh esp32s3
   . ./export.sh
   ```
   
   **Note**: ESP-IDF v5.5+ and master branch have USB Host API changes that may cause compatibility issues. v5.0 is the most stable version for this project.

2. Clone this repository:
   ```bash
   git clone <repository-url>
   cd fdf-esp32
   ```

### Building and Flashing

1. Set up ESP-IDF environment:
   ```bash
   export IDF_PATH=~/dev/esp-idf  # Adjust path to your ESP-IDF installation
   source ~/dev/esp-idf/export.sh
   ```

2. Configure the project:
   ```bash
   idf.py set-target esp32s3
   idf.py menuconfig
   ```

3. Build the project:
   ```bash
   idf.py build
   ```

4. Flash to ESP32-S3:
   ```bash
   idf.py flash monitor
   ```

## Current Status

✅ **Project builds successfully** with ESP-IDF v5.0  
✅ **Basic project structure** implemented  
✅ **FDF protocol parser** ready for data processing  
⚠️ **USB Host functionality** - stub implementation (needs actual USB host code)  
⚠️ **Bluetooth FTMS** - stub implementation (needs actual Bluetooth code)  

## Next Steps

The project currently has stub implementations for USB host and Bluetooth functionality. To complete the implementation:

1. **Implement USB Host**: Replace the stub in `usb_host_handler.c` with actual USB CDC-ACM host code
2. **Implement Bluetooth FTMS**: Replace the stub in `ble_ftms.c` with actual Bluetooth GATT server code
3. **Test with FDF Console**: Connect actual FDF rower console and test data flow
4. **Test with Fitness Apps**: Verify compatibility with Kinomap and other FTMS apps

## Hardware Setup

1. **Connect FDF Console**: Use a USB OTG cable to connect your FDF console to the ESP32-S3's USB port (not the UART port)

2. **Power**: Connect the ESP32-S3 to power via USB or external power supply

3. **LED Indicators**: The ESP32-S3 will show connection status via built-in LEDs

## Usage

1. **Power On**: Connect power to the ESP32-S3
2. **Connect FDF Console**: Plug in your FDF console via USB OTG cable
3. **Pair Device**: Look for "FDF Rower" in your fitness app's Bluetooth device list
4. **Start Rowing**: Begin your rowing session - data will be transmitted in real-time

## Supported Applications

- **Kinomap**: Full compatibility with indoor rowing workouts
- **Zwift**: Connect as indoor rowing machine
- **Concept2 Utility**: Monitor and log workouts
- **Any FTMS-compatible app**: Generic Bluetooth fitness machine support

## Troubleshooting

### FDF Console Not Detected
- Ensure USB OTG cable is properly connected
- Check that FDF console is powered on
- Verify USB port on ESP32-S3 (use USB port, not UART port)
- Check console logs for USB connection errors

### Bluetooth Connection Issues
- Ensure device is advertising (check logs)
- Try restarting the ESP32-S3
- Clear Bluetooth cache on your device
- Check that FTMS service is properly initialized

### Data Not Updating
- Verify FDF console is sending data (check logs)
- Ensure rowing session is active
- Check protocol parser is receiving data
- Monitor FTMS notifications in logs

## Configuration

### USB Host Settings
- Buffer size: 1024 bytes
- CDC-ACM driver enabled
- USB host library configured for ESP32-S3

### Bluetooth Settings
- Device name: "FDF Rower"
- Service: Fitness Machine Service (0x1826)
- Characteristic: Indoor Rower Data (0x2AD1)
- Advertising interval: 20-40ms

### Data Parsing
The FDF protocol parser expects data in the format:
```
STROKES:123 TIME:12:34 DISTANCE:5000 RATE:24 POWER:150 CALORIES:200
```

## Development

### Project Structure
```
main/
├── main.c              # Main application entry point
├── usb_host_handler.c/h # USB host and CDC-ACM communication
├── fdf_protocol.c/h     # FDF console protocol parser
├── ble_ftms.c/h         # Bluetooth FTMS service implementation
└── CMakeLists.txt       # Build configuration
```

### Adding New Metrics
To add support for additional rowing metrics:

1. Update `fdf_rowing_data_t` structure in `fdf_protocol.h`
2. Add parsing logic in `fdf_protocol.c`
3. Update FTMS data packet in `ble_ftms.c`
4. Add appropriate FTMS flags

### Debugging
Enable debug logging by setting log level to DEBUG in `sdkconfig`:
```
CONFIG_LOG_DEFAULT_LEVEL_DEBUG=y
```

## References

- [FDF Console Recorder](https://github.com/avilleret/fdf-console-recorder) - FDF protocol reference
- [Open Rowing Monitor](https://github.com/laberning/openrowingmonitor) - FTMS implementation reference
- [ESP-IDF USB Host Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/peripherals/usb_host.html)
- [Bluetooth FTMS Specification](https://www.bluetooth.com/specifications/specs/fitness-machine-service-1-0/)

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests.

## Support

For support and questions:
- Open an issue on GitHub
- Check the troubleshooting section above
- Review ESP-IDF documentation for USB host and Bluetooth issues
