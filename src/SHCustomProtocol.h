#ifndef __SHCUSTOMPROTOCOL_H__
#define __SHCUSTOMPROTOCOL_H__


#include <ACAN2515.h>
#include <ACAN2515Settings.h>
#include <ACANBuffer.h>
#include <CANMessage.h>
#include <MCP2515ReceiveFilters.h>



/*
 * Premier exemple de programme de l'article publé sur Locoduino:
 * https://www.locoduino.org/spip.php?article268
 */
#include <ACAN2515Settings.h> /* Pour les reglages  */
#include <ACAN2515.h>         /* Pour le controleur */
#include <CANMessage.h>       /* Pour les messages  */

/*
 * Broches pour le chip select et l'interruption du MCP2515
 */
static const uint8_t MCP2515_CS  = 20;
static const uint8_t MCP2515_INT = 2;

/*
 * L'objet pour piloter le MCP2515. SPI designe l'objet
 * utilise pour la connexion SPI car sur certaines cartes
 * notamment les Teensy, il peut y avoir plusieurs SPI.
 */
ACAN2515 controleurCAN(MCP2515_CS, SPI, MCP2515_INT);

/*
 * La frequence du quartz du MCP2515 en hertz. 
 * Sur les cartes CAN que l'on peut trouvez chez les revendeurs
 * chinois, il s'agit generalement d'un quartz 8MHz
 */
static const uint32_t FREQUENCE_DU_QUARTZ = 8ul * 1000ul * 1000ul;

/*
 * La fréquence du bus CAN
 */
static const uint32_t FREQUENCE_DU_BUS_CAN = 125ul * 1000ul;

/*
 * Deux objets pour les messages CAN en emission et en reception.
 * Par defaut les messages sont standard avec l'identifiant 0
 * et aucun octet de donnees
 */
CANMessage messageCANEmission;
CANMessage messageCANReception;

void my_setup()
{
  /* Demarre la connexion serie */
  // Serial.begin(115200);    
  
  // Serial.println("debug init ok\n");

  
  /* Demarre le SPI */
  pinMode (MCP2515_CS, OUTPUT);
  SPI.begin();
  /* Configuration du MCP2515 */
  Serial.println("Configuration du MCP2515");
  ACAN2515Settings configuration(FREQUENCE_DU_QUARTZ, FREQUENCE_DU_BUS_CAN);
  /* Loopback */
  configuration.mRequestedMode = ACAN2515Settings::NormalMode ;
  /* Demarrage du CAN */
  const uint32_t codeErreur = controleurCAN.begin(configuration, [] { controleurCAN.isr(); });
  if (codeErreur != 0) {
    // Serial.print("Erreur : 0x");
    // Serial.println(codeErreur, HEX);
    while (1);
  }
  else {
    // Serial.println("OK !");
  }


  messageCANEmission.id = 0x036 ; 
  messageCANEmission.len = 8 ;
  messageCANEmission.data32[0]=0x0E00003F;
  messageCANEmission.data32[1]=0x010000A0;
  
    
  if (controleurCAN.tryToSend(messageCANEmission)) {
      // Serial.print("envoi init");
   }  


  messageCANEmission.id = 0x0F6 ; 
  messageCANEmission.len = 8 ;
  messageCANEmission.data32[0]=0x60F6188A; // 142 24 246 96   164 88 88 16
  messageCANEmission.data32[1]=0x105858A4;
  
    
  if (controleurCAN.tryToSend(messageCANEmission)) {
      // Serial.print("envoi init2");
   }     
  delay(1000);
  messageCANEmission.id = 0x0F6 ; 
  messageCANEmission.len = 8 ;
  messageCANEmission.data32[0]=0x60F6188A; // 138 24 246 96   164 88 88 16
  messageCANEmission.data32[1]=0x105858AA;
  
    
  if (controleurCAN.tryToSend(messageCANEmission)) {
      // Serial.print("envoi init2");
   }     
   delay(1000);
  messageCANEmission.id = 0x0F6 ; 
  messageCANEmission.len = 8 ;
  messageCANEmission.data32[0]=0x0000008A; // 138 24 246 96   164 88 88 16
  messageCANEmission.data32[1]=0x00000000;
  
    
  if (controleurCAN.tryToSend(messageCANEmission)) {
      // Serial.print("envoi init2");
   }
       
   delay(5000);

    messageCANEmission.id = 0x0F6 ; 
  messageCANEmission.len = 8 ;
  messageCANEmission.data32[0]=0x0000008A;
  messageCANEmission.data32[1]=0xD0000000; 


  messageCANEmission.id = 0x0B6 ; 
  messageCANEmission.len = 8 ;
  messageCANEmission.data32[0]=0x00060040;
  messageCANEmission.data32[1]=0x99904099; //68000000
    
 controleurCAN.tryToSend(messageCANEmission);
}

void my_peugeotReceive(){
  if (controleurCAN.receive(messageCANReception)) {
    // Serial.print("recu!");
    // Serial.println(messageCANReception.id);
  }
}

void set_speed(int speed) {

  messageCANEmission.id = 0x0F6 ; 
  messageCANEmission.len = 8 ;
  messageCANEmission.data32[0]=0x0000008A;
  messageCANEmission.data32[1]=0x00000000; 

 controleurCAN.tryToSend(messageCANEmission);
 
  messageCANEmission.id = 0x0B6 ; 
  messageCANEmission.len = 8 ;
  messageCANEmission.data32[0]=0x00000000;
  messageCANEmission.data32[0]|=(speed&0xFF)<<16;
  messageCANEmission.data32[1]=0xD0000000;
 controleurCAN.tryToSend(messageCANEmission);
}


#include <Arduino.h>

class SHCustomProtocol {
private:
  int speed=0;
public:

  /*
  CUSTOM PROTOCOL CLASS
  SEE https://github.com/zegreatclan/SimHub/wiki/Custom-Arduino-hardware-support

  GENERAL RULES :
    - ALWAYS BACKUP THIS FILE, reinstalling/updating SimHub would overwrite it with the default version.
    - Read data AS FAST AS POSSIBLE in the read function
    - NEVER block the arduino (using delay for instance)
    - Make sure the data read in "read()" function READS ALL THE DATA from the serial port matching the custom protocol definition
    - Idle function is called hundreds of times per second, never use it for slow code, arduino performances would fall
    - If you use library suspending interrupts make sure to use it only in the "read" function when ALL data has been read from the serial port.
      It is the only interrupt safe place

  COMMON FUNCTIONS :
    - FlowSerialReadStringUntil('\n')
      Read the incoming data up to the end (\n) won't be included
    - FlowSerialReadStringUntil(';')
      Read the incoming data up to the separator (;) separator won't be included
    - FlowSerialDebugPrintLn(string)
      Send a debug message to simhub which will display in the log panel and log file (only use it when debugging, it would slow down arduino in run conditions)

  */

  // Called when starting the arduino (setup method in main sketch)



  


void setup() {
      my_setup();
  }

  

// Called when new data is coming from computer
  void read() {
    // EXAMPLE 1 - read the whole message and sent it back to simhub as debug message
    // Protocol formula can be set in simhub to anything, it will just echo it
    // -------------------------------------------------------
    speed = FlowSerialReadStringUntil('\n').toInt();
    FlowSerialDebugPrintLn("Message received : " + String(speed));
  
    /*
    // -------------------------------------------------------
    // EXAMPLE 2 - reads speed and gear from the message
    // Protocol formula must be set in simhub to
    // format([DataCorePlugin.GameData.NewData.SpeedKmh],'0') + ';' + isnull([DataCorePlugin.GameData.NewData.Gear],'N')
    // -------------------------------------------------------

    int speed = FlowSerialReadStringUntil(';').toInt();
    String gear = FlowSerialReadStringUntil('\n');

    FlowSerialDebugPrintLn("Speed : " + String(speed));
    FlowSerialDebugPrintLn("Gear : " + gear);
    */
  }

  // Called once per arduino loop, timing can't be predicted, 
  // but it's called between each command sent to the arduino





  void loop() {
  uint32_t my_speed=speed*100;
  my_peugeotReceive();
  
  messageCANEmission.id = 0x0F6 ; 
  messageCANEmission.len = 8 ;
  messageCANEmission.data32[0]=0x0000008A;
  messageCANEmission.data32[1]=0x00000000; 
  controleurCAN.tryToSend(messageCANEmission);

  messageCANEmission.id = 0x0B6 ; 
  messageCANEmission.len = 8 ;
  messageCANEmission.data32[0]=0x00000000;
  messageCANEmission.data32[0]|=(my_speed&0xFF00)<<8;
  messageCANEmission.data32[0]|=(my_speed&0xFF)<<24;

  messageCANEmission.data32[1]=0xD0000000;
  controleurCAN.tryToSend(messageCANEmission);

// FlowSerialDebugPrintLn("SetSpeed  : " + String(my_speed) + String(messageCANEmission.data32[0]));

  }

  

  // Called once between each byte read on arduino,
  // THIS IS A CRITICAL PATH :
  // AVOID ANY TIME CONSUMING ROUTINES !!!
  // PREFER READ OR LOOP METHOS AS MUCH AS POSSIBLE
  // AVOID ANY INTERRUPTS DISABLE (serial data would be lost!!!)
 
  void idle() {
  }
};
#endif