/* 
 * File:   EyeduinoMovie.h
 * Author: Eduard Petrenko
 *
 * Created on 2022 April
 * Update 
 * 
 * EyeduinoMovie uses allows operation with image using API similar to EyeduinoLight one.
 * Lifecicle API and image data access functions have the same names and call parameters
 * Image data is storied into MCU PROGMEM
 */

#ifndef EYEDUINO_MOVIE_H
#define	EYEDUINO_MOVIE_H

#include <Arduino.h>

//#ifdef EYEDUINO_LIGHT_H
//#error "EyeduinoLight can not be used instantly with EyeduinoMovie."
//#endif //_EYEDUINO_CFG

#define IMAGE_STATUS_INITIAL 0
#define IMAGE_STATUS_REQUESTED 1
#define IMAGE_STATUS_READING 2
#define IMAGE_STATUS_DONE 3
#define IMAGE_STATUS_ERROR -1

// lifecicle functions
void elInit();
void elRequestImage();
boolean elIsImageReady();

//image access functions
boolean eliaIsDefined(int _row, int _pixel);
boolean eliaIsBlack(int _row, int _pixel);
byte eliaIsBlackAsByte(int _row, int _pixel);

//custom byts (except 2 leftmost bits) access functions
void eliaSetCustomByte(int _row, int _pixel, byte _byte);
byte eliaGetCustomByte(int _row, int _pixel);

//custom bits access functions
void eliaSetCustomBit(int _row, int _pixel, int _bit, boolean _value);
boolean eliaGetCustomBit(int _row, int _pixel, int _bit);
byte eliaGetCustomBitAsByte(int _row, int _pixel, int _bit);
#endif	/* EYEDUINO_MOVIE_H */

int elimplImageReadingStatus = IMAGE_STATUS_INITIAL;
byte elimplImage[ROWS_PER_IMAGE][PIXELS_PER_ROW];

#ifdef USE_MONOPLANE_ARRAY
#define LINE_POSTPROC
#endif

#ifdef POSTPROC_COPY2USERBIT5
#define LINE_POSTPROC
#endif

#ifdef USE_MONOPLANE_ARRAY
	volatile byte elimplImageMonoPlane[ROWS_PER_IMAGE][PIXELS_PER_ROW/8];
	volatile int elimplMPCurByte = 0;
	volatile int elimplMPCurBit = -1;
//	volatile int elimplMPCurPixel = 0;
	volatile byte elimplMPBuffer = 0x00;
#endif

//byte elimplCurRow;
//byte elimplCurPixel;
//int elimplCurLine;
int emimpCurFrame;
long emimpFrameByTime;
long emimpFrameByTimeNext;

unsigned long microsFirstFrameRequested = 0;
unsigned long microsCurrFrameRequested;

#define MICROS_PER_FRAME 40000
#define MICROS_BETWEEN_FRAMES 20000 //=MICROS_PER_10_LINES*ROWS_PER_IMAGE
#define MICROS_PER_10_LINES 625

// lifecicle functions

void elInit() {
	unsigned long firstLine = movie[0][0];
	microsFirstFrameRequested = 0;
	
}

#define AFTER_MOVIE_BEHAVIOUR_REPEAT 1
#define AFTER_MOVIE_BEHAVIOUR_LAST_FRAME 2
#define AFTER_MOVIE_BEHAVIOUR_STOP 3

byte emimpAfterMovieBehaviour = AFTER_MOVIE_BEHAVIOUR_STOP;

void elRequestImage() {
	elimplImageReadingStatus = IMAGE_STATUS_REQUESTED;
	microsCurrFrameRequested = micros();
	if (microsFirstFrameRequested == 0) {
		microsFirstFrameRequested = microsCurrFrameRequested;
		emimpFrameByTime = 0;
		emimpFrameByTimeNext = 0;
		emimpCurFrame = 0;
	}
	else {
		emimpFrameByTime = (microsCurrFrameRequested - microsFirstFrameRequested) / MICROS_PER_FRAME;
		if (emimpFrameByTime > emimpFrameByTimeNext) {
			emimpFrameByTimeNext = emimpFrameByTime;
		}
		emimpCurFrame = emimpFrameByTimeNext;
		if (emimpFrameByTimeNext >= FRAMES_PER_MOVIE) {
			switch (emimpAfterMovieBehaviour) {
			case AFTER_MOVIE_BEHAVIOUR_REPEAT:
				emimpCurFrame = emimpFrameByTimeNext % FRAMES_PER_MOVIE;
				break;
			case AFTER_MOVIE_BEHAVIOUR_LAST_FRAME:
				emimpCurFrame = FRAMES_PER_MOVIE - 1;
				break;
			case AFTER_MOVIE_BEHAVIOUR_STOP:
				emimpCurFrame = -1;
				break;
			}
		}
	}
	//fill all image with undefined values
	for (byte _elimplCurRow = 0; _elimplCurRow < ROWS_PER_IMAGE; _elimplCurRow++) {
		for (byte _elimplCurPixel = 0; _elimplCurPixel < PIXELS_PER_ROW ; _elimplCurPixel++) {
			elimplImage[_elimplCurRow][_elimplCurPixel] = elimplImage[_elimplCurRow][_elimplCurPixel] & 0x3F;
		}
	}
}

//if there where enough micros to fill data
boolean elIsImageReady() {
	if (emimpCurFrame < 0) {
		return false;
	}
	if (micros() + MICROS_PER_10_LINES * ROWS_PER_IMAGE > microsFirstFrameRequested + emimpFrameByTimeNext * MICROS_PER_FRAME ) {
		elimplImageReadingStatus = IMAGE_STATUS_DONE;
		// fill data and do postprocessing for each line
		for (byte _elimplCurRow = 0; _elimplCurRow < ROWS_PER_IMAGE; _elimplCurRow++) {
			unsigned long _data = pgm_read_dword(&movie[emimpCurFrame][_elimplCurRow]);
			byte _elimplCurPixel = 0;
			for (unsigned long _mask = 0x80000000; _mask != 0; _mask = _mask >> 1) {
				if ((_mask & _data) > 0) { // if white pixel
					elimplImage[_elimplCurRow][_elimplCurPixel] = elimplImage[_elimplCurRow][_elimplCurPixel] | 0xC0;
				}
				else {
					elimplImage[_elimplCurRow][_elimplCurPixel] = (elimplImage[_elimplCurRow][_elimplCurPixel] | 0x80) & 0xBF;
				}
				_elimplCurPixel++;
			}
			// Line postprocessing routines

#ifdef USE_MONOPLANE_ARRAY
			elimplMPCurByte = 0;
			elimplMPCurBit = -1;
			elimplMPBuffer = 0x00;
#endif

#ifdef LINE_POSTPROC

			for (int elimplPPCurPixel = 0; elimplPPCurPixel < PIXELS_PER_ROW; elimplPPCurPixel++) {

#ifdef USE_MONOPLANE_ARRAY
				elimplMPCurBit++;
				elimplMPBuffer = elimplMPBuffer | (((elimplImage[_elimplCurRow][elimplPPCurPixel] & 0x40) << 1) >> elimplMPCurBit);
				if (elimplMPCurBit >= 7) {
					elimplImageMonoPlane[_elimplCurRow][elimplMPCurByte] = elimplMPBuffer;
					elimplMPCurByte++;
					elimplMPBuffer = 0x00;
					elimplMPCurBit = -1;
				}
#endif //USE_MONOPLANE_ARRAY

#ifdef POSTPROC_COPY2USERBIT5
				elimplImage[_elimplCurRow][elimplPPCurPixel] = (elimplImage[_elimplCurRow][elimplPPCurPixel] & 0xDF) | ((elimplImage[_elimplCurRow][elimplPPCurPixel] & 0x40) >> 1);
#endif

			}

#endif //LINE_POSTPROC

		}
	}
	if (elimplImageReadingStatus == IMAGE_STATUS_DONE) {
		emimpFrameByTimeNext++;
		return true;
	}
	else {
		return false;
	}
}


//image access functions
boolean eliaIsDefined(int _row, int _pixel){
	if ((elimplImage[_row][_pixel] & 0x80) == 0){
      return false;
    }
    else{
      return true;
    }
}

boolean eliaIsBlack(int _row, int _pixel){
    if((elimplImage[_row][_pixel] & 0x40)>0){
      return true;
    }
    else{
      return false;
    }
}

byte eliaIsBlackAsByte(int _row, int _pixel){
	byte _val = elimplImage[_row][_pixel];
	return (_val >> 6) & 0x01;
}

// set all content of image bytes except 2 leftmost bits
void eliaSetCustomByte(int _row, int _pixel, byte _byte){
	elimplImage[_row][_pixel] = (elimplImage[_row][_pixel] & 0xC0) | (_byte & 0x3F);
}

byte eliaGetCustomByte(int _row, int _pixel){
	return (elimplImage[_row][_pixel] & 0x3F);
}

// set all content of image bytes except 2 leftmost bits
void eliaSetCustomBit(int _row, int _pixel, int _bit, boolean _value){
	if ((_bit > 5) || (_bit < 0)){
		return;
	}
	if (_value){
		byte _b = 0x01;
		_b = _b << _bit;
		elimplImage[_row][_pixel] = elimplImage[_row][_pixel] | _b;
	}
	else{
		byte _b = 0x01;
		_b = _b << _bit;
		elimplImage[_row][_pixel] = elimplImage[_row][_pixel] & (~_b);
	}
}

boolean eliaGetCustomBit(int _row, int _pixel, int _bit){
	if ((_bit > 5) || (_bit < 0)){
		return false;
	}
	byte _b = 0x01;
	if ((elimplImage[_row][_pixel] & (_b << _bit)) > 0){
		return true;
	}
	else{
		return false;
	}
}

// returns 0 or 1
byte eliaGetCustomBitAsByte(int _row, int _pixel, int _bit){
	if ((_bit > 5) || (_bit < 0)){
		return 0;
	}
	byte _val = elimplImage[_row][_pixel];
	return ((_val >> _bit) & 0x01);
}
