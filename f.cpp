#include <FirebaseESP32.h>





#include <FirebaseJson.h>






#include <ESPAsyncWebServer.h>



#include <ESP32_MailClient.h>














#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>
#include <WiFi.h>
const char* ssid = "DESKTOP-L0Q9OS5 68619";
const char* password ="Eyad123456";
// Firebase configuration
#define FIREBASE_HOST "https://ledd-7f9b7-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "AIzaSyDABKV5wtahg_PRmTgkH4JXE16gfd8ZxkE"

// Gmail SMTP Server
#define emailSenderAccount    "oa9280255@gmail.com"    
#define emailSenderPassword   "kptolehuhkysgsty"
#define emailRecipient        "abbassamira549@gmail.com"
#define smtpServer            "smtp.gmail.com"
#define smtpServerPort        465
#define emailSubject          "ESP32 Test"

// Define the ultrasonic sensor pins
#define TRIG_PIN 23
#define ECHO_PIN 5

// Define the servo motor pin
#define SERVO_PIN 26

// Define the LED pins
#define LED_1_PIN 19
#define LED_2_PIN 27

// Define the light sensor pin
#define LIGHT_SENSOR_PIN 34

// Define the flame sensor pin
#define FLAME_SENSOR_PIN 33


// Define the buzzer pin

#define BUZZER_PIN_2 2
// Create a servo object
Servo servo;
FirebaseAuth auth;
FirebaseData fbdo;
FirebaseConfig config;
// Create LCD object
LiquidCrystal_I2C lcd(0x27, 16, 2); // Change the address if needed
SMTPData smtpData;

AsyncWebServer server(80);
int servoPosition = 90; 
void sendCallback(SendStatus info);
void setup() {
  // Initialize the serial port
  Serial.begin(115200);

  // Connect to WiFi network
   WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  
  // Print ESP32 Local IP Address
  Serial.print("IP Address: http://");
  Serial.println(WiFi.localIP());

  // Initialize Firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Attach the servo to the specified pin
  servo.attach(SERVO_PIN);

  // Set the LED pins as outputs
  pinMode(LED_1_PIN, OUTPUT);
  pinMode(LED_2_PIN, OUTPUT);

  // Initialize the light sensor
  pinMode(LIGHT_SENSOR_PIN, INPUT);
    server.on("/rotate_servo", HTTP_GET, [](AsyncWebServerRequest *request){
    servo.write(90); // Rotate the servo to 90 degrees
    request->send(200, "text/plain", "Servo rotated");
  });

  server.on("/stop_servo", HTTP_GET, [](AsyncWebServerRequest *request){
    servo.write(0); // Rotate the servo to 0 degrees
    request->send(200, "text/plain", "Servo stopped");
  });
    server.on("/turn_on_led1", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(LED_1_PIN, HIGH); // Toggle LED 1
    request->send(200, "text/plain", "Turn On LED 1");
  });

  server.on("/turn_on_led2", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(LED_2_PIN,HIGH); // Toggle LED 2
    request->send(200, "text/plain", "Turn On LED 2");
  });
  server.begin();

  
  // Initialize the flame sensor
  pinMode(FLAME_SENSOR_PIN, INPUT);

  // Initialize the buzzer
  

  pinMode(BUZZER_PIN_2, OUTPUT);
  
  // Initialize LCD
  lcd.init();
  lcd.backlight();
  digitalWrite(LED_1_PIN, HIGH);
  digitalWrite(LED_2_PIN, LOW);
  // Turn off both LEDs
  
  digitalWrite(BUZZER_PIN_2, LOW);
}

void loop() {
  // Send a trigger pulse to the ultrasonic sensor

     digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  
  // Wait for the echo pulse to return
  long duration = pulseIn(ECHO_PIN, HIGH);

  // Calculate the distance to the object
  float distance = duration * 0.034 / 2;

  // Read the light sensor value  
  int light_sensor_value = analogRead(LIGHT_SENSOR_PIN);

  // Read the flame sensor value
  int flame_sensor_value = digitalRead(FLAME_SENSOR_PIN);

  // Display on LCD
  lcd.setCursor(0, 0);
  lcd.print("Distance: ");
  lcd.print(distance);
  lcd.print(" cm ");
  lcd.setCursor(0, 1);
  lcd.print("Light: ");
  lcd.print(light_sensor_value);
  lcd.print("    ");

// If the distance is less than 100 cm and the flame sensor value is above a certain threshold, rotate the servo motor, turn on LEDs, activate buzzer, and send alert
  if (distance < 100) {
    servo.write(180);
    digitalWrite(LED_1_PIN, LOW);
    digitalWrite(LED_2_PIN, HIGH);
  }

  // If the light sensor value is less than 500, rotate the servo motor to 180 degrees and turn on both LEDs
  else if (light_sensor_value < 500) {
    servo.write(90);
    digitalWrite(LED_1_PIN, HIGH);
    digitalWrite(LED_2_PIN, HIGH);
  }

  // If the distance is greater than 100 cm and the light sensor value is greater than 500, rotate the servo motor to 0 degrees and turn off both LEDs
  
  else{
    servo.write(90);
    digitalWrite(LED_1_PIN, HIGH);
    digitalWrite(LED_2_PIN, LOW);
  }

   if (flame_sensor_value == 1){
   
    digitalWrite(BUZZER_PIN_2,HIGH);
     lcd.setCursor(0, 0);
    lcd.print("flame detected");
    digitalWrite(LED_1_PIN,HIGH);
     digitalWrite(LED_2_PIN,HIGH);
    delay(1000);
   
    digitalWrite(BUZZER_PIN_2,LOW);
     digitalWrite(LED_1_PIN,HIGH);
      digitalWrite(LED_2_PIN,LOW);

     smtpData.setLogin(smtpServer, smtpServerPort, emailSenderAccount, emailSenderPassword);
  smtpData.setSender("ESP32", emailSenderAccount);
  // Initialize the ultrasonic sensor
  // Set Email priority or importance High, Normal, Low or 1 to 5 (1 is highest)
  smtpData.setPriority("High");

  // Set the subject
  smtpData.setSubject(emailSubject);

  // Set the message with HTML format
  smtpData.setMessage("<div style=\"color:#2f4468;\"><h1>Fire Detected</h1><p>- GO Home Quickly</p></div>", true);
  // Set the email message in text format (raw)
  //smtpData.setMessage("Hello World! - Sent from ESP32 board", false);

  // Add recipients, you can add more than one recipient
  smtpData.addRecipient(emailRecipient);
  //smtpData.addRecipient("YOUR_OTHER_RECIPIENT_EMAIL_ADDRESS@EXAMPLE.com");

  smtpData.setSendCallback(sendCallback);

  //Start sending Email, can be set callback function to track the status
  if (!MailClient.sendMail(smtpData))
    Serial.println("Error sending Email, " + MailClient.smtpErrorReason());

  //Clear all data from Email object to free memory
  smtpData.empty();
  }
  FirebaseJson json;
  json.add("distance", distance);
  json.add("light_sensor", light_sensor_value);
  json.add("flame_sensor", flame_sensor_value);

  String path = "/sensor_readings"; // Change the reference path as needed
  Firebase.updateNode(fbdo, path, json);
  // ... (rest of the loop code)
  
}
      
void  sendCallback(SendStatus msg) {
  Serial.println(msg.info());

  // Do something when complete
  if (msg.success()) {
    Serial.println("----------------");
  }
}
 
  // Implement sending email alert using Gmail SMTP server
  // This part is complex and depends on your requirements and the libraries you choose
  // You might need to use libraries like "WiFiClientSecure", "Base64", or others to handle secure connections and email formatting
