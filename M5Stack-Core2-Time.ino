// M5Core2 - Digital and Analog clock
// https://github.com/m5stack/M5Core2/tree/master/src

#include <M5Core2.h>

// Variable declatation

// Time
RTC_TimeTypeDef RTCtime;
char timeStrbuff[64];
String h, m;
int lastSecond;

// Visual
int br = 2900;  // Initial brightness level 
bool digital = true, dark = true, sAnalog = true;
float sx, sy, mx, my, hx, hy, sdeg, mdeg, hdeg;
uint16_t centerX = 160, centerY = 120, osx, osy, omx, omy, ohx, ohy, x0, x1, yy0, yy1;
#define deg2rad 0.01745329251994329576923690768489

// IMU
float accX, accY, accZ;

// Define a new button in the center of the screen to switch between dark and light modes
Button Mid = Button(130,120,70,40, true, "Mid");

void setup()
{
  // Settings
  M5.begin(true,false,true,true);
  M5.IMU.Init();

  // Set initial screen
  M5.Axp.SetLcdVoltage(br); 
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Lcd.fillScreen(TFT_BLACK);

  // If time is wrong, can be set from the serial consol at any time.
  Serial.println("Send the current hour (24 Hours format): ");
}

void loop()
{
  // Main loop
  M5.update();

  // Read time and IMU values.
  M5.Rtc.GetTime(&RTCtime);
  M5.IMU.getAccelData(&accX,&accY,&accZ);

  // Serial data detected. Updating RTC time from console.
  if (Serial.available() > 0)
  {
    UpdateTime();
  }

  // Button A was pressed. Decreasing brightness.
  if(M5.BtnA.wasPressed())
  {
    br -= 100;
    if (br <= 2500)
    {
      br = 2500;
    }
    M5.Axp.SetLcdVoltage(br); 
  }
  
  // Button B was pressed or Accelerometer read 5 or higher in x axis, switch between digital and analog.
  if(M5.BtnB.wasPressed() || abs(accX) > 5) {
    digital = !digital;
    if (!digital) DrawClock();   // Call to a function to draw the clock.
    else {
      sAnalog = true;
      UpdateDark();
    }
  }

  // Button C was pressed. Increasing brightness.
  if(M5.BtnC.wasPressed())
  {
    br += 100;
    if (br >= 3300)
    {
      br = 3300;
    }
    M5.Axp.SetLcdVoltage(br);
  }

  if (digital) 
  {
    // Digital mode
    
    if (Mid.wasPressed()){
      // Center of screen was pressed, switching bettween dark and light modes.
      dark = !dark;
      UpdateDark();
    }

    // Load buffer with current time.
    sprintf(timeStrbuff,"%02d:%02d:%02d", RTCtime.Hours,RTCtime.Minutes,RTCtime.Seconds);

    // Display time in center of screen. Using font 7 because it looks like 7 segment display.
    M5.Lcd.drawString(timeStrbuff,50,90,7);
  }
  else 
  {
    // Analog mode
    if (RTCtime.Seconds != lastSecond) {
     // This gets executed every second
     lastSecond = RTCtime.Seconds;

     // Calculations to determine at what angle to point
     sdeg = RTCtime.Seconds * 6;                     // 0-59 -> 0-354
     mdeg = RTCtime.Minutes * 6 + sdeg * 0.01666667; // 0-59 -> 0-360 - includes seconds
     hdeg = RTCtime.Hours * 30 + mdeg * 0.0833333;   // 0-11 -> 0-360 - includes minutes and seconds

     hx = cos((hdeg-90)*deg2rad);    
     hy = sin((hdeg-90)*deg2rad);
     mx = cos((mdeg-90)*deg2rad);    
     my = sin((mdeg-90)*deg2rad);
     sx = cos((sdeg-90)*deg2rad);    
     sy = sin((sdeg-90)*deg2rad);

     // Draw the analog clock hands
     if ( RTCtime.Seconds==0 || sAnalog) {
      sAnalog = false;
      // Erase hour and minute hand positions every minute
      M5.Lcd.drawLine(ohx, ohy, centerX, centerY, TFT_BLACK);
      ohx = hx*62+centerX;    
      ohy = hy*62+centerY;
      M5.Lcd.drawLine(omx, omy, centerX, centerY, TFT_BLACK);
      omx = mx*84+centerX;    
      omy = my*84+centerY;
     }

     // Redraw new hand positions, hour and minute hands not erased here to avoid flicker, every minute.
     M5.Lcd.drawLine(osx, osy, centerX, centerY, TFT_BLACK);
     osx = sx*90+centerX;    
     osy = sy*90+centerY;
     M5.Lcd.drawLine(osx, osy, centerX, centerY, TFT_RED);
     M5.Lcd.drawLine(ohx, ohy, centerX, centerY, TFT_WHITE);
     M5.Lcd.drawLine(omx, omy, centerX, centerY, TFT_WHITE);
     M5.Lcd.drawLine(osx, osy, centerX, centerY, TFT_RED);
     M5.Lcd.fillCircle(centerX, centerY, 3, TFT_RED);
    }
  }
}

// Function used to set time reading from console or serial monitor. 
// First hours in (24 hours format) then minutes.
void UpdateTime() {
  h = Serial.parseInt();
  RTCtime.Hours = h.toInt();
  Serial.print("Send the minutes: ");
  while (Serial.available() > 0){ Serial.read(); }
  while (Serial.available() == 0){}
  m = Serial.parseInt();   
  RTCtime.Minutes = m.toInt();
  RTCtime.Seconds = 0;
  M5.Rtc.SetTime(&RTCtime);
  Serial.println();
  while (Serial.available() > 0){ Serial.read(); }
  Serial.println("Send the current hour (24 Hours format): ");
}

// Function used to switch between dark and light mode used when in digital mode and when coming out of analog mode.
void UpdateDark(){
  if (dark) {
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.fillScreen(TFT_BLACK); 
  }
  else {
     M5.Lcd.setTextColor(TFT_BLACK, TFT_WHITE);
     M5.Lcd.fillScreen(TFT_WHITE);
  }
}

// Function to draw clocks circle and lines. Only called onces when switching from digital to analog.
void DrawClock()
{
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.fillCircle(centerX, centerY, 118, TFT_GREEN);
  M5.Lcd.fillCircle(centerX, centerY, 110, TFT_BLACK);
  // Draw 12 lines
  for(int i = 0; i<360; i+= 30) {
    sx = cos((i-90)*deg2rad);
    sy = sin((i-90)*deg2rad);
    x0 = sx*114+centerX;
    yy0 = sy*114+centerY;
    x1 = sx*100+centerX;
    yy1 = sy*100+centerY;

    M5.Lcd.drawLine(x0, yy0, x1, yy1, TFT_GREEN);
  }
  // Draw 60 dots
  for(int i = 0; i<360; i+= 6) {
    sx = cos((i-90)*deg2rad);
    sy = sin((i-90)*deg2rad);
    x0 = sx*102+centerX;
    yy0 = sy*102+centerY;
    // Draw minute markers
    M5.Lcd.drawPixel(x0, yy0, TFT_WHITE);
    
    // Draw main quadrant dots
    if(i==0 || i==180) M5.Lcd.fillCircle(x0, yy0, 2, TFT_WHITE);
    if(i==90 || i==270) M5.Lcd.fillCircle(x0, yy0, 2, TFT_WHITE);
  }
  M5.Lcd.fillCircle(centerX, centerY, 3, TFT_WHITE);
}
