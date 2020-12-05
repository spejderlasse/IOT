/*
 * Project GateControl
 * Description: Controling the gate for a chicken coop.
 * Author: Lasse Greve Rasmussen
 * Date:  26-11-2020
 */

int photosensor = A0;
int activate_servo = D2;
int gate_PWM = D3;
int push_auto = D4;
int push_close = D5;
int push_open = D6;
int signal = D7;
int sw_eco = D8;

const int GATE_OPEN = 5;
const int GATE_CLOSED = 35;
const int GATE_MID = 17;
const int PWM_FREKQUENZY = 50;
int gate_pos;

int lightlevel;
const int LIGTH_LOW = 50;
const int LIGTH_HIGH = 110;

String mystring;
//constants for sleep
int MINUT = 60;
int HOUR = 60 * MINUT;
int MAX_SLEEP = HOUR * 20; // longest day/night is 17.5 hour

//constants for delay
int MS2SECOND = 1000;
int MS2MINUT = MS2SECOND * 60;
int MS2HOUR = MS2MINUT * 60;
int MAXWAIT = 5* MS2HOUR * 20; // longest day/night is 17.5 hour
int DELAYTIME = MS2SECOND*10;

int gatecontrole(String command);
void OpenGate();
void CloseGate();
void GoToAuto();
void myHandler(const char *event, const char *data);
void sensor_control();

int timer;
int man_counter;

typedef enum{
manual,
normal,
sensor_mode,
exam,
} mode_types;
mode_types mode;

// setup() runs once, when the device is first turned on.
void setup() {

  // set gate to mid position and open
  pinMode(gate_PWM, OUTPUT);
  pinMode(activate_servo, OUTPUT);
  gate_pos = GATE_MID;
  CloseGate();

  //variables to access online
  Particle.variable("ligthlevel", &lightlevel, INT);
  Particle.variable("gate position", &gate_pos, INT);

  //online option to open and close gate
  Particle.function("gate",gatecontrole);

  // setting ligthlevel between high and low
  //ligthlevel = (LIGTH_HIGH + LIGTH_LOW) * 0.5;

  //take controle of rgb and turn off
  RGB.control(true);
  RGB.color(0, 0, 0);
  pinMode(signal, OUTPUT);
  digitalWrite(signal, LOW);

  //attaching interrupts for manually opening or closing the gate
  pinMode(push_close, INPUT_PULLDOWN);
  pinMode(push_open, INPUT_PULLDOWN);
  pinMode(push_auto, INPUT_PULLDOWN);
  attachInterrupt(push_open, OpenGateI, RISING, 9);
  attachInterrupt(push_close, CloseGateI, RISING, 8);
  attachInterrupt(push_auto, GoToAuto, RISING, 7);
  
  mode = normal;
  timer = 0;
}

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
    sensor_control();
    break;
  case manual:
    RGB.color(0, 0, 10);
    delay(1000);
    if (man_counter < 2)
    {
      man_counter++;
    }
    else
    {
      digitalWrite(activate_servo, LOW);
    }
    break;
    
  default:
    delay(1000);
    break;
  }

}

void sensor_control()
{
  /*find mean of lightlevel 10 meassurements in 5.5 seconds
  publishing and evaluating after 10 seconds*/
  RGB.color(0,0,0);
  lightlevel = 0;
  for (int i =0; i<10; i++)
  {
    lightlevel = lightlevel + analogRead(photosensor);
    delay(550);
  }
  RGB.color(0,0,10);
  lightlevel = lightlevel * 0.1;
  String ligthlevel_str = String(lightlevel);
  Particle.publish("light", ligthlevel_str, PRIVATE);
  delay(4500);

  //Is action requred then act
  if (lightlevel < LIGTH_LOW && gate_pos != GATE_CLOSED )
  {
    CloseGate();
  }
  else if (lightlevel > LIGTH_HIGH && gate_pos != GATE_OPEN)
  {
    OpenGate();
  }  
}

int gatecontrole(String command) {
  timer=0;

  switch (mode)
  {
  case sensor_mode:
    mode = normal;
    break;
  case manual:
    return 0;
  default:
    break;
  }

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

void GoToAuto()
{
  mode = normal;
  digitalWrite(activate_servo, LOW);
  RGB.color(0, 0, 0);
}

