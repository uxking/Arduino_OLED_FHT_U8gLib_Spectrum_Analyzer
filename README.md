# Arduino OLED FHT U8gLib Spectrum Analyzer
Arduino based Spectrum Analyzer with an .96" OLED display (SSD1306).  
Button used to switch between different bar sizes 12, 8, and 6.  

Utilized U8glib.h for the display.  

Speaker input used for signal.  

FHT library for frequency spectrum analyser http://wiki.openmusiclabs.com/wiki/ArduinoFHT  

Using GreyGnome's PinChangeInt to change detect button push and change number of bars displayed: https://github.com/GreyGnome/PinChangeInt  
Also found some code by the-fuchs on Github to help with the peak hold display. I believe his code was working with an MSGEQ7 7 band chip, but I was able to use some of his magic in my sketch.  
https://github.com/the-fuchs/AVR-7-BandAudioSpectrum check out main.c and his drawDouble or drawSingle functions.
