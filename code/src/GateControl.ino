/*
 * Project GateControl
 * Description: Controling the gate for a chicken coop.
 * Author: Lasse Greve Rasmussen
 * Date:  06-12-2020
 */

//input and output variables
int photosensor = A0;
int activate_servo = D2;
int gate_PWM = D3;
int push_auto = D4;
int push_close = D5;
int push_open = D6;
int sw_eco = D8;

//constants for gate controle
const int GATE_OPEN = 5;
const int GATE_CLOSED = 35;
const int GATE_MID = 17;
const int PWM_FREKQUENZY = 50;

//variables to publish
int gate_pos;
int lightlevel = 0;

//constants defining lightlevels
const int LIGTH_LOW = 1;
const int LIGTH_HIGH = 5;
const int MEAN_N = 5*300;//

//constants for delay
int MS2SECOND = 1000;
int MS2MINUT = MS2SECOND * 60;
int MS2HOUR = MS2MINUT * 60;
int MAXWAIT = 5* MS2HOUR * 20; // set to 60000 for test
int DELAYTIME = MS2SECOND*10;

//prototypes of functions
int gatecontrole(String command);
void OpenGate();
void CloseGate();
void GoToNormal();
void SensorControl();

//counter to detect of there is to long time between online function calls
int timer;

//counter to keep power on after manual interrupts
int man_counter;

//enum for modes
typedef enum{
manual,
normal,
sensor_mode,
} mode_types;
mode_types mode;

/*
seting up the device at startup
*/
void setup() {

  // set gate to mid position and open
  pinMode(gate_PWM, OUTPUT);
  pinMode(activate_servo, OUTPUT);
  gate_pos = GATE_MID;
  CloseGate();

  //variables to access online
  Particle.variable("ligthlevel", &lightlevel, INT);
  Particle.variable("gate position", &gate_pos, INT);

  //online function call to open and close gate
  Particle.function("gate",gatecontrole);

  //take controle of rgb and turn it  off
  RGB.control(true);
  RGB.color(0, 0, 0);

  //attaching interrupts for manually opening or closing the gate
  pinMode(push_close, INPUT_PULLDOWN);
  pinMode(push_open, INPUT_PULLDOWN);
  pinMode(push_auto, INPUT_PULLDOWN);
  attachInterrupt(push_open, OpenGateI, RISING, 9);
  attachInterrupt(push_close, CloseGateI, RISING, 8);
  attachInterrupt(push_auto, GoToNormal, RISING, 7);
  
  //starting in normal mode
  mode = normal;
  timer = 0;
}

/*
the loop is running a switch based on which mode is active
Controling RGB, running delays and timers, calling relevant functions
*/
void loop()
{
  switch (mode)
  {
  case normal:
    RGB.color(0, 10, 0);
    delay(DELAYTIME);
    timer = timer + DELAYTIME;
    if (timer >= MAXWAIT)
    {
      mode = sensor_mode;
      RGB.color(10, 0, 0);
    }
    break;
  case sensor_mode:
    RGB.color(10, 0, 0);
    SensorControl();
    break;
  case manual:
    RGB.color(0, 0, 10);
    delay(1000);
    if (man_counter < 3)
    {
      man_counter++;
    }
    else
    {
      digitalWrite(activate_servo, LOW);
    }
    break;
  default:
    delay(200);
    break;
  }
}

/*
This function will 
Find a mean of MEAN_N samples
Publish the value
Open or close the gate, if it is required
*/
void SensorControl()
{
  lightlevel = 0;
  for (int i =0; i<MEAN_N; i++)
  {
    RGB.color(0,0,0);
    delay(100);
    lightlevel = lightlevel + analogRead(photosensor);
    delay(100);
    RGB.color(10,0,0);
    if(mode!=sensor_mode)
    {
      return;
    }
  }
  lightlevel = lightlevel /MEAN_N;

  String ligthlevel_str = String(lightlevel);
  Particle.publish("light", ligthlevel_str, PRIVATE);
  delay(4500);

  if (lightlevel <= LIGTH_LOW && gate_pos != GATE_CLOSED )
  {
    CloseGate();
  }
  else if (lightlevel >= LIGTH_HIGH && gate_pos != GATE_OPEN)
  {
    OpenGate();
  }  
}

/*
This function is for online function call
The call should bring an argument that defines to call OpenGate og Close Gate
*/
int gatecontrole(String command) {
  timer=0;
  if (mode==manual)
  {
    return 0;
  }
  mode = normal;
  if (command == "open") 
  {
    OpenGate();
  }
  else if (command == "close") 
  {
    CloseGate();
  }
  return 1;
}

/*
Function for opening the gate
turning on servo supply
opening in steps of 200 ms till it is open
publishing the value of an open gate
shuting off servo supply after a delay
*/
void OpenGate()
{
  digitalWrite(activate_servo, HIGH);
  while(gate_pos>GATE_OPEN)
  {
    gate_pos--;
    analogWrite(gate_PWM, gate_pos, PWM_FREKQUENZY);
    delay(200);
  }
  String pwm_pos_str = String(gate_pos);
  Particle.publish("position", pwm_pos_str, PRIVATE);
  delay(2000);
  digitalWrite(activate_servo, LOW);
  return;
}

/*
Function for closeing the gate
turning on servo supply
closeing in steps of 200 ms till it is closed
publishing the value of a closed gate
shuting off servo supply after a delay
*/
void CloseGate()
{
  digitalWrite(activate_servo, HIGH);
  while(gate_pos<GATE_CLOSED)
  {
    gate_pos++;
    analogWrite(gate_PWM, gate_pos, PWM_FREKQUENZY);
    delay(200);
  }
  String pwm_pos_str = String(gate_pos);
  Particle.publish("position", pwm_pos_str, PRIVATE);
  delay(2000);
  digitalWrite(activate_servo, LOW);
  return;
}

/*
Function for opening the gate at interrupts
this is clean af delays to keep the interrupt as short as possible
when opening manually the risk of hurting a chicken in the way, is low because
a person is standing nex to the gate
turning off the servo supply is handled in the main loop
*/
void OpenGateI()
{
  RGB.color(0, 0, 10);
  digitalWrite(activate_servo, HIGH);
  gate_pos = GATE_OPEN;
  analogWrite(gate_PWM, gate_pos, PWM_FREKQUENZY);
  mode = manual;
  man_counter = 0;
  return;
}

/*
Function for closeing the gate at interrupts
this is clean af delays to keep the interrupt as short as possible
when closeing manually the risk of hurting a chicken in the way, is low because
a person is standing nex to the gate
turning off the servo supply is handled in the main loop
*/
void CloseGateI()
{
  RGB.color(0, 0, 10);
  digitalWrite(activate_servo, HIGH);
  gate_pos = GATE_CLOSED;
  analogWrite(gate_PWM, gate_pos, PWM_FREKQUENZY);
  mode = manual;
  man_counter = 0;
  return;
}

/*
function to handle interrupts for leaving manual mode
*/
void GoToNormal()
{
  mode = normal;
  digitalWrite(activate_servo, LOW);
  RGB.color(0, 0, 0);
}