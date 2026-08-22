#ifndef CONFIG_H
#define CONFIG_H

// InfluxDB configuration
#define INFLUXDB_URL "https://your-influxdb-server.com:8086"
#define INFLUXDB_TOKEN "your_influxdb_token_here"
#define INFLUXDB_ORG "your_organization_name"
#define INFLUXDB_BUCKET "your_bucket_name"
#define INFLUXDB_MEASUREMENT "your_measurement_name"

// WiFi credentials
#define WIFI_SSID "your_wifi_network_name"
#define WIFI_PASSWORD "your_wifi_password"

// Time zone (get it here: https://en.wikipedia.org/wiki/List_of_tz_database_time_zones)
#define TZ_INFO "your_time_zone" // E.g. Europe/Berlin

// NTP server (you can use your local NTP server or keep this default)
#define NTP_SERVER "pool.ntp.org"

// Hardware configuration
// Data wire is plugged to this GPIO pin
#define ONE_WIRE_BUS 1

// Available DS18B20 thermal sensors devices
// Replace with your actual sensor addresses and names
#define NUMBER_OF_SENSORS 1
static const String sensors_keys[NUMBER_OF_SENSORS] = { "your_sensor_address_here" }; // E.g. "28FF4E23101704FB"
static const String sensors_values[NUMBER_OF_SENSORS] = { "your_sensor_name_here" }; // E.g. "kitchen_temperature"

#endif
