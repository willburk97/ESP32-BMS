# ESP32-BMS
Single ESP32 monitoring and balancing multiple cell Li-Ion battery.

For each cell, to enable discharging, the ESP32 drives an opto-coupler (P521) with the digital output and the opto-coupler is driving the discharge mosfet(IRFZ44N) /resistor(16ohm,3W) across each cell. This'll be duplicatable across any number of cells (given the ESP32 has enough analog inputs and digital outputs. I'm using it for a 4 cell BMS/monitor which is using Adafruit io for trending/remote monitoring. The RJ11 port is to connect it to my Kisae SW1220 Inverter. This is NOW coded in. I wasn't able to test the newer design with the Kisae Inverter but the circuit in my original BMS worked fine (The single Arduino with a ESP8266-01 BMS).  

Note:  I added an I2C port to easily allow for adding components (like maybe an LCD/OLED display).

Ordered PCB and succesfully tested it (before I added the mounting holes and the wider traces.)   Worked great (didn't test i2c or rj11 port.)

My Kisae inverter died in a lightning strike, but if you have one I think the schematic/board need to be changed to have Battery+ going to the optocoupler and the other leg going to the RJ11 port. (The wires on the opto-coupler will have to be reversed of course.)

My replacement inverter, a Go-Power GP-SW2000-12, has a remote input (Gnd, Enab, !Enab). I was able to use the schematic/board as is and simply connect the !Enab input on the inverter directly to the opto-coupler connection at the RJ11 port. (Which gets grounded when the digital output from the ESP32 turns on.)

Note: The Kisae inverter used a momentary Battery+ signal to turn the inverter on or off (change it from one state to the other).
    The Go Power inverter requires a constant input signal (either battery+ or Gnd depending on the input you use) in order to be powered on. (Enab requires Batt+, !Enab requires Gnd, pick one to use.)
