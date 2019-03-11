//The wiring inside the wooden node box next to the workbench Mega
//09-17-16  I rebuilt the data line to pin six for the PWM lighting experiments.  A purple dupont connection on the Mega to 
//to striped blue to yellow lead of 5 wire tstat cable
//the yellow wire then goes to the wooden fuse box above the doorway (fuses e - h .) and connects to the whiteorange wire of the 12wire gray data cable.
//the whiteorange wire then goes to the power source switch box where it connects to the gate wire of the MOSFET circuit.


// 11.24.2017
// Begining the rewrite of "Workbench_Mega"

// 11.27.2017
// Repaired Main Loop to allow for rollover safe function for my500msLoopfunction()
// Added voltage averaging in fast main loop part of program.  Calibrated voltage.

// 11.28.2017
// Adding voltage compensation code to the track light dimmer.  I want to prevent those lights
// from exceeding 12vdc.  I think the high charging rail voltage is damaging the bulbs.

// 11.30.2017
// I am changing the way the RTC clock is read and used to end dependence on constant updates.

// 12.1.2017
// I created (tmElements_t  current_microcontroller_time) and I am updating it every second.
// This will replace all the calls to the external clock for the functions that still rely on
// the current long time format.

// 12.2.2017
// I timed the difference in the clock on the Arduino Mega against the RTC module.  The Arduino
// was almost 36 seconds slower after only 6 hours!  I had the idea to update the clock at midnight,
// but I guess I'll have to update every second.

// 12.3.2017
// I made repairs to the message manager function.  It needed independent timing.  It still needs
// attention

// 12.4.2017
// I finished wiring up the AUX power wiring.  Begining a new function myAuxPowerfunction()and
// myTestForMorningBatteryChargefunction()

// 12.5.2017
// Improvements to sunrise / sunset variables

// 1.5.2018
// New batteries!  4 x 100AH

//BATTERY BENCHMARKS
//START 13.3V  9:41 315WATTS OUT 84 IN 9:43 DROPPED TO 12.4 PRETTY QUICKLY, faster than i expected

//1.6.2018
//9:54:00 TO 9:55:30 READ 12.4 AT MONITOR, THEY JUMPED BACK TO 12.7, THEN .8 PRetty quickly mebbe 400 watts 

//2.16.2018
//I'm working on a two stage circuit swapping system for breakers 4 and 8 here in the workshop.
//

//2.26.2018
//I want to restructure the modes to make linear sense
//mode 0 sleep =  aux power on
//mode 1 wake  =  aux power off for x minutes to let batteries settle
// idea:  a function that takes these parameters (min/max voltage allowable time min/max) 
//        from one of the mode routines, and returns a number when the condition has been met.


//5.1.2018 first object written!  MyTimer

//5.8.2018 Trying to implement a "state machine"
//5.11.2018  Success!  a machine with 16 states of being.  I'll be able to cut out a whole lot of
//           spaghetti code now.
