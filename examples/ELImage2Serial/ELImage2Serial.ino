#include "movie01.h"
#include "EyeduinoMovie.h"

#define EYDUINO_IMAGE_SIGNATURE "{EYNOIM}"
#define EYDUINO_IMAGE_END_TAG "\n"

char encoded[3];

char *codes = "0123456789ABCDEF";
void encodeByte(byte _raw){
  encoded[0] = codes[(_raw & 0xF0) >> 4];
  encoded[1] = codes[_raw & 0x0F];
  encoded[2] = '\0';
}
  
//const byte _bitmask[8] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};  
const byte _bitmask[8] = {0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};  
void storeImage() {
  char _shortBuffer;
  char _strBuffer[ROWS_PER_IMAGE*PIXELS_PER_ROW/8*2+1];
  byte _row;
  byte _pixel;
  int _strIndex=0;
  int _charIndex=0;
  int _bitIndex=-1;
  //очищаем строковый буфер
  for(_charIndex=0;_charIndex<ROWS_PER_IMAGE*PIXELS_PER_ROW/8*2;_charIndex++){
    _strBuffer[_charIndex]=0x00;
  }  
  // инициализируем индекс и буфер
  _charIndex=0;
  _shortBuffer=0x00;
  // пробегаем по всей картинке
  for(_row=0;_row<ROWS_PER_IMAGE;_row++){
    for(_pixel=0;_pixel<PIXELS_PER_ROW;_pixel++){
      _bitIndex++;
      // если чёрное, помечаем соответствующий бит
      if(eliaIsBlack(_row,_pixel)){
        _shortBuffer=_shortBuffer | _bitmask[_bitIndex];
      }
      // если восьмой бит, скидываем информацию
      if(_bitIndex>=7){
        _bitIndex=-1;
        encodeByte(_shortBuffer);
        _strBuffer[_strIndex]=encoded[0];
        _strIndex++;
        _strBuffer[_strIndex]=encoded[1];
        _strIndex++;
        _shortBuffer=0x00;        
      }
    }  
  }
//  _bitIndex++;
//  _shortBuffer=_shortBuffer | _bitmask[_bitIndex];
//  encodeByte(_shortBuffer);
//  _strBuffer[_strIndex]=encoded[0];
//  _strIndex++;
//  _strBuffer[_strIndex]=encoded[1];
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
  storeImage();
  delay(10);
}
