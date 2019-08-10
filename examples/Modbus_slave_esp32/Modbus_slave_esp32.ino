/*
  Modbus_slave_esp32 
 http://github.com/andresarmento/modbus-arduino
 This example is tested with Modbus Box and PC QModbusMaster
 
 Modbus functions supported:
 0x01 - Read Coils
 0x02 - Read Input Status (Read Discrete Inputs)
 0x03 - Read Holding Registers
 0x04 - Read Input Registers
 0x05 - Write Single Coil
 0x06 - Write Single Register
 0x0F - Write Multiple Coils
 0x10 - Write Multiple Registers
 */


#include <Modbus.h>
#include <ModbusSerial.h>
#define PRINTREGISTERS

// ModBus Port information
#define BAUD 9600   
#define ID 4
#define TXPIN 4                     // pin de habilitacion del Max485 Modbus Box

#define NUM_DISCRETE_INPUT 3
#define NUM_INPUT_REGISTERS 3
#define NUM_COILS 3
#define NUM_HOLDING_REGISTERS 1820  // Holding reg start at 1810 end at 1819, 
#define digin26  26
#define digin0   0
#define PRINTINTERVAL 2000          //in miliseconds

// ModbusLoRa Object
ModbusSerial modbus;

const int ledPin = 27;  //ACT led

int value = 0;
int idx = 0;

unsigned long update_time = 0;
unsigned long inc_time = 0;
unsigned long last_print = 0;

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(digin26, INPUT);
  pinMode(digin0, INPUT);
  digitalWrite(ledPin, HIGH);

  Serial.begin(115200);
  Serial.println("Hello! - ModBus Serial Slave");

  // Config Modbus Serial (port, speed, byte format) 
  modbus.config(&Serial1, 9600,  TXPIN);

  // Set the Slave ID (1-247)
  modbus.setSlaveId(ID);  

  // Use addIsts() for digital inputs - Discrete Input - Master Read-Only
  for (int i = 0; i < NUM_DISCRETE_INPUT; ++i) {
    modbus.addIsts(i);
  }

  // Use addIreg() for analog Inputs - Input Register - Master Read-Only
  for (int i = 0; i < NUM_INPUT_REGISTERS; ++i) {
    modbus.addIreg(i);
  }

  // Use addCoil() for digital outputs -  Coil - Master Read-Write 
  for (int i = 0; i < NUM_COILS; ++i) {
    modbus.addCoil(i);
  }

  // Use addHreg() for analog outpus - Holding Register - Master Read-Write
  for (int i = 1810; i < NUM_HOLDING_REGISTERS; ++i) {
    modbus.addHreg(i, i);
  }


  update_time = millis();
  inc_time = update_time;

  digitalWrite(ledPin, HIGH);
}

void loop() {
  //Call once inside loop() - all magic here
  modbus.task();

  if (millis() > inc_time + 500) {
    inc_time = millis();

    ++value;

    if(value >= 5000)
      value = 0;
  }

  if (millis() > update_time + 1000) {
    update_time = millis();

    // Set Digital Input - Discrete Input - Master Read-Only   ( tested and works ok) * angel
    modbus.Ists(0, digitalRead(digin26));  //Mbox WIFI setup
    modbus.Ists(1, LOW); 
    modbus.Ists(2, digitalRead(digin0));  //Mbox setup
    
    // Set Analog Input - Input Register - Master Read-Only  ( tested and works ok) * angel
    modbus.Ireg(0, 12345);
    modbus.Ireg(1, random(1, 65535));
    modbus.Ireg(2, random(1, 1000));


    //Attach ledPin to Digital Output - Coil - Master Read-Write  ( tested and works ok) * angel
    modbus.Coil(0, !modbus.Coil(0));
    digitalWrite(ledPin, modbus.Coil(0));  
     
    modbus.Hreg(1810, 1810);  //Holding register comienzan con la direccion 1810-1819 ( tested and works ok) * angel
    modbus.Hreg(1811, random(1, 65535));
    modbus.Hreg(1812, 12);
    modbus.Hreg(1813, -1813);
    modbus.Hreg(1814, 13);
    modbus.Hreg(1815, random(1, 2340));
    modbus.Hreg(1816, -1);   
    modbus.Hreg(1817, random(1, 1000)); 
    modbus.Hreg(1818, random(1, 1000)); 
    modbus.Hreg(1819, 1819); 
   
    
    ++idx;
    if( idx == 1124)
      idx = 0;
  }

#ifdef PRINTREGISTERS
  if ( millis() > last_print + PRINTINTERVAL) {
    last_print = millis();

    // Clear serial monitor - Does not work with Serial Monitor from Arduino IDE
    // On a terminal emulator run the following comand to get a complete serial monitor  (screen port baudrate)
    // screen /dev/tty.usbmodem401373 57600
    Serial.println('\n');
    Serial.write(27);       // ESC command
    Serial.print("[2J");    // clear screen command
    Serial.write(27);
    Serial.print("[H");     // cursor to home command

    Serial.print(" - Serial Slave - millis: ");
    Serial.println(millis());
    Serial.println("----------------------------------------------- ");

    // Print all Registers
    Serial.println("Digital Inputs - Read-Only");
    for (int i = 0; i < NUM_DISCRETE_INPUT; ++i) {
      Serial.print("Input ");
      Serial.print(i);
      Serial.print(" : ");
      Serial.println((modbus.Ists(i) != 0) ? "ON" : "OFF");
    }
    Serial.println();
    Serial.println("Analog Inputs - Read-Only");
    for (int i = 0; i < NUM_INPUT_REGISTERS; ++i) {
      Serial.print("I-Reg ");
      Serial.print(i);
      Serial.print(" : ");
      Serial.println(modbus.Ireg(i));
    }
    Serial.println();
    Serial.println("Digital Outputs - Read-Write");
    for (int i = 0; i < NUM_COILS; ++i) {
      Serial.print("Coil ");
      Serial.print(i);
      Serial.print(" : ");
      Serial.println((modbus.Coil(i) != 0) ? "ON" : "OFF");
    }
    Serial.println();
    Serial.println("Analog Outputs - Read-Write");
    for (int i = 1810; i < NUM_HOLDING_REGISTERS; ++i) {
      Serial.print("H-Reg ");
      Serial.print(i);
      Serial.print(" : ");
      Serial.println(modbus.Hreg(i));
    }

    Serial.println("------------------------ ");

    delay(5);
  }
#endif
}

