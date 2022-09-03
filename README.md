# Universal-Arduino-geiger-counter

## General Description:

The idea behind this project is to have a universal-ish and relatively simple geiger counter that can be build with cheap and easy to find component. If you are new to this subject since we are dealing with high voltage i strongly suggest you to buy a pre-made geiger tube module (those that can be found on Aliexpress), this can save you time and money in case you fry something with the high voltage. Again, in this project we are dealing with relatively high voltage (~400v) that (even if at low current) can hurt you, some high voltage modules on the internet include high capacity capacitors on the 400v output, those can deliver high voltage high current pulse capable of seriously hurt you and destroy your equipment. 



## BOM:

- Arduino Nano (Uno works too)

- 128x32 OLED display
- Push button (x2) 
- Geiger-Muller Tube
- HV generator (~400v)

### Note: if you are using a pre-made Geiger module since the high voltage part and the output interfaces are alredy included, you will only need the arduino, the screen, the two push button and some wire to connect everything together.


 Optional:
- On/Off switch
- Probe connector
- Lithium battery (to make it portable)
- Lithium protection/charge board
- Buzzer
- 3d Printed Case
- Magnets (to attach different probes to the main unit)


Misc:
- Wire (for the probe better a shielded ones).
- Various component based on the used schematics for the tube driver (resistor, diode, transistor ecc.).
- A breadboard or stripboard / general pcb material (since we are using high voltage and noise sensitive stuff the pcb is the recommended way).

## Arduino pinout Description:

- Arduino pin 2 (D2 on Nano): Interrupt pin, used to sense the pulse.
- Arduino pin 4 (D4 on Nano): Buzzer pin, connected to buzzer positive pin is used to make it tic in case of an event.
- Arduino pin 5 (D5 on Nano): Screen control pin, connected to a button is used to turn On and Off the display.
- Arduino pin 6 (D6 on Nano): Buzzer control pin, connected to a button is used to turn On and Off the buzzer.
- Arduino pin A0 (A0 on Nano): Battery sense pin, connected to battery positive is used to sense the battery voltage and later display the battery charge.
- Arduino pin A4 (A4 on Nano): Screen SDA;
- Arduino pin A5 (A5 on Nano): Screen SCL;


## Build Instruction, Wiring and Schematics:

The optional part is mandatory if you want a complete portable unit, however you can build a functional geiger counter just by using the basic component. In case you are using the module depend on what module are using you only need to connect the output pulse pin to Arduino pin 2, 5v/VCC, and ground. If you have decided to go the hard way you have to build the tube interface and make the connection by yourself, for the high voltage power supply i still suggest you using some pre-made 400v module. By searching on google i found this: 

![Geiger-Counter-kit](https://user-images.githubusercontent.com/17268735/188287614-a6f1a719-df5a-466f-b613-65c30b21cc44.png)

Source: https://www.electroschematics.com/diy-geiger-counter/


## How to use it:
 
In the code you can see this line: #define instruction false. If you set it to true at every boot the instruction will be printed on the screen, however this option will slow down some operation (boot and set menu) because the entire instructions on how to use the menu will be printed. If you dont care about time you can enable this option, this will help you to remember how to use it every time. Since the step to set a tube is pretty simple, by default i set this option to false. 

1) When off press the screen and buzzer buttons together.
2) Turn on the device using the power switch or by plugging the cable on the Arduino.
- If you did this steps right on the screen will appear the SET MENU text.
3) Once the text appeared release the two buttons. On the screen will appear the name of the first tube on the list.
4) Press screen / buzzer button to navigate the menu. Once you selected your tube just wait, after 6 second the selected tube will be automatically set and the conversion factor displayed.

##EXTRA NOTE:
The goal is to keep this project as cheap and simple as possible, i didn't really over engineered the device, that's why i didn't use extra complex circuitry or expensive coax connector / cable for the probe. I used a simple insulated audio jack, DONT USE THE METAL ONES since in case of a failure the metal case can conduct the 400v into you, even if the power supply is not earth referenced  we dont want i any case 400v on something that the user can easily touch. Same with the high voltage probe cable, i didnt use high voltage coax cable but a generic shielded cable. In general for the circuit i suggest you to just use common sense for the high voltage part, if you dont have enough experience whith that just use pre-build module. In the software in order to improve speed i avoided unnecessary operation during the interrupt call or during timer overflow call, again i didnt really over engineered it. 
 

