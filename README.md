# EyeduinoMovie
Arduino library for operation with recorded into PROGMEM video from analog camera
EyeduinoMovie.h is library itself
movie01.h is the first video with Mysterious White Spot
examples:
- ELImage2Serial encodes movie frames into byte stream that can be interpreted and shown by ElDrawImage Processing programm (included)
- ELPrint encodes movie frames as ASCII images that can be viewed directly in Arduino IDE Serial Monitor tool
- EMImage2Serial_MONOPLANE has the same functionality as ELImage2Serial but uses USE_MONOPLANE_ARRAY feature of library
