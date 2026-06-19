# Tree for dEIn Labor https://www.tu.berlin/dein-labor

## IMPORTANT!!!  
in my vscode workspace, the platformio project's files are NOT linked to project git repo!  

-> work in platformio project
-> when done/made process, sync  

(got too annoyed to actually get it working)

## some notes

// https://github.com/FastLED/FastLED/wiki/
// https://github.com/FastLED/FastLED/wiki/Power-notes
// >Some of these are just good engineering. Some are just plain voodoo. Use whichever ones that work for you.
// > - At the 'far' end of long LED strips, connect the Data line to Ground.
// > - Insert a 200 ohm resistor between the output pins on the microcontroller and the inputs (data, clock) on the LED strip.
// > - Use a level-shifter to raise the voltage of the output pins' 3.3 volts to a full 5 volts before sending it into the LED strip's data (and/or clock) inputs.
// ws2812b datasheet: https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf
// sk6812 datasheet: https://cdn-shop.adafruit.com/product-files/1138/SK6812+LED+datasheet+.pdf 
