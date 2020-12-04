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

int ligthlevel;
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

int gatecontrole(String command);
void OpenGate();
void CloseGate();
void GoToAuto();
void myHandler(const char *event, const char *data);
void sensor_control(int sleep);

typedef enum{
manual,
wifi,
eco,
exam,
} mode_types;
mode_types mode;

// setup() runs once, when the device is first turned on.
void setup() {


  // set gate to mid position and open
  pinMode(gate_PWM, OUTPUT);
  pinMode(activate_servo, OUTPUT);
  gate_pos = GATE_MID;
  OpenGate();

  //variables to access online
  Particle.variable("ligthlevel", &ligthlevel, INT);
  Particle.variable("gate position", &gate_pos, INT);

  //online option to open and close gate
  Particle.function("gate",gatecontrole);

  // setting ligthlevel between high and low
  ligthlevel = (LIGTH_HIGH + LIGTH_LOW) * 0.5;

  //take controle of rgb and turn off
  RGB.control(true);
  RGB.color(0, 0, 0);

  //attaching interrupts for manually opening or closing the gate
  pinMode(sw_eco, INPUT_PULLDOWN);
  pinMode(push_close, INPUT_PULLDOWN);
  pinMode(push_open, INPUT_PULLDOWN);
  pinMode(push_auto, INPUT_PULLDOWN);
  pinMode(signal, OUTPUT);
  digitalWrite(signal, LOW);
  attachInterrupt(push_open, OpenGateI, RISING, 9);
  attachInterrupt(push_close, CloseGateI, RISING, 8);
  attachInterrupt(push_auto, GoToAuto, RISING, 7);
  
  mode = wifi;

}

void loop()
{

  SystemSleepConfiguration config;
  config.mode(SystemSleepMode::STOP)
    .network(NETWORK_INTERFACE_WIFI_STA, SystemSleepNetworkFlag::NONE)
    .gpio(push_open, RISING);
    //.gpio(push_close, RISING)
    //.gpio(push_auto, RISING);
    //.duration(40s);
  System.sleep(config);

  while(mode==wifi)
  {
    digitalWrite(signal,HIGH);
    delay(500);
    digitalWrite(signal ,LOW);
    delay(500);
  }
  delay(2000);
  RGB.color(0,100,100);
  delay(2000);
  RGB.color(0,0,0); 

  while(mode==manual)
  {
    digitalWrite(signal,HIGH);
    delay(100);
    digitalWrite(signal ,LOW);
    delay(100);
  }







  /*
  if(mode != eco && sw_eco)
  {
    mode = eco;
  }
  else if(mode == eco && !sw_eco)
  {
    mode = wifi;
  }*/
  /*
  switch (mode)
  {
  case manual:
    delay(10000);
    digitalWrite(activate_servo, LOW);
    break;
  case wifi:
    digitalWrite(signal,HIGH);
    delay(1000);
    digitalWrite(signal ,LOW);
    delay(1000);

    if(!WiFi.ready())
    {
      sensor_control(1);
    }
    break;
  case eco:
    sensor_control(1);
    break;
  case exam:
    RGB.color(0, 8, 8);
    delay(1000);
    RGB.color(0, 0, 0);
    sensor_control(1);
    break;
  default:
    break;
  }  */
}

void sensor_control(int sleeptime)
{
  int lightlevel = 0;
  for (int i =0; i<10; i++)
  {
    lightlevel = lightlevel + analogRead(photosensor);
  }

  String ligthlevel_str = String(ligthlevel);
  Particle.publish("light", ligthlevel_str, PRIVATE);

  if (ligthlevel < LIGTH_LOW && gate_pos != GATE_CLOSED )
  {
    CloseGate();
  }
  else if (ligthlevel > LIGTH_HIGH && gate_pos != GATE_OPEN)
  {
    OpenGate();
  }  

  //SystemSleepConfiguration& network(network_interface_t netif, EnumFlags<SystemSleepNetworkFlag> flags = SystemSleepNetworkFlag::NONE)

  SystemSleepConfiguration config;
  config.mode(SystemSleepMode::STOP)
    .network(NETWORK_INTERFACE_WIFI_STA, SystemSleepNetworkFlag::NONE)
    .gpio(push_open, RISING)
    .gpio(push_close, RISING)
    .duration(sleeptime);
  System.sleep(config);
}

int gatecontrole(String command) {
  if (command == "open") 
  {
    OpenGate();
    return 1;
  }
  else if (command == "close") 
  {
    CloseGate();
    return -1;
  }
  else 
  {
    return 0;
  }
}

void OpenGate()
{
  RGB.color(0, 8, 0);
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
  RGB.color(0, 0, 0);

  return;
}

void CloseGate()
{
  RGB.color(8, 0, 0);
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
  RGB.color(0, 0, 0);

  return;
}

void OpenGateI()
{
  RGB.color(0, 8, 0);
  digitalWrite(activate_servo, HIGH);
  gate_pos = GATE_OPEN;
  analogWrite(gate_PWM, gate_pos, PWM_FREKQUENZY);
  mode = manual;
  return;
}

void CloseGateI()
{
  RGB.color(8, 0, 0);
  gate_pos = GATE_CLOSED;
  digitalWrite(activate_servo, HIGH);
  analogWrite(gate_PWM, gate_pos, PWM_FREKQUENZY);
  mode = manual;
  return;
}


void GoToAuto()
{
  mode = wifi;
  digitalWrite(activate_servo, LOW);
  RGB.color(0, 0, 0);
}

void myHandler(const char *event, const char *data) {
  String mystring = String(data);

}