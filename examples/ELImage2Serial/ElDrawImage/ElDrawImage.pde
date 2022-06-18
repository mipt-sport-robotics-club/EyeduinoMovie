import processing.serial.*;

String strImage="{EYNOIM}FFFFFFFFFFFFFFFFFFFFFFFF00000000000000000000000000000000000001FC000006A700001A0000003C0000002000000050000000D0000FBFFFFF0E6B72750000FC000000FC000000FC000000FC000000FC000001FC000001FE000001FC000001FC000001FE000001FE000001FE000003FE000001FF000001FF000003FF0080";
String IMAGE_START_TAG = "{EYNOIM}";
char IMAGE_END_TAG = 0x0A;

final int ROW_COUNT = 32;
final int PIXELS_PER_ROW = 32;

boolean [][]_image = new boolean[ROW_COUNT][PIXELS_PER_ROW];
Serial myPort = null;

int prevMillis = 0;
float fRate = 0;

final int PIXEL_SIZE = 15;
final int MARGINE = 10;
final int BORDER_WIDTH = 4; 

final int LEGEND_WIDTH = 100;
final int LEGEND_FONT_HEIGHT = 24;


void settings(){
//  fullScreen();
  size(PIXELS_PER_ROW*PIXEL_SIZE+BORDER_WIDTH+MARGINE*2, ROW_COUNT*PIXEL_SIZE+BORDER_WIDTH+2*MARGINE*3+LEGEND_FONT_HEIGHT);
}

void setup(){
  printArray(Serial.list());
  if(Serial.list().length>0){
    String portName = Serial.list()[0];
    myPort = new Serial(this, portName, 115200);
    myPort.bufferUntil(IMAGE_END_TAG);
  }
}

void serialEvent(Serial p) {
  String _strBuff = p.readString();
  int _imgStrIndex = _strBuff.indexOf(IMAGE_START_TAG);
  if(_imgStrIndex>=0){
    strImage = _strBuff.substring(_imgStrIndex);
//    println(_imgStrIndex);
    fRate = 1000/(millis()-prevMillis);
    prevMillis = millis();
    redraw();
  }
}

void decodeImage(){
  int _bitIndex=0;
  int _rowIndex=0;
  int _pixelIndex=0;
  String _buff;
  int pixelCount = (strImage.length()-IMAGE_START_TAG.length())/2;
//  println(strImage.length()); 
//  println(pixelCount);
  for(int i = 0;i<pixelCount;i++){
    _buff=binary(unhex(strImage.substring(IMAGE_START_TAG.length()+i*2,IMAGE_START_TAG.length()+i*2+2)),8);
    for(_bitIndex=0;_bitIndex<8;_bitIndex++){
      if(_buff.charAt(_bitIndex)=='1') {
        _image[_rowIndex][_pixelIndex]=true;
      }
      else {
        _image[_rowIndex][_pixelIndex]=false;
      }
      _pixelIndex++;
//      println(_pixelIndex);
    }
    if(_pixelIndex>=PIXELS_PER_ROW-1){
      _pixelIndex=0;
      _rowIndex++;
    }
    if(_rowIndex>=ROW_COUNT-1){
//      println(strImage.substring(IMAGE_START_TAG.length()+i*2));
      break;
    }
//    println(_buff);
  }
}

void draw(){
  background(255);
  fill(#FFFFFF);
  stroke(255,0,0);
  strokeWeight(4);
  rect(MARGINE,MARGINE,PIXELS_PER_ROW*PIXEL_SIZE+BORDER_WIDTH,ROW_COUNT*PIXEL_SIZE+BORDER_WIDTH);
  stroke(0,0,0);
  strokeWeight(1);
  fill(#000000);
  decodeImage();
  for(int _row=0;_row<ROW_COUNT;_row++){
    for(int _pixel=0;_pixel<PIXELS_PER_ROW;_pixel++){
      if(_image[_row][_pixel]){
        rect(MARGINE+BORDER_WIDTH/2-1+_pixel*PIXEL_SIZE,MARGINE+BORDER_WIDTH/2+_row*PIXEL_SIZE,PIXEL_SIZE,PIXEL_SIZE);
      }
    }
  }
  text((new String("Frame rate: \t"))+str(fRate),MARGINE,ROW_COUNT*PIXEL_SIZE+BORDER_WIDTH+2*MARGINE,LEGEND_WIDTH,LEGEND_FONT_HEIGHT);
}
