# Tree for dEIn Labor https://www.tu.berlin/dein-labor

## IMPORTANT!!!
This repo now contains both variants:

- `src/esp32_ledTree_Karl.ino` for Arduino IDE users (simply open this via Arduino IDE (let it create a sketch folder)).
- `src/usingPlatformIO/main.cpp` for PlatformIO (advanced users).

The repo root also has `platformio.ini`, so PlatformIO builds directly from the repo via `src/usingPlatformIO`.


## some notes

### esp32 devkit v1 type-c 

Important: [the original espressif devkits](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32/esp-dev-kits-en-master-esp32.pdf#e) came with 38 pins, then there were some clones with 36, and finally the 30 pin version became the defacto standard for clones one would buy online. Make sure you know which one you're using to avoid confusion!  
The ones we had available and provided are 30 pin clones using CP210x USB to UART bridge (make sure to download the drivers, following for example [this tutorial](https://randomnerdtutorials.com/install-esp32-esp8266-usb-drivers-cp210x-windows/)). 

#### getting started: 

there's plenty of tutorials, but this is the one I'd most recommend: https://randomnerdtutorials.com/getting-started-with-esp32/  

#### pin references:

you could use the official documentation, but it's probably a bit too technical, so here are some more accessible alternatives

a fairly extensive reference:  
https://lastminuteengineers.com/esp32-pinout-reference/  

fancier looking site with 30pin board: 
https://www.espboards.dev/esp32/esp32-30pin-devkit-generic/#board-pinout  
I haven't fully checked out the correctness of the site's information, but it looks pretty good. They also identified the [safe pins to use](https://www.espboards.dev/esp32/esp32-30pin-devkit-generic/#board-pinout:~:text=Safe%20Pins%20to%20Use) (my example plug and play interface uses GPIOs 27, 26, 25, 33, 32 -> the exact same ones they recommend)

nice reference for which pins to use / not to use:  
https://randomnerdtutorials.com/esp32-pinout-reference-gpios/  
One can rarely go wrong with randomnerdtutorials when it comes to arduino stuff, even if some of it might be a bit old or even outdated. Then again, so is the esp32 at this point :D 
WARNING: their pinout shows the original 36pin version board!  

### LED stuff

// https://github.com/FastLED/FastLED/wiki/
// https://github.com/FastLED/FastLED/wiki/Power-notes
// >Some of these are just good engineering. Some are just plain voodoo. Use whichever ones that work for you.
// > - At the 'far' end of long LED strips, connect the Data line to Ground.
// > - Insert a 200 ohm resistor between the output pins on the microcontroller and the inputs (data, clock) on the LED strip.
// > - Use a level-shifter to raise the voltage of the output pins' 3.3 volts to a full 5 volts before sending it into the LED strip's data (and/or clock) inputs.
// ws2812b datasheet: https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf
// sk6812 datasheet: https://cdn-shop.adafruit.com/product-files/1138/SK6812+LED+datasheet+.pdf 
