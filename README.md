# Workshop Power Manager
Copyright (C) 2019 Paul R Bailey  aka  'Jack Arson'
Written on a Raspberry Pi (Linux) using Arduino-1.8.9
also runs fine on Windows with Arduino-1.8.8

Workshop Power Manager controls my 12 volt solar power system in my workshop.  It is a unique machine I've built for my own use.  I hope that by making my code public, it will find a like minded tinkerer.  

I am also curious if anyone will ever find this project.  If you do find it, (and it helps you in some way,) I would love to hear about it. axe8765@gmail.com


 * It monitors system voltage and can activate a battery charger or an inverter.
 * It controls two 120 volt circuits and can switch them away from the grid
  to the inverter as needed.
* Reads two LDR's (light-dependent resistors,) for a visual confirmation of circuit swapped status
 * The micro-controller is an Arduino Mega. This sketch will also fit an Uno. 
 * It has a battery backed up RTC clock, and a 4 x 20 LCD screen.  
 * the clock and LCD communicate to the Mega through the i2c protocol. 
 * Homemade 9 volt power supply (drops to 5 volts after the Mega's regulator.)
 * Homemade voltage divider. (Allows my 5 volt Mega to measure up to 20 VDC.)
 * Has a rotating message display system I use to remind myself of upcoming events. 
 * Controls and protects a track lighting system I converted to 12VDC.
 * It also reads a potentiometer I am using as a dimmer switch.  
