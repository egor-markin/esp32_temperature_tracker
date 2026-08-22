#include <WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

// Include configuration file (copy config_template.h to config.h and fill with your data)
#include "config.h"

// Delay (in milliseconds) before sending another request to the server
#define TIMER_DELAY 60000

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
  Serial.print("Connected to WiFi network with IP address: ");
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
      String sensorAddress = convertDeviceAddressToString(tempDeviceAddress);
      String sensorLabel = getSensorIdByAddress(sensorAddress);

      if (sensorLabel == "") {
        sensorLabel = "<unknown>";
      }

      Serial.print("Device ");
      Serial.print(i, DEC);
      Serial.print(" has address ");
      Serial.print(sensorAddress);
      Serial.print(" and label: ");
      Serial.println(sensorLabel);
    } else {
      Serial.print("Found ghost device at ");
      Serial.print(i, DEC);
      Serial.print(" but could not detect address. Check power and cabling");
    }
  }

  // Accurate time is necessary for certificate validation and writing in batches
  // We use the NTP servers in your area as provided by: https://www.pool.ntp.org/zone/
  // Syncing progress and the time will be printed to Serial.
  timeSync(TZ_INFO, NTP_SERVER);

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

void prepareAndWriteData() {
  // Request temperatures from the sensors
  thermalSensors.requestTemperatures();

  // Clear fields for reusing the data point. Tags remain the same.
  dataPoint.clearFields();

  // Loop through each device, creating the resulting data
  for (int i = 0; i < numberOfDevices; i++) {
    if (thermalSensors.getAddress(tempDeviceAddress, i)) {
      String sensorAddressStr = convertDeviceAddressToString(tempDeviceAddress);
      float tempC = thermalSensors.getTempC(tempDeviceAddress);
      dataPoint.addField(getSensorIdByAddress(sensorAddressStr), tempC);
    }
  }

  // Print the data being written
  Serial.print("Writing: ");
  Serial.println(dataPoint.toLineProtocol());

  // Write the data point to InfluxDB
  if (!influxDbClient.writePoint(dataPoint)) {
    Serial.print("InfluxDB write failed: ");
    Serial.println(influxDbClient.getLastErrorMessage());
  }
}

void reconnectToWiFi() {
  Serial.println("WiFi disconnected. Attempting to reconnect...");

  int attempt = 1;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("Reconnection attempt ");
    Serial.print(attempt);
    Serial.println("...");

    WiFi.reconnect();

    // Wait with an exponentially increasing delay between attempts
    int delayTime = min(60000, attempt * 1000); // Max delay of 1 minute
    delay(delayTime);

    if (WiFi.status() == WL_CONNECTED) {
      Serial.print("Reconnected to WiFi network successfully with IP address: ");
      Serial.println(WiFi.localIP());
      return; // Exit the function once reconnected
    }

    attempt++;
  }

  Serial.println("WiFi reconnection failed after multiple attempts.");
}

void writeData() {
  if (WiFi.status() == WL_CONNECTED) {
    prepareAndWriteData();
  } else {
    reconnectToWiFi();

    // If reconnected, send the data point
    if (WiFi.status() == WL_CONNECTED) {
      prepareAndWriteData();
    }
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
  // Return an empty string if the key is not found
  return "";
}
