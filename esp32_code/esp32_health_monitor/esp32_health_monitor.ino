#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

MAX30105 particleSensor;
BLEServer *pServer;
BLECharacteristic *pCharacteristic;

#define SERVICE_UUID        "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define BUFFER_SIZE 100
#define GSR_PIN 34

uint32_t irBuffer[BUFFER_SIZE];
uint32_t redBuffer[BUFFER_SIZE];
int32_t spo2; int8_t validSPO2;
int32_t heartRate; int8_t validHeartRate;

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  pinMode(GSR_PIN, INPUT);

  if (!particleSensor.begin(Wire, I2C_SPEED_STANDARD)) {
    Serial.println("MAX30102 not found");
    while (1);
  }

  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x0A);
  particleSensor.setPulseAmplitudeIR(0x0A);
  particleSensor.setSampleRate(100);
  particleSensor.setFIFOAverage(4);
  particleSensor.setADCRange(16384);
  particleSensor.setPulseWidth(411);

  BLEDevice::init("ESP32 Health Monitor");
  pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_NOTIFY);
  pCharacteristic->addDescriptor(new BLE2902());
  pService->start();
  BLEDevice::getAdvertising()->addServiceUUID(SERVICE_UUID);
  BLEDevice::startAdvertising();
}

void loop() {
  for (int i = 0; i < BUFFER_SIZE; i++) {
    while (!particleSensor.check());
    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
  }

  maxim_heart_rate_and_oxygen_saturation(irBuffer, BUFFER_SIZE, redBuffer,
                                         &spo2, &validSPO2, &heartRate, &validHeartRate);

  float gsrValue = analogRead(GSR_PIN) / 4095.0 * 3.3;
  if (heartRate > 40 && heartRate < 160 && validHeartRate && validSPO2) {
    String data = "SpO2:" + String(spo2) + "%, Heart Rate:" + String(heartRate) + ", GSR:" + String(gsrValue, 2);
    Serial.println(data);
    pCharacteristic->setValue(data.c_str());
    pCharacteristic->notify();
  }

  delay(2000);
}
