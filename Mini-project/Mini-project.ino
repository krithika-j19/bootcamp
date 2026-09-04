#include <U8g2lib.h>
#include <Wire.h>
#include <Servo.h>

// Page Buffer Mode for low RAM footprint
U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// 📌 NOTE: If your screen stays blank, comment out the line above and use this instead:
// U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

Servo myservo;   
int cm = 0;

// Pins Configuration
const int buzzerPin = 8; // NEW: Buzzer connected to Pin 8

const int gateMoveDuration = 650; 
const int STOP_PULSE = 1500; 

long readUltrasonicDistance(int triggerPin, int echoPin)
{
  pinMode(triggerPin, OUTPUT); 
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);
  pinMode(echoPin, INPUT);
  return pulseIn(echoPin, HIGH);
}

void setup() {
  digitalWrite(12, LOW);
  myservo.attach(6); 
  myservo.writeMicroseconds(STOP_PULSE); 
  
  pinMode(buzzerPin, OUTPUT); // Initialize buzzer pin
  Serial.begin(9600);

  u8g2.begin();
  
  // Startup chirp to confirm buzzer is working
  tone(buzzerPin, 2000, 100); 
}

void loop() {
  cm = 0.01723 * readUltrasonicDistance(2, 3);

  if(cm > 0 && cm <= 10){ 
    Serial.print("Car detected at: ");
    Serial.print(cm);
    Serial.println(" cm. Opening gate...");
    
    // NEW: Alert beep when car is first detected
    tone(buzzerPin, 1500, 150); 
    delay(150);
    
    // ---- OLED: SHOW OPENING STATUS ----
    u8g2.firstPage();
    do {
      u8g2.setFont(u8g2_font_ncenB12_tr);
      u8g2.drawStr(0, 20, "VEHICLE!!");
      u8g2.setFont(u8g2_font_6x10_tf);
      u8g2.setCursor(0, 40);
      u8g2.print("Distance: "); u8g2.print(cm); u8g2.print(" cm");
      u8g2.drawStr(0, 55, "Status: OPENING GATE");
    } while ( u8g2.nextPage() );

    // 1. OPEN GATE (with warning tone)
    tone(buzzerPin, 1000); // Continuous low pitch warning while moving
    myservo.writeMicroseconds(1570);             
    delay(gateMoveDuration);                       
    noTone(buzzerPin); // Turn off buzzer once gate stops
    
    // 2. HOLD POSITION
    myservo.writeMicroseconds(STOP_PULSE); 
    Serial.println("Gate UP. Waiting 3 seconds...");
    
    // ---- OLED: SHOW OPEN STATUS ----
    u8g2.firstPage();
    do {
      u8g2.setFont(u8g2_font_ncenB12_tr);
      u8g2.drawStr(0, 20, "GATE OPEN");
      u8g2.setFont(u8g2_font_6x10_tf);
      u8g2.drawStr(0, 40, "Drive safely.");
      u8g2.drawStr(0, 55, "Waiting: 3 seconds...");
    } while ( u8g2.nextPage() );

    delay(3000); 

    // 3. CLOSE GATE
    Serial.println("Closing gate...");
    
    // ---- OLED: SHOW CLOSING STATUS ----
    u8g2.firstPage();
    do {
      u8g2.setFont(u8g2_font_ncenB12_tr);
      u8g2.drawStr(0, 20, "CLOSING...");
      u8g2.setFont(u8g2_font_6x10_tf);
      u8g2.drawStr(0, 45, "Securing barrier.");
    } while ( u8g2.nextPage() );

    // Rapid double-beep alert right before closing
    tone(buzzerPin, 800, 100); delay(150);
    tone(buzzerPin, 800, 100); delay(150);

    tone(buzzerPin, 1200); // Play tone while lowering barrier
    myservo.writeMicroseconds(1430);
    delay(gateMoveDuration);                                    
    noTone(buzzerPin); 
    
    // 4. LOCK GATE
    myservo.writeMicroseconds(STOP_PULSE);
    Serial.println("Gate DOWN.");
    
    delay(2000); 
  } else {
    myservo.writeMicroseconds(STOP_PULSE);
    noTone(buzzerPin); // Ensure buzzer remains silent while idle

    // ---- OLED: IDLE STATUS ----
    u8g2.firstPage();
    do {
      u8g2.setFont(u8g2_font_ncenB12_tr);
      u8g2.drawStr(0, 20, "GATE LOCKED");
      u8g2.setFont(u8g2_font_6x10_tf);
      u8g2.setCursor(0, 40);
      u8g2.print("Sensor Dist: "); u8g2.print(cm); u8g2.print(" cm");
      u8g2.drawStr(0, 55, "Status: Ready / Idle");
    } while ( u8g2.nextPage() );
  }                         
  
  delay(100); 
}