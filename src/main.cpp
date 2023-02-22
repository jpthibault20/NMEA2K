#include <Arduino.h>
#include <SPI.h>
#include "LTG_lib_CLAVIER.hpp"

// creation objet LCD
#include <LCD_I2C.h>
LCD_I2C lcd(0x27, 16, 2);

// création objet MCP2515
#include "LTG_lib_MCP2515.h"
MCP2515_LTG_class MCP2515_1((int8_t)10);

// variable globale
uint8_t res_tmp;
uint16_t angle = 0;
unsigned char stmp[8] = {0, 0, 0, 0, 0, 0, 0, 0};

#define _standby 0
#define _envoi_1x 1
#define _envoi_continu 2
#define _activation_filtre 3
#define _DESactivation_filtre 4
uint8_t mode = _standby;

uint32_t ID = 0;
uint8_t EXT = 0;
uint8_t RTR = 0;
uint8_t LEN = 0;
uint8_t DATA[8];
uint8_t Buf_N = 2;

uint32_t tps = 0;
bool envoie = 0;

#define maxindex 25
uint8_t N2K_PRIORITE[maxindex];
uint32_t N2K_PGN[maxindex];
uint8_t N2K_DESTINATAIRE[maxindex];
uint8_t N2K_ADRESSE[maxindex];
uint8_t N2K_DATA[maxindex][8];

// creation objet CLAVIER
CLAVIER_LTG_class clavier;

void setup()
{
  { // init clavier
    clavier.init(4, 3, 5, 6, 2);
  }

  { // init port série
    Serial.begin(115200);
    while (!Serial)
    {
      ; // wait for serial port to connect. Needed for native USB port only
    }
    Serial.println(F("Serial int OK !"));
  }

  { // INIT lcd
    lcd.begin();
    lcd.backlight();
    lcd.clear();
    lcd.print(F("Init. LCD -> OK!"));
    delay(1000);
  }

  { // init MCP2515
    lcd.clear();
    lcd.print(F("Init. MCP2515..."));

    res_tmp = MCP2515_1.MCP2515_init(CAN_500KBPS, 7);
    if (res_tmp != MCP2515_OK)
    {
      lcd.clear();
      lcd.print("ERREUR n:");
      lcd.print(res_tmp);
      while (1)
        ;
    }

    delay(100);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Init. MCP2515 OK"));
    delay(1000);
  }
}

void loop()
{
  {// partie réception message
    //if (MCP2515_1.MCP2515_init_Filtrage_PGN(0, 127250) != MCP2515_OK) // permet d'afficher uniquement les messages avec le PGN voulue
    //{
    //  lcd.clear();
    //  lcd.print("ERR Filtre");
    //  while (1)
    //    ;
    //}

    if (envoie == 0)
    {
      stmp[0] = 0;   // unique number assigned by manufacturer
      stmp[1] = 0;   // unique number assigned by manufacturer
      stmp[2] = 5;   // unique number assigned by manufacturer
      stmp[3] = 3;   // unique number assigned by manufacturer + manufactured code
      stmp[4] = 9;   // device instance
      stmp[5] = 140; // device fonction
      stmp[6] = 20;  // device classe
      stmp[7] = 9;   // systeme instance + industry group

      if (MCP2515_1.MCP2515_N2K_sendMsg(6, 60928, 7, 1, stmp) == MCP2515_OK) {
        Serial.println(F("envoie OK   !!!!!"));
      }

      tps = millis();
      envoie = 1;

      //if (res_tmp != MCP2515_OK)
      //{
      //  lcd.clear();
      //  lcd.print("Erreur envoi");
      //  lcd.print(res_tmp);
      //  while (1)
      //    ;
      //}
    }

    

    if (MCP2515_1.MCP2515_readMsg(ID, EXT, RTR, LEN, DATA, Buf_N) == MCP2515_OK_msg_aviable)
    {
      MCP2515_1.MCP2515_N2K_decodage_ID(ID, N2K_PRIORITE[0], N2K_PGN[0], N2K_ADRESSE[0]);
      lcd.clear();

      //lcd.setCursor(0, 0);
      //lcd.print("  PGN : ");
      //lcd.print(N2K_PGN[0]);

      //if (N2K_PGN[0] == 127250)
      //{
      //  angle = (DATA[2] * 256. + DATA[1]) * 0.0057; // 256. pour spécifier que nous faisont un callul avec des doubles

      //  lcd.setCursor(0, 1);
      //  lcd.print("    data : ");
      //  lcd.print(angle);
      //}

      Serial.println("\n\nN2K_Prio | N2K_pgn | N2K_destinataire | N2K_adresse");
      Serial.print(N2K_PRIORITE[0]);
      Serial.print(F("        | "));
      Serial.print(N2K_PGN[0]);
      Serial.print(F("  | "));
      Serial.print(N2K_DESTINATAIRE[0]);
      Serial.print(F("                | "));
      Serial.print(N2K_ADRESSE[0]);

      Serial.println(F("\n\nID        | EXT | RTR | LEN | DATA"));
      Serial.print(ID);
      Serial.print(F(" | "));
      Serial.print(EXT);
      Serial.print(F("   | "));
      Serial.print(RTR);
      Serial.print(F("   | "));
      Serial.print(LEN);
      Serial.print(F("   | "));

      for (int i = 0; i < 8; i++)
      {
        Serial.print(" ");
        Serial.print(DATA[i]);
      }

      Serial.println(F("\n\n\n-----------------------------------------------"));
    }
    //else
    //{
    //  lcd.clear();
    //  lcd.setCursor(0, 0);
    //  lcd.print("   NO MESSAGE");
    //}
  }

  if (millis() > tps + 1000)   while(1);

  /*{ // partie envoie message (profondeur)

    stmp[0] = 255; // Sequence ID (une seule trame donc 255)
    stmp[1] = 40;  // Water Depth, Transducer(distance entre le sonnar et le fond, MSB et LSB inversé)
    stmp[2] = 8;   // Water Depth, Transducer
    stmp[3] = 1;   // Water Depth, Transducer
    stmp[4] = 0;   // Water Depth, Transducer
    stmp[5] = 0;   // Offset (distance entre la ligne de flotaison et le sonnar)
    stmp[6] = 0;   // Offset
    stmp[7] = 252; // Maximum Depth Range (valeure max du capteur)

    res_tmp = MCP2515_1.MCP2515_N2K_sendMsg(6, 128267, 0, 0, stmp);

    if (res_tmp != MCP2515_OK)
    {
      lcd.clear();
      lcd.print("Erreur envoi");
      lcd.print(res_tmp);
      while (1)
        ;
    }
  }*/

  //delay(100);
}