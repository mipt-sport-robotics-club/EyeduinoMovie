#define USE_MONOPLANE_ARRAY

#include "movie01.h"
#include "EyeduinoMovie.h"

#define CUSTOM_BIT 5

#define EYDUINO_IMAGE_SIGNATURE "{EYNOIM}"
#define EYDUINO_IMAGE_END_TAG "\n"

char encoded[3];

char *codes = "0123456789ABCDEF";
void encodeByte(byte _raw){
  encoded[0] = codes[(_raw & 0xF0) >> 4];
  encoded[1] = codes[_raw & 0x0F];
  encoded[2] = '\0';
}

void storeImageFromMonoPlane() {
  char _strBuffer[ROWS_PER_IMAGE*PIXELS_PER_ROW/8*2+1];
  byte _row;
  byte _pixels;
  int _strIndex=0;
  //очищаем строковый буфер
  for(_strIndex=0;_strIndex<=ROWS_PER_IMAGE*PIXELS_PER_ROW/8*2;_strIndex++){
    _strBuffer[_strIndex]=0x00;
  } 
  // инициализируем индекс
  _strIndex=0;
  for(_row=0;_row<ROWS_PER_IMAGE;_row++){
    for(_pixels=0;_pixels<PIXELS_PER_ROW/8;_pixels++){
        encodeByte(elimplImageMonoPlane[_row][_pixels]);
        _strBuffer[_strIndex]=encoded[0];
        _strIndex++;
        _strBuffer[_strIndex]=encoded[1];
        _strIndex++;
    }  
  }
  Serial.print(EYDUINO_IMAGE_SIGNATURE);
  Serial.print(_strBuffer);
  Serial.print(EYDUINO_IMAGE_END_TAG);
}

void setup(){
  elInit();
  Serial.begin(115200);
}

void loop(){
  elRequestImage();
  while(!elIsImageReady()){
    delay(1);
  }
  storeImageFromMonoPlane();
  delay(1);
}
