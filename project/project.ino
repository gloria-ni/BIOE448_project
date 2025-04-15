#include <Wire.h> // Necessary for I2C communication
int accel = 0x53; // I2C address for this sensor (from data sheet)
float x, y, z;
int steps = 0;

#include <LiquidCrystal.h>
// initialize the library by associating LCD pins with arduino pin number connections
const int rs=10, en=8, d4=2, d5=3, d6=4, d7=5;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

#include <ArduinoBLE.h>
BLEService newService("180A");
BLEByteCharacteristic readChar("2A58", BLERead);
BLEByteCharacteristic writeChar("2A57", BLEWrite);

void setup() {
  Serial.begin(9600);
  Wire.begin(); // Initialize serial communications
  Wire.beginTransmission(accel); // Start communicating with the device
  Wire.write(0x2D); // Enable measurement
  Wire.write(8); // Get sample measurement
  Wire.endTransmission();

  lcd.begin(16,2); // set up LCD's number of columns and rows
  lcd.print("Step Tracker"); // initial display message

  Serial.begin(9600);
  while(!Serial);
  if (!BLE.begin()){
    Serial.println("Waiting for ArduinoBLE");
    while(1);
  }
  BLE.setLocalName("Gloria Project");
  //BLE.setAdvertisedServiceUuid("5189861260ABCDEF");
  BLE.setAdvertisedService(newService);
  newService.addCharacteristic(readChar);
  newService.addCharacteristic(writeChar);
  BLE.addService(newService);
  readChar.writeValue(0);
  writeChar.writeValue(0);
  BLE.advertise();
  Serial.println("Bluetooth device active");
}

void loop() {
  Wire.beginTransmission(accel);
  Wire.write(0x32); // Prepare to get readings for sensor (address from data sheet)
  Wire.endTransmission(false);
  Wire.requestFrom(accel, 6, true); // Get readings (2 readings per direction x 3 directions = 6 values)
  x = (Wire.read() | Wire.read() << 8); // Parse x values
  y = (Wire.read() | Wire.read() << 8); // Parse y values
  y = (Wire.read() | Wire.read() << 8); // Parse z values

  Serial.println(x);


  if (x>900){
    steps = steps + 1;
    lcd.clear();
    lcd.print(String(steps) + " steps");
    delay(400); 
  }

  if(steps=10000){
    lcd.clear();
    lcd.print("Daily Goal Met!");
    delay(5000);
  }

  BLEDevice central = BLE.central(); // wait for a BLE central

  if (central) {  // if a central is connected to the peripheral
    Serial.print("Connected to central: ");
    
    Serial.println(central.address()); // print the central's BT address
    
    digitalWrite(LED_BUILTIN, HIGH); // turn on the LED to indicate the connection

    while (central.connected()) { // while the central is connected:
      // if (writeChar.written()) {
        // if (writeChar.value()) {
          readChar.writeValue(steps);
          Serial.println("Step count printed to the peripheral");
        // }
      // }
    }

    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
  }
}
