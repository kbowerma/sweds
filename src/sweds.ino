/*
 * Project sweds
 * Description:
 * Author: Kyle Bowerman
 * Date: 8/30/23
 * 3 led light strips on swed1 and swed2
 * working with PIR
 */

#include "sweds.h"
#include <neopixel.h>
#include <DeviceNameHelperRK.h>

// Globals
int led2 = D7;
int led1 = D0; // Instead of writing D0 over and over again, we'll write led1
int red, green, blue, white = 0; // variable to store color
int strip1pin = D2;
int strip2pin = D3;
int strip3pin = D4;
int PIR = D5;
int numpixel = 18;
int motionState;
int lastMotionTime, secSinceMotion = -1;
String lastMotionAt = "unset";
String modeName = "unset";
String deviceName = "";
int EEPROM_OFFSET = 100;
int myHour;

// Objects
SYSTEM_MODE(AUTOMATIC);
Adafruit_NeoPixel strip1(numpixel, strip1pin, SK6812RGBW);
Adafruit_NeoPixel strip2(81, strip2pin, SK6812RGBW);
Adafruit_NeoPixel strip3(85, strip3pin, SK6812RGBW);

// MyConfig myDefaults = { false, false, "default", 20};

MyLEDConfig myLED1 = {1, 0, 0, 0, 0};
MyLEDConfig myLED2 = {2, 0, 0, 0, 0};
MyLEDConfig myLED3 = {3, 0, 0, 0, 0};
MyLEDConfig LEDS[3] = {myLED1, myLED2, myLED3};
MyConfig readConfig;

void setup()
{

    // get Config
    EEPROM.get(CONFIGADDR, readConfig);
    // You must call this from setup!
    DeviceNameHelperEEPROM::instance().setup(EEPROM_OFFSET);
    deviceName = DeviceNameHelperEEPROM::instance().getName();

    if (readConfig.initialized == 1)
    {
        Particle.publish("Config_initialized", "yes");
    }
    else
    {
      Particle.publish("Config_initialized", String(readConfig.initialized));
      setConfig("init");
    }

    setUpParticleVariables();
    setUpParticleFunctions();
    setUpParticlePubSub();

    pinMode(led2, OUTPUT); // Built in led
    pinMode(led1, OUTPUT);
    pinMode(strip1pin, OUTPUT); // 18x neopixel
    pinMode(strip2pin, OUTPUT); // 81x neopixel
    pinMode(strip3pin, OUTPUT); // 81x neopixel
    pinMode(PIR, INPUT_PULLDOWN);

    strip1.begin();
    strip1.show(); // Initialize all pixels to 'off'
    strip2.begin();
    strip2.show(); // Initialize all pixels to 'off'
    strip3.begin();
    strip3.show(); // Initialize all pixels to 'off'
                   // Publish my vars
}

void loop()
{
 // Device name helper
  DeviceNameHelperEEPROM::instance().loop();

  // Motion Loop logic
  if (readConfig.motionEnabled == true )
  {
    motionState = digitalRead(PIR);

    // Got motion
    if (motionState == HIGH)
    { // got motion
        motionHandler();
    } 
    else { 
        if ( readConfig.mode == 1 & secSinceMotion > readConfig.awayHoldTMR ) {
            // run suspend
            suspend("Idle");
        }

    }
    // last item in loop
    secSinceMotion = (millis() - lastMotionTime) / 1000;

    // turn off led if motion is older than 5 seconds ago
    if ( secSinceMotion > 5 ) {
      digitalWrite(led2, LOW);
    }


}


}

// ---------- Functions ---------

void setUpParticleVariables()
{
    // get the modename
    getModeName();
    Particle.variable("red", red);
    Particle.variable("green", green);
    Particle.variable("blue", blue);
    Particle.variable("white", white);
    Particle.variable("version", MYVERSION);
    Particle.variable("fileName", FILENAME);
    Particle.variable("buildDate", BUILD_DATE);
    Particle.variable("myfirmware", MYFIRMWARE);
    Particle.variable("lastMotion", secSinceMotion);
    Particle.variable("lastMotionAt",lastMotionAt);
    Particle.variable("mode", modeName);
}

void setUpParticleFunctions()
{
    Particle.function("ledConfig", ledConfig);
    Particle.function("setConfig", setConfig);
    Particle.function("suspend",suspend);
    Particle.function("restore",restore);
}

void setUpParticlePubSub()
{
    Particle.publish("FILENAME", FILENAME);
    Particle.publish("MYVERSION", MYVERSION);
    Particle.publish("BUILD_DATE", BUILD_DATE);
    Particle.publish("MYFIRMWARE", MYFIRMWARE);
    // Subscribe
    Particle.subscribe("office", ledhandler, MY_DEVICES);
}

int ledConfig(String command)
{
    // usage:  strip:color:brightness   2:white:200
    int delim1 = command.indexOf(":");
    int delim2 = command.lastIndexOf(":");
    String mystrip = command.substring(0, delim1);
    String mycolor = command.substring(delim1 + 1, delim2);
    String mybrightness = command.substring(delim2 + 1);
    int stripId = mystrip.toInt();
    String myMsg = deviceName+"/ledConfig";
    // Particle.publish("mystrip", String(mystrip));
    // Particle.publish("mycolor", String(mycolor));
    // Particle.publish("mybrightness", String(mybrightness));
    Particle.publish(myMsg,command);

    if (mycolor == "red")
    {
        red = mybrightness.toInt();
        juiceLeds(stripId, red, green, blue, white);
        return 5;
    }
    if (mycolor == "green")
    {
        green = mybrightness.toInt();
        juiceLeds(stripId, red, green, blue, white);
        return 7;
    }
    if (mycolor == "blue")
    {
        blue = mybrightness.toInt();
        juiceLeds(stripId, red, green, blue, white);
        return 8;
    }
    if (mycolor == "white")
    {
        white = mybrightness.toInt();
        juiceLeds(stripId, red, green, blue, white);
        return 9;
    }
    if (mycolor == "all")
    {
        white = red = green = blue = mybrightness.toInt();
        juiceLeds(stripId, red, green, blue, white);
        return 9;
    }
    if (mycolor == "reset")
    {
        System.reset();
        return 99;
    }
    else
        return 0;
}

void storeLedValues(int stripId, int ured, int ugreen, int ublue, int uwhite)
{

    // EEPROM.get(CONFIGADDR,readConfig);
        EEPROM.put(LEDBASEADDR + stripId, LEDS[stripId - 1]);
        String myLEDString = String(stripId) + ":" + String(ured);
        Particle.publish(deviceName+"/store",myLEDString);

    switch (stripId)
    { case 1:
        myLED1 = {1, ured, ugreen, ublue, uwhite};
        break;
      case 2:
        myLED2 = {2, ured, ugreen, ublue, uwhite};
        break;
      case 3:
        myLED3 = {3, ured, ugreen, ublue, uwhite};
        break;
    }

}

void juiceLeds(int stripId, int ured, int ugreen, int ublue, int uwhite)
{

    // Store the struct values
    if (modeName == "store")
    {
        storeLedValues(stripId, ured, ugreen, ublue, uwhite);
    }

    if (stripId == 1)
    {
        //Particle.publish("juiceleds", String(stripId));

        int pix = 18;
        for (int n = 0; n < pix; n++)
        {
            strip1.setPixelColor(n, ugreen, ured, ublue, uwhite);
            delay(10);
            strip1.show();
        }
    }
    else if (stripId == 2)
    {
        int pix = 81;
        for (int n = 0; n < pix; n++)
        {
            strip2.setPixelColor(n, ugreen, ured, ublue, uwhite);
            delay(10);
            strip2.show();
        }
    }
    else if (stripId == 3)
    {
        int pix = 84;
        for (int n = 0; n < pix; n++)
        {
            strip3.setPixelColor(n, ugreen, ured, ublue, uwhite);
            delay(10);
            strip3.show();
        }
    }
    else if (stripId == 4)
    {
        int pix = 84;
        for (int n = 0; n < pix; n++)
        {
            strip3.setPixelColor(n, ugreen, ured, ublue, uwhite);
        }
        strip3.show();
    }
    else if (stripId == 5)
    {

        strip1.clear();
        strip1.show();
        strip2.clear();
        strip2.show();
        strip3.clear();
        strip3.show();
    }
    else
    {
        Particle.publish("ERROR", "Did not find stripid");
    }
}

void ledhandler(const char *event, const char *data)
{
    String stdata = String(data);
    //String myName = DeviceNameHelperEEPROM::instance().getName();
    Particle.publish(deviceName, stdata);
    //Particle.publish("hour",String(myHour));  // prints the hour

    ledConfig(String(stdata));
}

void motionHandler()
{
  // toggle blue led D7
    digitalWrite(led2, HIGH);
    lastMotionTime = millis();

    //lastMotionAt = String(Time.hour()-5)+":"+String(Time.minute());
    Time.zone(-5);
    lastMotionAt = Time.format("%I:%M %p");


    // restore
    if ( readConfig.mode == 2 & readConfig.motionEnabled == true & secSinceMotion < 2 ) {
        restore("motion detected");
    }
    return;
}

int setConfig(String command)
{

    int seperator = command.indexOf("/");
    String key = command.substring(0, seperator);
    String value = command.substring(seperator + 1);

    EEPROM.get(CONFIGADDR, readConfig);

    if (key == "read")
    {
        //Particle.publish("mode", String(readConfig.mode));
        getModeName();
        Particle.publish("motionEnabled", String(readConfig.motionEnabled));
        Particle.publish("awayHoldTMR", String(readConfig.awayHoldTMR));
        Particle.publish("gestureArmed", String(readConfig.gestureArmed));
    }

    if (key == "led")
    {

        // set the led values in the MyLedConfig structs
        for (int x = 0; x < 3; x = x + 1)
        {
            String name = "LED" + String(x + 1);
            String prntLed = String(LEDS[x].stripid);
            prntLed += ":" + String(LEDS[x].red);
            prntLed += ":" + String(LEDS[x].green);
            prntLed += ":" + String(LEDS[x].blue);
            prntLed += ":" + String(LEDS[x].white);
            Particle.publish(name, prntLed);
        }
    }
    if (key == "eeprom")
    {
        // and read it from the eeprom
        for (int x = 1; x < 4; x = x + 1)
        {
            MyLEDConfig tmp;
            EEPROM.get(LEDBASEADDR + x, tmp);
            String key = "EEPROM:" + String(LEDBASEADDR + x);
            String value = String(tmp.stripid);
            value += ":" + String(tmp.red);
            value += ":" + String(tmp.green);
            value += ":" + String(tmp.blue);
            value += ":" + String(tmp.white);
            Particle.publish(key, value);
        }
    }

    if (key == "init")
    {
        MyConfig tmp = {1, false, false, 0, 600};
        EEPROM.put(CONFIGADDR, tmp);
        Particle.publish("eeprom", "initalized");
    }

    if (key == "awayHoldTMR")
    {
        readConfig.awayHoldTMR = value.toInt();
        EEPROM.put(CONFIGADDR, readConfig);
    }

    if (key == "motionEnabled")
    {
        readConfig.motionEnabled = value.toInt();
        EEPROM.put(CONFIGADDR, readConfig);
    }
    if (key == "mode")
    {
        readConfig.mode = value.toInt();
        EEPROM.put(CONFIGADDR, readConfig);
        // call this function to set the new named value
        getModeName();
    }
    if (key == "reset")
    {
        System.reset();
    }

    // Do a final get config
    EEPROM.get(CONFIGADDR, readConfig);
    return 1;
}

int suspend(String command){

    // default to force if Idle is not passed
    if ( command != "Idle" ){
        command = "forced";
    }

    // 1 read config
    EEPROM.get(CONFIGADDR, readConfig);
    // 2 Change mode to suspend
    readConfig.mode = 2;
    EEPROM.put(CONFIGADDR, readConfig);
    // 3 turn off the lights
    Particle.publish(deviceName+"/suspend",command);
    getModeName();
    // now turn the lights off
        strip1.clear();
        strip1.show();
        strip2.clear();
        strip2.show();
        strip3.clear();
        strip3.show();

return 1;
}

int restore(String command){
    // read the value from the eeprom
    // convert to values
    // juice the leds
    juiceLeds(myLED1.stripid,myLED1.red,myLED1.green,myLED1.blue,myLED1.white);
    juiceLeds(myLED2.stripid,myLED2.red,myLED2.green,myLED2.blue,myLED2.white);
    juiceLeds(myLED3.stripid,myLED3.red,myLED3.green,myLED3.blue,myLED3.white);
    // change the mode
     EEPROM.get(CONFIGADDR, readConfig);
    readConfig.mode = 1;  // mode 1 store 
     EEPROM.put(CONFIGADDR, readConfig);
     getModeName();

    return 1;
}

void getModeName()
{
    EEPROM.get(CONFIGADDR, readConfig);
    switch (readConfig.mode)
    {
    case 0:
        modeName = "none";
        break;
    case 1:
        modeName = "store";
        break;
    case 2:
        modeName = "suspend";
        break;
    case 3:
        modeName = "foo";
        break;
    default:
        modeName = "default";
    }
    Particle.publish("mode", modeName);
}
s