#include <Arduino.h>
#include <HX711.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// Pins for HX711
const int LOADCELL_DOUT_PIN = 15;
const int LOADCELL_SCK_PIN = 2;

// Initialize HX711
HX711 scale;

// New BLE Service and Characteristic UUIDs
#define SERVICE_UUID "1234"
#define CHARACTERISTIC_UUID "4321"

// BLE Characteristic
BLECharacteristic *pCharacteristic;

// Variable to store load cell data
long loadCellData = 0;
int receivedData =0;
// Callback class for receiving data
class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    Serial.print("Received Value: ");
    for (int i = 0; i < value.length(); i++) {
      Serial.print(value[i]);
    }
    Serial.println();
    
    // Process the received value (convert to integer, float, etc. if needed)
    receivedData = atoi(value.c_str());
    Serial.print("Processed Value: ");
    Serial.println(receivedData);
    // You can now use receivedData in your program
  }
};

void setup() {
  Serial.begin(115200);

  // Initialize HX711
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(416.296295);  // You can calibrate your scale using known weights
  scale.tare();       // Reset the scale to 0

  // Initialize BLE
  BLEDevice::init("ESP32 BLE");
  BLEAddress bleAddress = BLEDevice::getAddress();
  Serial.print("BLE Address: ");
  Serial.println(bleAddress.toString().c_str());

  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_WRITE
                    );

  // Add a descriptor to allow notifications
  pCharacteristic->addDescriptor(new BLE2902());

  // Set callback for write
  pCharacteristic->setCallbacks(new MyCallbacks());

  pService->start();
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
}

void loop() {
  if (scale.is_ready()) {
    loadCellData = scale.get_units(1);  // Read data from HX711
    // Serial.print("Load Cell Reading: ");
    // Serial.println(loadCellData);

    // Convert load cell data to string
    char dataString[20];
    sprintf(dataString, "%ld", loadCellData);

    // Update BLE characteristic
    pCharacteristic->setValue(dataString);
    pCharacteristic->notify();
  } else {
    Serial.println("HX711 not found.");
  }
  if(receivedData ==143){
    scale.tare();       // Reset the scale to 0
    receivedData =0;
  }
  
  delay(100); // Adjust delay as needed
}
