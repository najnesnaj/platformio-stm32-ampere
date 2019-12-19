#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

#define iddAddr 0x42

#define EPD_CS       PA15
#define EPD_DC       PB11
#define EPD_RESET    PB2
#define EPD_BUSY     PA8
#define EPD_POWER    PB10
#define EPD_SPI_MOSI PB5
#define EPD_SPI_MISO PB4
#define EPD_SPI_SCK  PB3

U8G2_SSD1606_172X72_F_4W_SW_SPI u8g2(U8G2_R0, EPD_SPI_SCK, EPD_SPI_MOSI, EPD_CS, EPD_DC, EPD_RESET);

volatile int stron;
String power = "No Data";

void setup(void) {
  pinMode(EPD_POWER, OUTPUT);
  digitalWrite(EPD_POWER, 0);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  Serial.println("ATCnetz.de");
  Wire.begin();
  pinMode(PC13, INPUT);
  Idd_Init();
  Idd_req_meas( 100 );
  attachInterrupt(digitalPinToInterrupt(PC13), getmessaure, RISING);
  u8g2.begin();
}

void loop(void) {
  u8g2.clearBuffer();
  u8g2.setFontMode(1);
  u8g2.setFont(u8g2_font_cu12_tr);
  u8g2.setCursor(0, 15);
  u8g2.print("Amperemeter:");
  u8g2.setFont(u8g2_font_profont29_tf );
  u8g2.setCursor(0, 45);
  u8g2.println(power);
  u8g2.setFont(u8g2_font_cu12_tr);
  u8g2.setCursor(0, 70);
  u8g2.print("Runtime:");
  u8g2.print(millis()/60000);
  u8g2.print(" min");
  u8g2.sendBuffer();
  delay(3000);
  Serial.println("ATCnetz.de");
}

void getmessaure(  )
{
  int meas = Idd_get_meas();
  char str[12];
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));

  if ( meas < 0 )
  {
    return;
  }
  stron = meas;
  sprintf( (char*)str, "%d.%.3dmA", (int)meas / 100000, ((int)meas % 100000) / 100 );
  if ( str[0] == '0' )
  {
    sprintf( (char*)str, "%d.%.3duA", (int)meas / 100, ((int)meas % 100) * 10 );
    if ( str[0] == '0' )
    {
      sprintf( (char*)str, " %dnA ", (int)meas * 10 );
    }
  }
  power = str;
  Serial.println(str);
  Idd_req_meas( 100 );
}

void Idd_req_meas( uint8_t predelay )
{
  uint8_t param;
  predelay |= 0x80; // IDD_PRE_DELAY |= IDD_PREDELAY_20_MS
  I2C_Write_Reg(iddAddr, 0x81, &predelay, 1 ); //add predelay before Idd measurement
  param = 0x09; // IDD_CTRL = ( ( 4 << 1 ) & IDD_CTRL_SHUNT_NB ) | IDD_CTRL_REQ
  I2C_Write_Reg(iddAddr, 0x80, &param, 1 ); // request Idd measurement
}

int Idd_get_meas( void )
{
  uint8_t temp[3];
  uint8_t ack;
  I2C_Read_Reg( iddAddr, 0x08, temp, 1 );
  if ( temp[0] & 0x04 )
  {
    Idd_Init();
    return -1;
  }
  I2C_Read_Reg( iddAddr, 0x14, temp, 3 );
  ack = 0x02;
  I2C_Write_Reg(iddAddr, 0x44, &ack, 1 );
  return (temp[0] << 16) + (temp[1] << 8) + temp[2];
}

void I2C_Read_Reg(int addr, int start, uint8_t *buffer, int size)
{
  int i;
  Wire.beginTransmission(addr);
  Wire.write(start);
  Wire.endTransmission();
  Wire.requestFrom(addr, size, true);
  i = 0;
  while (Wire.available() && i < size)
  {
    buffer[i++] = Wire.read();
  }
}

int I2C_Write_Reg(int addr, int start, uint8_t *pData, int size)
{
  Wire.beginTransmission(addr);
  Wire.write(start);
  Wire.write(pData, size);
  Wire.endTransmission();
}

void Idd_Init( void )
{
  uint8_t params[14];
  params[0] = 0x80;
  I2C_Write_Reg(iddAddr, 0x40, params, 1 );
  delay( 100 );
  params[0] = 0x03;
  I2C_Write_Reg(iddAddr, 0x41, params, 1 );
  delay( 1 );
  params[0] = 0x06;
  I2C_Write_Reg(iddAddr, 0x42, params, 1 );
  params[0] = 0x04;
  I2C_Write_Reg(iddAddr, 0x40, params, 1 );
  params[0] = 0x03; params[1] = 0xE8;   // SH0 = 1000 mohm
  params[2] = 0x00; params[3] = 0x18;   // SH1 = 24 ohm
  params[4] = 0x02; params[5] = 0x6C;   // SH2 = 620 ohm
  params[6] = 0x00; params[7] = 0x00;   // SH3 = not included
  params[8] = 0x27; params[9] = 0x10;   // SH4 = 10,000 ohm
  params[10] = 0x13; params[11] = 0x7E; // Gain = 49.9 (4990)
  params[12] = 0x0B; params[13] = 0xB8; // VDD_MIN = 3000 mV
  I2C_Write_Reg(iddAddr, 0x82, params, 14 );
}
