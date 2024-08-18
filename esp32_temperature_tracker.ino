#include <WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

// Data wire is plugged TO GPIO 4
#define ONE_WIRE_BUS 4

// InfluxDB-related data
#define INFLUXDB_URL "https://your-influxdb-url.com"
#define INFLUXDB_TOKEN "your-influxdb-token"
#define INFLUXDB_ORG "your-organization"
#define INFLUXDB_BUCKET "your-bucket"
#define INFLUXDB_MEASUREMENT "your-measurement"

// Time zone info (get it here: https://en.wikipedia.org/wiki/List_of_tz_database_time_zones)
#define TZ_INFO "your-timezone"

// WiFi credentials
#define WIFI_SSID "your-wifi-ssid"
#define WIFI_PASSWORD "your-wifi-password"

// Delay (in milliseconds) before sending another request to the server
#define TIMER_DELAY 60000

// Available DS18B20 thermal sensors devices
const int NUMBER_OF_SENSORS = 1;
const String sensors_keys[NUMBER_OF_SENSORS] = { "your-sensor-key-1" };
const String sensors_values[NUMBER_OF_SENSORS] = { "your-temperature-field-name" };

// Declare InfluxDB client instance with preconfigured InfluxCloud certificate
InfluxDBClient influxDbClient(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);

// Declare Data point
Point dataPoint(INFLUXDB_MEASUREMENT);

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature thermalSensors(&oneWire);

// Number of temperature devices found
int numberOfDevices;

// We'll use this variable to store a found device address
DeviceAddress tempDeviceAddress;

void setup() {
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi...");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
 
  // Start up the library
  thermalSensors.begin();

  Serial.print("Looking for DS18B20 temperature sensors... ");

  // Grab a count of devices on the wire
  numberOfDevices = thermalSensors.getDeviceCount();
  
  // locate devices on the bus
  Serial.print("found ");
  Serial.print(numberOfDevices, DEC);
  Serial.println(" device(s)");

  // Loop through each device, print out address
  for (int i = 0; i < numberOfDevices; i++) {
    // Search the wire for address
    if (thermalSensors.getAddress(tempDeviceAddress, i)) {
      Serial.print("Device ");
      Serial.print(i, DEC);
      Serial.print(" with the address ");
      Serial.println(convertDeviceAddressToString(tempDeviceAddress));
    } else {
      Serial.print("Found ghost device at ");
      Serial.print(i, DEC);
      Serial.print(" but could not detect address. Check power and cabling");
    }
  }

  // Accurate time is necessary for certificate validation and writing in batches
  // We use the NTP servers in your area as provided by: https://www.pool.ntp.org/zone/
  // Syncing progress and the time will be printed to Serial.
  timeSync(TZ_INFO, "pool.ntp.org");

  // Checking the InfluxDB server connection
  if (influxDbClient.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(influxDbClient.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(influxDbClient.getLastErrorMessage());
  }

  // Writing the first data point
  writeData();
}

void loop() {
  if ((millis() - lastTime) > TIMER_DELAY) {
    writeData();
    lastTime = millis();
  }
}

void writeData() {
      // Check WiFi connection status
    if (WiFi.status() == WL_CONNECTED) {
      thermalSensors.requestTemperatures(); // Send the command to get temperatures

      // Clear fields for reusing the point. Tags will remain the same as set above.
      dataPoint.clearFields();

      // Loop through each device, creating the resulting request string
      String httpRequestData = "";
      for (int i = 0; i < numberOfDevices; i++) {
        // Search the wire for address
        if (thermalSensors.getAddress(tempDeviceAddress, i)) {
          String sensorAddressStr = convertDeviceAddressToString(tempDeviceAddress);
          float tempC = thermalSensors.getTempC(tempDeviceAddress);
          dataPoint.addField(getSensorIdByAddress(sensorAddressStr), tempC);
        }
      }

      // // Printing what are we exactly writing
      Serial.print("Writing: ");
      Serial.println(dataPoint.toLineProtocol());

      // // Writing a point
      if (!influxDbClient.writePoint(dataPoint)) {
        Serial.print("InfluxDB write failed: ");
        Serial.println(influxDbClient.getLastErrorMessage());
      }
    } else {
      Serial.println("WiFi Disconnected");
    }
}

String convertDeviceAddressToString(DeviceAddress da) {
  String str = "";
  for (uint8_t j = 0; j < 8; j++) {
    if (da[j] < 16) {
      str += "0";
    }
    str += String(da[j], HEX);
  }
  str.toUpperCase();
  return str;
}

// Function to retrieve a value by key
String getSensorIdByAddress(const String &key) {
  for (int i = 0; i < NUMBER_OF_SENSORS; i++) {
    if (sensors_keys[i] == key) {
      return sensors_values[i];
    }
  }
  // Return an empty string if key not found
  return "";
}