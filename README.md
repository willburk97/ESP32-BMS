# ESP32-BMS
Single ESP32 monitoring and balancing multiple cell Li-Ion battery.

For each cell, to enable discharging, the ESP32 drives an opto-coupler (P521) with the digital output and the opto-coupler is driving the discharge mosfet(IRFZ44N) /resistor(16ohm,3W) across each cell. This'll be duplicatable across any number of cells (given the ESP32 has enough analog inputs and digital outputs. I'm using it for a 4 cell BMS/monitor which is using Adafruit io for trending/remote monitoring. The RJ11 port is to connect it to my Kisae SW1220 Inverter. This isn't coded in at this time and the newly revised inverter control circuit hasn't been tested. The circuit in my original BMS has been tested (The single Arduino with a ESP8266-01 BMS).  

Note:  I added an I2C port to easily allow for adding components (like maybe an LCD/OLED display).

Ordered PCB and succesfully tested it (before I added the mounting holes and the wider traces.)   Worked great (didn't test i2c or rj11 port.)
