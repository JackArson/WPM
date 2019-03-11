
//state_machine states
//0 initiate sleep state
//1 sleep state (the battery charger is always on)
//2 initiate wake state
//3 wake state
//4 initiate balanced state
//5 balanced state
//6 initiate inverter warm up state
//7 inverter warm up state
//8 initiate stage one inverter state
//9 stage one inverter state
//10 initiate stage two inverter state
//11 stage two inverter state
//12 initiate daytime charging
//13 daytime charging
//14 initiate inverter cool down
//15 inverter cool down
//===========================================
void myStateMachineInitSleepStatefunction() {               //state 0
  //================================5.8.2018===
  myPrintStatefunction("Sleeping");
  //initialize relevant pins with redundancy
  digitalWrite (battery_charger_signal_pin, HIGH);  //battery charger on
  digitalWrite (inverter_signal_pin, LOW);         //inverter off
  digitalWrite(stage_one_inverter_relay, LOW);    // relay one off
  digitalWrite(stage_two_inverter_relay, LOW);    // relay two off
  machine_state = 1; //go to sleep state
  return;
}

//=======================================
void myStateMachineSleepStatefunction() {                   //state 1
  //============================5.8.2018===
  if (myIsItDaylightfunction() == true) {        //switch to initiate wake state if light
    machine_state = 2; //initiate wake state
    return;
  }

  return;
}

//==========================================
void myStateMachineInitWakeStatefunction() {                //state 2
  //==============================5.8.2018====
  myPrintStatefunction("Waking  ");
  //initialize relevant pins with redundancy
  digitalWrite (battery_charger_signal_pin, LOW);   // battery charger off
  digitalWrite (inverter_signal_pin, LOW);          // inverter off
  digitalWrite(stage_one_inverter_relay, LOW);      // relay one off
  digitalWrite(stage_two_inverter_relay, LOW);      // relay two off
  machine_state = 3;
}

//======================================
void myStateMachineWakeStatefunction() {                    //state 3
  //===========================5.8.2018===
  //check to see if it is time to go back to sleep
  //check to see how long it is past dawn.
  //if it is delta_t, or 15 minutes, past dawn switch to state 4 (init balanced)
  //The wake up delay ensures that the the inverter does not burn off the fresh battery charger energy
  //Time of day used here so the machine can begin inverting quickly if reset mid-day
  //I'm planning on starting in balanced mode however, so should never come up
  if (myIsItDaylightfunction() == false) {        //return to initiate sleep mode if dark, unlikely
    machine_state = 0; //initiate sleep state
    return;
  }
  if (myIsItDeltaTimePastDawnfunction() == true) {
    machine_state = 4;  //initiate balanced
    return;
  }
}

//==============================================
void myStateMachineInitBalancedStatefunction() {            //state 4
  //===================================5.8.2018===


  myPrintStatefunction("Balanced");
  //initialize relevant pins with redundancy
  digitalWrite (battery_charger_signal_pin, LOW);  //battery charger off
  digitalWrite (inverter_signal_pin, LOW);         //inverter off
  digitalWrite(stage_one_inverter_relay, LOW);    // relay one off
  digitalWrite(stage_two_inverter_relay, LOW);    // relay two off
  //
  machine_state = 5;                               // balanced initialization complete
  return;
}

//==========================================
void myStateMachineBalancedStatefunction() {                //state 5
  //===============================5.8.2018===

  const float voltage_to_start_inverter = 13.80;
  const float voltage_to_start_daytime_charging = 12.60;

  if (myIsItDaylightfunction() == false) {        //switch to initiate sleep mode if dark
    machine_state = 0; //initiate sleep state
    return;
  }
  if (stable_voltage >= voltage_to_start_inverter) {
    machine_state = 6;     //init warm up inverter
    return;
  }
  if (stable_voltage <= voltage_to_start_daytime_charging) {
    machine_state = 12;    //init daytime charging
    return;
  }


  return;
}

//======================================================
void myStateMachineInitWarmUpInverterStatefunction() {    //state 6
  //===========================================5.8.2018===
  const byte seconds_to_warm_up = 4;

  myPrintStatefunction("Warm Up ");
  //initialize relevant pins with redundancy
  digitalWrite (battery_charger_signal_pin, LOW);  //battery charger off
  digitalWrite (inverter_signal_pin, HIGH);         //inverter on
  digitalWrite(stage_one_inverter_relay, LOW);    // relay one off
  digitalWrite(stage_two_inverter_relay, LOW);    // relay two off
  myTimer.Set(seconds_to_warm_up);
  machine_state = 7;
}



//==================================================
void myStateMachineWarmUpInverterStatefunction() {        //state 7
  //=======================================5.8.2018===

  if (myTimer.counter) {    //warming up during countdown
    return;
  }
  machine_state = 8;

}
//======================================================
void myStateMachineInitStageOneInverterStatefunction() {    //state 8
  //===========================================5.8.2018===

  const byte  stage_two_switching_delay = 15;  //seconds.  this prevents stage two from engaging too soon

  myPrintStatefunction("Invert1 ");
  //initialize relevant pins with redundancy
  digitalWrite (battery_charger_signal_pin, LOW);  //battery charger off
  digitalWrite (inverter_signal_pin, HIGH);         //inverter on
  digitalWrite(stage_one_inverter_relay, HIGH);    // relay one on
  digitalWrite(stage_two_inverter_relay, LOW);    // relay two off
  myTimer.Set(stage_two_switching_delay);
  machine_state = 9;
}

//==================================================
void myStateMachineStageOneInverterStatefunction() {        //state 9
  //=======================================5.8.2018===

  const float voltage_to_turn_inverter_off = 12.55;
  const float voltage_to_switch_to_stage_two = 13.80;

  inverter_run_time++;

  if (myIsItDaylightfunction() == false) {        //switch to initiate sleep mode if dark
    machine_state = 0; //initiate sleep state -   //might be helpful for a charger stuck in "on" state
    return;
  }

  if (stable_voltage >= voltage_to_switch_to_stage_two && !myTimer.counter) {
    machine_state = 10;  //initiate stage two inverter
    return;
  }

  if (stable_voltage <= voltage_to_turn_inverter_off) {
    machine_state = 14; //initiate inverter cooldown state, so a burst of solar energy does not short
    //cycle the inverter
    return;
  }

}
//======================================================
void myStateMachineInitStageTwoInverterStatefunction() {    //state 10
  //==============================5.8.2018================
  myPrintStatefunction("Invert2 ");
  //initialize relevant pins with redundancy
  digitalWrite (battery_charger_signal_pin, LOW);  //battery charger off
  digitalWrite (inverter_signal_pin, HIGH);         //inverter on
  digitalWrite(stage_one_inverter_relay, HIGH);    // relay one on
  digitalWrite(stage_two_inverter_relay, HIGH);    // relay two on
  machine_state = 11;
}

//==================================================
void myStateMachineStageTwoInverterStatefunction() {        //state 11
  //===========================5.8.2018===============

  const float voltage_to_drop_back_to_stage_one = 12.70;

  if (myIsItDaylightfunction() == false) {        //switch to initiate sleep mode if dark
    machine_state = 0; //initiate sleep state -   //might be helpful for a charger stuck in "on" state
    return;
  }

  inverter_run_time++;

  if (stable_voltage <= voltage_to_drop_back_to_stage_one) {
    machine_state = 8;
    return;
  }

}

//================================================
void myStateMachineInitDaytimeChargingfunction() {    //state 12
  //====================================5.11.2018===
  myPrintStatefunction("Charging");
  //initialize relevant pins with redundancy
  digitalWrite (battery_charger_signal_pin, HIGH);  //battery charger on
  digitalWrite (inverter_signal_pin, LOW);         //inverter off
  digitalWrite(stage_one_inverter_relay, LOW);    // relay one off
  digitalWrite(stage_two_inverter_relay, LOW);    // relay two off
  machine_state = 13;
}
//============================================
void myStateMachineDaytimeChargingfunction() {  //state 13
  //================================5.11.2018===

  const float voltage_to_switch_off_charger = 13.40;

  if (stable_voltage >= voltage_to_switch_off_charger) {
    machine_state = 4;  //switch to initbalanced
    return;
  }
}

//================================================
void myStateMachineInitInverterCooldownfunction() {    //state 14
  //====================================5.11.2018===

  byte inverter_cooldown_time = 30;  //seconds

  myPrintStatefunction("Cooldown");
  //initialize relevant pins with redundancy
  digitalWrite (battery_charger_signal_pin, LOW);  //battery charger off
  digitalWrite (inverter_signal_pin, LOW);         //inverter off
  digitalWrite(stage_one_inverter_relay, LOW);    // relay one off
  digitalWrite(stage_two_inverter_relay, LOW);    // relay two off
  myTimer.Set(inverter_cooldown_time);
  machine_state = 15;
}

//================================================
void myStateMachineInverterCooldownfunction() {    //state 15
  //====================================5.11.2018===

  if (!myTimer.counter) {
    machine_state = 4;  //init balanced
  }
}
