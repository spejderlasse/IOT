/*
 * Project GateControl
 * Description: Controling the gate for a chicken coop.
 * Author: Lasse Greve Rasmussen
 * Date:  26-11-2020
 */

int photosensor = A0;
int gate_PWM = D3;
int push_open = D6;
int push_close = D5;
int push_auto = D4;
int activate_servo = D2;
int man_sig = D7;
const int PWM_OPEN = 5;
const int PWM_CLOSE = 35;
const int PWM_MID = 16;
int ligthlevel;
int lightlevel_cur;
int pwm_pos;
const int PWM_FREKQUENZY = 50;
const int LIGTH_LOW = 50;
const int LIGTH_HIGH = 110;
int sunset;
int sunrise;
int curr_time;
String mystring;
int DELAYTIME = 600000;

int gatecontrole(String command);
void OpenGate();
void CloseGate();
void GoToAuto();
void myHandler(const char *event, const char *data);

typedef enum{
manual,
automatic,
} mode_types;

mode_types mode;

// setup() runs once, when the device is first turned on.
void setup() {

  // set gate to mid position and open
  pinMode(gate_PWM, OUTPUT);
  pinMode(activate_servo, OUTPUT);
  pwm_pos = PWM_MID;
  OpenGate();

  //variables to access online
  Particle.variable("ligthlevel", &ligthlevel, INT);
  Particle.variable("gate position", &pwm_pos, INT);
  Particle.variable("current light", &lightlevel_cur, INT);
  Particle.variable("sunset", &sunset, INT);
  Particle.variable("sunrise", &sunset, INT);
  Particle.variable("curr_time", &sunset, INT);

  //online option to open and close gate
  Particle.function("gate",gatecontrole);

  Particle.subscribe("hook-response/suntimes", myHandler, MY_DEVICES);

  // setting ligthlevel between high and low
  ligthlevel = (LIGTH_HIGH + LIGTH_LOW) * 0.5;

  //take controle of rgb and turn off
  RGB.control(true);
  RGB.color(0, 0, 0);

  //attaching interrupts for manually opening or closing the gate
  pinMode(push_close, INPUT_PULLDOWN);
  pinMode(push_open, INPUT_PULLDOWN);
  pinMode(push_auto, INPUT_PULLDOWN);
  pinMode(man_sig, OUTPUT);
  attachInterrupt(push_open, OpenGateI, RISING, 8);
  attachInterrupt(push_close, CloseGateI, RISING, 8);
  attachInterrupt(push_auto, GoToAuto, RISING, 8);
  mode = automatic;

}

void loop()
{
  while(WiFi.ready()==0)
  {
    //go to sensorcontrol
  }

  if(mode = manual)
  {
    lightlevel_cur = analogRead(photosensor);
    ligthlevel = lightlevel_cur * 0.5 + ligthlevel * 0.5;
    if (ligthlevel < LIGTH_LOW && pwm_pos != PWM_CLOSE )
    {
      CloseGate();
    }
    if (ligthlevel > LIGTH_HIGH && pwm_pos != PWM_OPEN)
    {
      OpenGate();
    }  
  }
  String ligthlevel_str = String(ligthlevel);
  Particle.publish("light", ligthlevel_str, PRIVATE);
  RGB.color(0, 8, 0);
  delay(5000);//use DELAYTIME
  RGB.color(8, 0, 0);
  if(mode == automatic)
  {
    SystemSleepConfiguration config;
    config.mode(SystemSleepMode::ULTRA_LOW_POWER)
      .gpio(push_auto, RISING)
      .duration(20s);
    System.sleep(config);
    
  }

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
  while(pwm_pos>PWM_OPEN)
  {
    pwm_pos--;
    analogWrite(gate_PWM, pwm_pos, PWM_FREKQUENZY);
    delay(200);
  }
  
  String pwm_pos_str = String(pwm_pos);
  Particle.publish("position", pwm_pos_str, PRIVATE);
  delay(500);
  digitalWrite(activate_servo, LOW);
  //RGB.color(0, 8, 0);

  return;
}

void CloseGate()
{
  digitalWrite(activate_servo, HIGH);
  while(pwm_pos<PWM_CLOSE)
  {
    pwm_pos++;
    analogWrite(gate_PWM, pwm_pos, PWM_FREKQUENZY);
    delay(200);
  }

  String pwm_pos_str = String(pwm_pos);
  Particle.publish("position", pwm_pos_str, PRIVATE);
  delay(500);
  digitalWrite(activate_servo, LOW);
  //RGB.color(8, 0, 0);

  return;
}

void OpenGateI()
{
  digitalWrite(activate_servo, HIGH);
  pwm_pos = PWM_OPEN;
  digitalWrite(activate_servo, HIGH);
  analogWrite(gate_PWM, pwm_pos, PWM_FREKQUENZY);
  mode = manual;
  digitalWrite(man_sig, HIGH);
  RGB.color(0, 4, 0);
  return;
}

void CloseGateI()
{
  pwm_pos = PWM_CLOSE;
  digitalWrite(activate_servo, HIGH);
  analogWrite(gate_PWM, pwm_pos, PWM_FREKQUENZY);
  mode = manual;
  digitalWrite(man_sig, HIGH);
  RGB.color(4, 0, 0);
  return;
}

void GoToAuto()
{
  mode = automatic;
  digitalWrite(man_sig, LOW);
  digitalWrite(activate_servo, LOW);
  RGB.color(0, 0, 0);
}

void myHandler(const char *event, const char *data) {
  String mystring = String(data);

}