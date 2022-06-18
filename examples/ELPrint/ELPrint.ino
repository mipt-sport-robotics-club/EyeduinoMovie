#include "movie01.h"
#include "EyeduinoMovie.h"

void setup(){
  elInit();
  Serial.begin(115200);
}

void loop(){
  elRequestImage();
  while(!elIsImageReady()){
    delay(1);
  }
  showImage();
  delay(100);
}

void showImage() {
  int i=0;
  int j=0;
  char imgRow[PIXELS_PER_ROW+1];
  Serial.println("--------------------------------");
  for(i=0;i<ROWS_PER_IMAGE;i++){
    for(j=0;j<PIXELS_PER_ROW;j++){
      if(!eliaIsDefined(i,j)) {
        imgRow[j]='.';
      }
      else {
        if(eliaIsBlack(i,j)) {
          imgRow[j]='#';
        }
        else{
          imgRow[j]=' ';
        }
      }
    }
    imgRow[PIXELS_PER_ROW]='\0';
//    Serial.print(i);
//    Serial.print('\t');
    Serial.println(imgRow);
  }
  Serial.println("--------------------------------");
}
