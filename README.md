# ESP32-BMS
Work in progress.  Single ESP32 monitoring and balancing multiple cell Li-Ion battery.

Currently driving an opto-coupler with the digital output and the opto-coupler is driving the discharge mosfet/resistor across cell 4. This'll be duplicatable
across any number of cells (given the ESP32 has enough analog inputs and digital outputs. I'm using it for a 4 cell BMS/monitor which is using Adafruit io for
trending/remote monitoring.
