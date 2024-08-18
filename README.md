## ESP32 Temperature Logger with InfluxDB Integration

This project is a temperature monitoring system built using an ESP32 microcontroller and DS18B20 temperature sensors. The system reads temperature data from the sensors and logs it to an InfluxDB database. The data is then available for real-time monitoring and analysis. 

### Key Features:
- **ESP32-Based**: Utilizes the powerful ESP32 microcontroller with built-in WiFi for seamless connectivity.
- **Multiple Sensor Support**: Supports multiple DS18B20 temperature sensors connected via a OneWire bus.
- **InfluxDB Integration**: Sends temperature data to an InfluxDB database, perfect for time-series data storage and visualization.

### How It Works:
1. **Sensor Initialization**: The ESP32 initializes the DS18B20 sensors on the OneWire bus.
2. **WiFi Connection**: The ESP32 connects to the specified WiFi network for internet access.
3. **Temperature Data Collection**: The ESP32 reads temperature data from each connected sensor.
4. **InfluxDB Logging**: The data, along with a timestamp, is sent to the InfluxDB server for storage and later analysis.
5. **Real-Time Monitoring**: Data can be visualized in real-time using InfluxDB’s dashboards or other compatible tools.

### Setup Instructions:
1. **Clone the Repository**: `git clone https://github.com/egor-markin/esp32-temperature-logger.git`
2. **Install Dependencies**: Make sure you have the necessary libraries (`WiFi`, `OneWire`, `DallasTemperature`, `InfluxDbClient`) installed.
3. **Configure WiFi and InfluxDB**: Update the WiFi credentials and InfluxDB connection settings in the code.
4. **Upload Code**: Use the Arduino IDE or PlatformIO to upload the code to your ESP32.
5. **Monitor Data**: Use InfluxDB or Grafana to monitor the logged temperature data.

### Requirements:
- **Hardware**: ESP32, DS18B20 temperature sensors, pull-up resistor for the OneWire bus.
- **Software**: Arduino IDE or PlatformIO, InfluxDB for data storage and visualization.
