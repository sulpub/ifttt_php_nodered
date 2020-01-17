/*
 Neopixel Light with ESP8266 using MQTT 
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_NeoPixel.h>
#include <stdio.h>

//OTA function
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

/*
information
Define wifi and MQTT informations in global parameters

For changing zone replace the text on the entire software (replace all)
And change the number of led of the Neopixel band.
salle1 : 20 led + alarm pir
salle2 : 6 led 
salle3 : 21 led pour la led meuble salon
salle4 : 24 led anneau derriere meuble salon

If no alarm module put define ALARM_MODULE at 0.
 */

#define MODULE  "salle1"
#define ALARM_MODULE  1

//librairie clock
#include <WiFiUdp.h>

// GPIO2 on the ESP-01 for controlling NeoPixels
// D4 sur LOLIN -> GPIO2 for controlling NeoPixels
#define PIN            2

// Number of led on the Neopixels stripline
#define NUMPIXELS      20

// Adafruit noepixel librairy
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

/*
  ________.__        ___.          .__                                              __                       
 /  _____/|  |   ____\_ |__ _____  |  |   ___________ ____________    _____   _____/  |_  ___________  ______
/   \  ___|  |  /  _ \| __ \\__  \ |  |   \____ \__  \\_  __ \__  \  /     \_/ __ \   __\/ __ \_  __ \/  ___/
\    \_\  \  |_(  <_> ) \_\ \/ __ \|  |__ |  |_> > __ \|  | \// __ \|  Y Y  \  ___/|  | \  ___/|  | \/\___ \ 
 \______  /____/\____/|___  (____  /____/ |   __(____  /__|  (____  /__|_|  /\___  >__|  \___  >__|  /____  >
        \/                \/     \/       |__|       \/           \/      \/     \/          \/           \/ 
*********************************************************************************************************************
 */
//Globals parameters
// value for wifi network.
const char* ssid          = "********";
const char* password      = "***********";
const char* mqtt_server   = "XXX.XXX.XXX.XXX";  //Ip serveur MQTT

// value for MQTT broker.
String clientId           = "led_anneau_salle1";
String clientIdfix        = "led_anneau_salle1";
String subscribe_str      = "ESP8266_salle1";
const char* subscribe_char= "ESP8266_salle1";
String client_login       = "login";      		// login mqtt
String client_pwd         = "password";    		//password mqtt

//End globals parameters
//******************************************************************************************************************

//delay for loading led strip
int delayval                      = 25; // delay in milliseconds

// interval envoi status toute les 10 secondes
unsigned long currentMillis       = 0;
unsigned long previousMillis      = 0;   // will store last time LED was updated
long interval                     = 10000;              // en (milliseconds 10s)
long now                          = 0;

//variable for PIR alarm
int sensorPir                     = 0;
unsigned long currentMillis_pir   = 0;
unsigned long previousMillis_pir  = 0;   // will store last time LED was updated
long interval_pir                 = 3000;               // en (milliseconds 10s)
int alarm_present                 = 0;

long lastMsg                      = 0;
char msg[150];
char buf[10];

char buffer_tmp[200];

int value                         = 0;
String string_value               = "   ";
int red_value                     = 0;
int green_value                   = 0;
int blue_value                    = 0;

int red_value1                    = 0;
int green_value1                  = 0;
int blue_value1                   = 0;

int int_indice                    = 0;
unsigned char varr=0,varg=0,varb  = 0;
int int_alea                      = 0;
int init_info                     = 0;
int int_update                    = 0;
int int_i                         = 0;
int int_random_val                = 0;
int nb_test_ctl_wifi              = 0;
int nb_test_ctl_mqtt              = 0;

// Number de test max or retry connection
#define TEST_WIFI      20
#define TEST_MQTT      20

// Variables for candle animation
  // color variables: mix RGB (0-255) for desired yellow
  int redPx = 255;
  int grnHigh = 100; //110-120 for 5v, 135 for 3.3v
  int bluePx = 10; //10 for 5v, 15 for 3.3v
  // animation time variables, with recommendations
  int burnDepth = 10; //10 for 5v, 14 for 3.3v -- how much green dips below grnHigh for normal burn - 
  int flutterDepth = 25; //25 for 5v, 30 for 3.3v -- maximum dip for flutter
  int cycleTime = 120; //120 -- duration of one dip in milliseconds
  
  // pay no attention to that man behind the curtain
  int fDelay;
  int fRep;
  int flickerDepth;
  int burnDelay;
  int burnLow;
  int flickDelay;
  int flickLow;
  int flutDelay;
  int flutLow;
// End variables for candle animation

//Variables for clock
  unsigned int localPort = 2390;        // local port to listen for UDP packets
  IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server
  const int NTP_PACKET_SIZE = 48;       // NTP time stamp is in the first 48 bytes of the message
  byte packetBuffer[ NTP_PACKET_SIZE];  // buffer to hold incoming and outgoing packets
  
  // A UDP instance to let us send and receive packets over UDP
  WiFiUDP Udp;
  
  int global_hour=0;
  int global_min=0;
  int global_sec=0;
  
  void requete_time(void);
  unsigned long sendNTPpacket(IPAddress& address);
// End variables for clock

//Functions declaration 

void alarm_pir(void);
void red_status(void);
void green_status(void);
void blue_status(void);
void requete_time(void);
void all_led_off(void);
void special_animation(void);
void alarm_pir(void);
void status_led_mqtt(void);
void status_wifi(void);
void status_mqtt(void);

/*    
  _________       __                  __      __.______________.___ 
 /   _____/ _____/  |_ __ ________   /  \    /  \   \_   _____/|   |
 \_____  \_/ __ \   __\  |  \____ \  \   \/\/   /   ||    __)  |   |
 /        \  ___/|  | |  |  /  |_> >  \        /|   ||     \   |   |
/_______  /\___  >__| |____/|   __/    \__/\  / |___|\___  /   |___|
        \/     \/           |__|            \/           \/         
 */
void setup_wifi() {

  //rouge probleme
  red_status();
  
  delay(10);

  //delai restart
  delay(random(3000)); // Delay for a period of time (in milliseconds).
  
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connection to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }

  //bleu wifi OK
  blue_status();

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP Adress: ");
  Serial.println(WiFi.localIP());

  //connection client horloge NTP
  Serial.println("\nStarting connection to server NTP");
  Udp.begin(localPort);
}



/*
              .__  .__ ___.                  __        _____   ______________________________
  ____ _____  |  | |  |\_ |__ _____    ____ |  | __   /     \  \_____  \__    ___/\__    ___/
_/ ___\\__  \ |  | |  | | __ \\__  \ _/ ___\|  |/ /  /  \ /  \  /  / \  \|    |     |    |   
\  \___ / __ \|  |_|  |_| \_\ \/ __ \\  \___|    <  /    Y    \/   \_/.  \    |     |    |   
 \___  >____  /____/____/___  (____  /\___  >__|_ \ \____|__  /\_____\ \_/____|     |____|   
     \/     \/              \/     \/     \/     \/         \/        \__>                   
 */

void callback_mqtt(char* topic, byte* payload, unsigned int length) 
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print(" - long ");
  Serial.print(length);
  Serial.print("] ");
  string_value="";
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    string_value +=(char)payload[i];
  }
  Serial.println();

  value = string_value.toInt();
   
  /*Serial.print("value [");
  Serial.print(string_value);
  Serial.print(" - ");
  Serial.print(value);
  Serial.print("] ");
  Serial.println();*/

  // Red led configuration
  sprintf(buffer_tmp, "set/%s/red", subscribe_str.c_str());
  //Serial.println(buffer_tmp);
  if (strcmp (topic,buffer_tmp) == 0)
  {
      //init affichage info une seule fois
      init_info=1;

      //Serial.println("red");
      red_value = value;

      for(int i=0;i<NUMPIXELS;i++){   
      // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
      pixels.setPixelColor(i, pixels.Color(red_value,green_value,blue_value)); // Moderately bright green color.    
      pixels.show(); // This sends the updated pixel color to the hardware.   
      delay(delayval); // Delay for a period of time (in milliseconds).
      }
  }


  // Green led configuration
  sprintf(buffer_tmp, "set/%s/green", subscribe_str.c_str());
  //Serial.println(buffer_tmp);
  if (strcmp (topic,buffer_tmp) == 0)
  {   
      //init affichage info une seule fois
      init_info=1;
      
      //Serial.println("green");
      green_value = value;

      for(int i=0;i<NUMPIXELS;i++){   
      // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
      pixels.setPixelColor(i, pixels.Color(red_value,green_value,blue_value)); // Moderately bright green color.    
      pixels.show(); // This sends the updated pixel color to the hardware.   
      delay(delayval); // Delay for a period of time (in milliseconds).
      }
  }


  // Blue led configuration
  sprintf(buffer_tmp, "set/%s/blue", subscribe_str.c_str());
  //Serial.println(buffer_tmp);
  if (strcmp (topic,buffer_tmp) == 0)
  {
      //init affichage info une seule fois
      init_info=1;

      //Serial.println("blue");
      blue_value = value;

      for(int i=0;i<NUMPIXELS;i++){   
      // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
      pixels.setPixelColor(i, pixels.Color(red_value,green_value,blue_value)); // Moderately bright green color.    
      pixels.show(); // This sends the updated pixel color to the hardware.   
      delay(delayval); // Delay for a period of time (in milliseconds).
      }
  }    
  
}



WiFiClient espClient;
PubSubClient client(mqtt_server, 1883, callback_mqtt,espClient);



/*                             
__________                                                 __       _____   ______________________________
\______   \ ____   ____  ____   ____   ____   ____   _____/  |_    /     \  \_____  \__    ___/\__    ___/
 |       _// __ \_/ ___\/  _ \ /    \ /    \_/ __ \_/ ___\   __\  /  \ /  \  /  / \  \|    |     |    |   
 |    |   \  ___/\  \__(  <_> )   |  \   |  \  ___/\  \___|  |   /    Y    \/   \_/.  \    |     |    |   
 |____|_  /\___  >\___  >____/|___|  /___|  /\___  >\___  >__|   \____|__  /\_____\ \_/____|     |____|   
        \/     \/     \/           \/     \/     \/     \/               \/        \__>                   
 */
void reconnect_mqtt() {

  int compteur_mqtt=0;

  //MQTT PB
  blue_status(); 
   
  // Loop until we're reconnected
  while (!(client.connect(clientId.c_str(), client_login.c_str(), client_pwd.c_str()))) {

    //bleu wifi OK MQTT PB
    blue_status(); 
  
    compteur_mqtt++;
    if (compteur_mqtt==10) break;
    
    Serial.print("Wait for MQTT connexion...");

    //delai reboot
    delay(random(1000)); // Delay for a period of time (in milliseconds).
  
    // Create a random client ID
    client.setCallback(callback_mqtt);
  
    // Attempt to connect
    if (client.connect(clientId.c_str(), client_login.c_str(), client_pwd.c_str())) 
      {
          Serial.println("MQTT connection OK");
            
          delay(2000);
          snprintf (msg, 75, "online");
          Serial.print("Publish message: ");
          Serial.print(clientId.c_str());
          Serial.print(" ");
          Serial.println(msg);
          client.publish(clientIdfix.c_str(), msg);

          // init subscribe
          sprintf(buffer_tmp, "set/%s/#", subscribe_str.c_str());
          //Serial.println(buffer_tmp);
          client.subscribe(buffer_tmp);

          //Reload MQTT
          client.setCallback(callback_mqtt);

          //Blue <=> wifi ok after Green <=> MQTT OK
          pixels.setPixelColor(0, pixels.Color(0,20,0)); // Moderately bright green color.    
          pixels.setPixelColor(0, pixels.Color(0,20,0)); // Moderately bright green color.    
          pixels.setPixelColor(0, pixels.Color(0,20,0)); // Moderately bright green color.    
          pixels.show(); // This sends the updated pixel color to the hardware.   
          delay(delayval); // Delay for a period of time (in milliseconds).
        
      }
      else 
      {
        Serial.print("Fail, rc=");
        Serial.print(client.state());
        Serial.println(" Restart in 5 seconds");
      }
      // Wait 5 seconds before retrying
      delay(5000);
  }

      if (client.connect(clientId.c_str(), client_login.c_str(), client_pwd.c_str())) 
      {
          Serial.println("MQTT connection OK");
        
          delay(2000);
          //delai reboot
          delay(random(3000)); // Delay for a period of time (in milliseconds).
  
          snprintf (msg, 75, "online");
          Serial.print("Publish message: ");
          Serial.print(clientId.c_str());
          Serial.print(" ");
          Serial.println(msg);
          client.publish(clientIdfix.c_str(), msg);

          // init subscribe
          sprintf(buffer_tmp, "set/%s/#", subscribe_str.c_str());
          //Serial.println(buffer_tmp);
          client.subscribe(buffer_tmp);

          //Reload MQTT
          client.setCallback(callback_mqtt);

          //Green <=> wifi and MQTT OK
          green_status();      
      }
}


/*                
               __                
  ______ _____/  |_ __ ________  
 /  ___// __ \   __\  |  \____ \ 
 \___ \\  ___/|  | |  |  /  |_> >
/____  >\___  >__| |____/|   __/ 
     \/     \/           |__|    

 */
void setup() {
  
  //init variable for candle animation
  flickerDepth = (burnDepth + flutterDepth) / 2.4;
  burnLow = grnHigh - burnDepth;
  burnDelay = (cycleTime / 2) / burnDepth;
  flickLow = grnHigh - flickerDepth;
  flickDelay = (cycleTime / 2) / flickerDepth;
  flutLow = grnHigh - flutterDepth;
  flutDelay = ((cycleTime / 2) / flutterDepth);

  //random emission status
  interval=10000+random(3000);
  
  //init led neopixel
  pixels.begin(); // This initializes the NeoPixel library.

  //turn off all the neopixel led at the begining
  for(int i=0;i<NUMPIXELS;i++){   
	  // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
	  pixels.setPixelColor(i, pixels.Color(0,0,0)); // Moderately bright green color.    
	  pixels.show(); 	// This sends the updated pixel color to the hardware.   
	  delay(delayval); 	// Delay for a period of time (in milliseconds).
	  }

  //Use the first for indicate status of connecion red <=> Problem
  pixels.setPixelColor(0, pixels.Color(20,0,0));    
  pixels.show(); 		// This sends the updated pixel color to the hardware.   
  delay(delayval); 		// Delay for a period of time (in milliseconds).
  
  //init UART for debug
  Serial.begin(115200);
  Serial.println("");
  Serial.println("Reboot");  
  Serial.println("Setup run");
  
  //Wifi configuration and connection
  setup_wifi();

  // abonnement MQTT
  client.setCallback(callback_mqtt);

  //connect MQTT
  reconnect_mqtt();

  //alarm present
  alarm_present=ALARM_MODULE;

  //OTA Function
  // Port defaults to 8266
  ArduinoOTA.setPort(8266);
  // Hostname defaults to esp8266_location
  ArduinoOTA.setHostname(subscribe_char);
  // No authentication by default
  ArduinoOTA.setPassword((const char *)"password_ota");
  ArduinoOTA.begin();
       
} //fin setup



/*
   _____         .__         .____                         
  /     \ _____  |__| ____   |    |    ____   ____ ______  
 /  \ /  \\__  \ |  |/    \  |    |   /  _ \ /  _ \\____ \ 
/    Y    \/ __ \|  |   |  \ |    |__(  <_> |  <_> )  |_> >
\____|__  (____  /__|___|  / |_______ \____/ \____/|   __/ 
        \/     \/        \/          \/            |__|    
 */
void loop() {   

  //test wifi et reconnexion si probleme
  status_wifi();

  //status MQTT
  status_mqtt();

  client.loop();
  
  //status led MQTT
  status_led_mqtt();

  //run specific animation
  special_animation();

  //alarm module PIR
  alarm_pir();

  //OTA Function
  ArduinoOTA.handle();
    
} // fin void loop 



/*
  _________ __          __                    _____   ______________________________
 /   _____//  |______ _/  |_ __ __  ______   /     \  \_____  \__    ___/\__    ___/
 \_____  \\   __\__  \\   __\  |  \/  ___/  /  \ /  \  /  / \  \|    |     |    |   
 /        \|  |  / __ \|  | |  |  /\___ \  /    Y    \/   \_/.  \    |     |    |   
/_______  /|__| (____  /__| |____//____  > \____|__  /\_____\ \_/____|     |____|   
        \/           \/                \/          \/        \__>                   
 */
void status_mqtt(void)
{  
  //test MQTT et reconnexion si probleme
  if (!(client.connect(clientId.c_str(), client_login.c_str(), client_pwd.c_str()))) 
  {
    nb_test_ctl_mqtt++;
    if (nb_test_ctl_mqtt>TEST_MQTT)
    {
      nb_test_ctl_mqtt=0;
      Serial.println("MQTT disconnection (pb MQTT)");
      reconnect_mqtt();
    }
  }
  else
  {

  }
}



/*
  _________ __          __                  __      __.______________.___ 
 /   _____//  |______ _/  |_ __ __  ______ /  \    /  \   \_   _____/|   |
 \_____  \\   __\__  \\   __\  |  \/  ___/ \   \/\/   /   ||    __)  |   |
 /        \|  |  / __ \|  | |  |  /\___ \   \        /|   ||     \   |   |
/_______  /|__| (____  /__| |____//____  >   \__/\  / |___|\___  /   |___|
        \/           \/                \/         \/           \/         
 */
void status_wifi(void)
{
    
  if (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    nb_test_ctl_wifi++;
    if (nb_test_ctl_wifi>TEST_WIFI)
    {
      nb_test_ctl_wifi=0;
      Serial.println("Wifi disconnection (pb wifi)\r\n");
      setup_wifi();
      Serial.println("MQTT disconnection(pb MQTT)\r\n");
      reconnect_mqtt();
    }
  }
}


/*
  _________ __          __                 .____     ___________________       ___________________________
 /   _____//  |______ _/  |_ __ __  ______ |    |    \_   _____/\______ \     /     \__    ___/\__    ___/
 \_____  \\   __\__  \\   __\  |  \/  ___/ |    |     |    __)_  |    |  \   /  \ /  \|    |     |    |   
 /        \|  |  / __ \|  | |  |  /\___ \  |    |___  |        \ |    `   \ /    Y    \    |     |    |   
/_______  /|__| (____  /__| |____//____  > |_______ \/_______  //_______  / \____|__  /____|     |____|   
        \/           \/                \/          \/        \/         \/          \/                    
 */
//Information status of led to the MQTT server
void status_led_mqtt(void)
{
  delay(500);
      
  //delai envoi status
  currentMillis = millis();

  if (currentMillis - previousMillis >= interval) 
  {
      //Abonnement MQTT
      client.setCallback(callback_mqtt);
      
      //ramdom emission status
      interval=10000+random(3000);
        
      //mise a jour horloge
      int_update=1;
      
      // save the last time you blinked the LED
      previousMillis = currentMillis;

      if ((client.connect(clientId.c_str(), client_login.c_str(), client_pwd.c_str()))) 
      {
        //envoi status 
        if (red_value==0 && green_value==0 && blue_value==0)
        {
          snprintf (msg, 75, "OFF:R:V:B;0;0;0");
          client.publish(clientIdfix.c_str(), msg);
          Serial.print("0");
        }
        else
        {
          snprintf (msg, 75, "ON:R:V:B;%d;%d;%d",red_value,green_value,blue_value);
          client.publish(clientIdfix.c_str(), msg);
          Serial.print("1");
        } 

        // init subscribe
        sprintf(buffer_tmp, "set/%s/#", subscribe_str.c_str());
        //Serial.println(buffer_tmp);
        client.subscribe(buffer_tmp);
    
        //rechargement ecoute MQTT
        client.setCallback(callback_mqtt);  
                       
      }

      //recupération horloge NTP UDP
      if (green_value==260)
      requete_time();
      
  }//fin if attente emission status led.  
}


/*
   _____  .__                         __________._____________ 
  /  _  \ |  | _____ _______  _____   \______   \   \______   \
 /  /_\  \|  | \__  \\_  __ \/     \   |     ___/   ||       _/
/    |    \  |__/ __ \|  | \/  Y Y  \  |    |   |   ||    |   \
\____|__  /____(____  /__|  |__|_|  /  |____|   |___||____|_  /
        \/          \/            \/                        \/ 
 */
void alarm_pir(void)
{
  if(alarm_present==1)
  {
      // read the input on analog pin 0:
      sensorPir = analogRead(A0);
          
      currentMillis_pir = millis();
          
      //delai envoi status
      if (sensorPir>600)  //alarm detected
      {
        if (currentMillis_pir - previousMillis_pir >= interval_pir) 
        {
          // save the last time buffer fluctuation capteur PIR
          previousMillis_pir = currentMillis;
    
          //send data MTT and uart
          if (client.connect(clientId.c_str(), client_login.c_str(), client_pwd.c_str())) 
          {  
              snprintf (msg, 75, "alarm");
              Serial.print("Message ");
              Serial.print(clientId.c_str());
              Serial.print(" ");
              Serial.println(msg);
              client.publish(clientIdfix.c_str(), msg);       
          }      
        }
      } //if sensor pir value control
      
  } //fin define alarm
}  //fin fonction alarm pir


/*
   _____         .__                __  .__                                             .__       .__   
  /  _  \   ____ |__| _____ _____ _/  |_|__| ____   ____     ____________   ____   ____ |__|____  |  |  
 /  /_\  \ /    \|  |/     \\__  \\   __\  |/  _ \ /    \   /  ___/\____ \_/ __ \_/ ___\|  \__  \ |  |  
/    |    \   |  \  |  Y Y  \/ __ \|  | |  (  <_> )   |  \  \___ \ |  |_> >  ___/\  \___|  |/ __ \|  |__
\____|__  /___|  /__|__|_|  (____  /__| |__|\____/|___|  / /____  >|   __/ \___  >\___  >__(____  /____/
        \/     \/         \/     \/                    \/       \/ |__|        \/     \/        \/      
 */
void special_animation(void)
{
//The mqtt value of the green level upper 255 active special animation

/*
   _____         .__                __  .__                        ____ 
  /  _  \   ____ |__| _____ _____ _/  |_|__| ____   ____   ______ /_   |
 /  /_\  \ /    \|  |/     \\__  \\   __\  |/  _ \ /    \ /  ___/  |   |
/    |    \   |  \  |  Y Y  \/ __ \|  | |  (  <_> )   |  \\___ \   |   |
\____|__  /___|  /__|__|_|  (____  /__| |__|\____/|___|  /____  >  |___|
        \/     \/         \/     \/                    \/     \/        
*/

  //animation aleatoire
  if (green_value==256)
  {
    if(init_info==1)
    {
      init_info=0;
      Serial.println("allumage aleatoire");   
      all_led_off();
    }  
    varr=random(30);
    varg=random(30);
    varb=random(30);    
    pixels.setPixelColor(int_indice, pixels.Color(varr,varg,varb));
    pixels.show(); // This sends the updated pixel color to the hardware.
    delay(5);
    int_indice++;
    if (int_indice>=NUMPIXELS) int_indice=0;
  }

/*
   _____         .__                __  .__                       ________  
  /  _  \   ____ |__| _____ _____ _/  |_|__| ____   ____   ______ \_____  \ 
 /  /_\  \ /    \|  |/     \\__  \\   __\  |/  _ \ /    \ /  ___/  /  ____/ 
/    |    \   |  \  |  Y Y  \/ __ \|  | |  (  <_> )   |  \\___ \  /       \ 
\____|__  /___|  /__|__|_|  (____  /__| |__|\____/|___|  /____  > \_______ \
        \/     \/         \/     \/                    \/     \/          \/
 */
  //animation chenillard led rouge
  if (green_value==257)
  {    
    if(init_info==1)
    {
      init_info=0;
      Serial.println("Chenillard rouge");   
      all_led_off();
    }  
    pixels.setPixelColor((int_indice++)%NUMPIXELS, pixels.Color(5,0,0));
    pixels.setPixelColor((int_indice+1)%NUMPIXELS, pixels.Color(10,0,0));
    pixels.setPixelColor((int_indice+2)%NUMPIXELS, pixels.Color(50,0,0));
    pixels.setPixelColor((int_indice+3)%NUMPIXELS, pixels.Color(100,0,0));
    pixels.setPixelColor((int_indice-1)%NUMPIXELS, pixels.Color(0,0,0));
    pixels.setPixelColor((int_indice-1)%NUMPIXELS, pixels.Color(0,0,0));
    pixels.setPixelColor((int_indice-3)%NUMPIXELS, pixels.Color(0,0,0));
    pixels.setPixelColor((int_indice-4)%NUMPIXELS, pixels.Color(0,0,0));   
    pixels.show(); // This sends the updated pixel color to the hardware.
    delay(5);  //ms
  }

/*  
   _____         .__                __  .__                       ________  
  /  _  \   ____ |__| _____ _____ _/  |_|__| ____   ____   ______ \_____  \ 
 /  /_\  \ /    \|  |/     \\__  \\   __\  |/  _ \ /    \ /  ___/   _(__  < 
/    |    \   |  \  |  Y Y  \/ __ \|  | |  (  <_> )   |  \\___ \   /       \
\____|__  /___|  /__|__|_|  (____  /__| |__|\____/|___|  /____  > /______  /
        \/     \/         \/     \/                    \/     \/         \/ 
 */
  //animation chenillard led bleu
  if (green_value==258)
  { 
    if(init_info==1)
    {
      init_info=0;
      Serial.println("Chenillard bleu");   
      all_led_off();
    }   
    pixels.setPixelColor((int_indice++)%NUMPIXELS, pixels.Color(0,0,5));
    pixels.setPixelColor((int_indice+1)%NUMPIXELS, pixels.Color(0,0,10));
    pixels.setPixelColor((int_indice+2)%NUMPIXELS, pixels.Color(0,0,50));
    pixels.setPixelColor((int_indice+3)%NUMPIXELS, pixels.Color(0,0,100));
    pixels.setPixelColor((int_indice-1)%NUMPIXELS, pixels.Color(0,0,0));
    pixels.setPixelColor((int_indice-1)%NUMPIXELS, pixels.Color(0,0,0));
    pixels.setPixelColor((int_indice-3)%NUMPIXELS, pixels.Color(0,0,0));
    pixels.setPixelColor((int_indice-4)%NUMPIXELS, pixels.Color(0,0,0));   
    pixels.show(); // This sends the updated pixel color to the hardware.
    delay(5);  //ms
  }
 

/*
   _____         .__                __  .__                          _____  
  /  _  \   ____ |__| _____ _____ _/  |_|__| ____   ____   ______   /  |  | 
 /  /_\  \ /    \|  |/     \\__  \\   __\  |/  _ \ /    \ /  ___/  /   |  |_
/    |    \   |  \  |  Y Y  \/ __ \|  | |  (  <_> )   |  \\___ \  /    ^   /
\____|__  /___|  /__|__|_|  (____  /__| |__|\____/|___|  /____  > \____   | 
        \/     \/         \/     \/                    \/     \/       |__| 
 */
  //animation aleatoire 2 leds blanches
  if (green_value==259)
  {
    if(init_info==1)
    {
      init_info=0;
      Serial.println("allumage aleatoire 2 led blanche");   
      all_led_off();
    }  
    varr=random(30);
    varg=random(30);
    varb=random(30);    
    pixels.setPixelColor(int_indice, pixels.Color(0,0,0));
    pixels.show(); // This sends the updated pixel color to the hardware.
    delay(delayval);
    int_indice++;
    if (int_indice>=NUMPIXELS) 
    {
    int_indice=0;
    pixels.setPixelColor(random(NUMPIXELS), pixels.Color(255,255,255));
    pixels.setPixelColor(random(NUMPIXELS), pixels.Color(255,255,255));
    }
  }

/*
   _____         .__                __  .__                        .________
  /  _  \   ____ |__| _____ _____ _/  |_|__| ____   ____   ______  |   ____/
 /  /_\  \ /    \|  |/     \\__  \\   __\  |/  _ \ /    \ /  ___/  |____  \ 
/    |    \   |  \  |  Y Y  \/ __ \|  | |  (  <_> )   |  \\___ \   /       \
\____|__  /___|  /__|__|_|  (____  /__| |__|\____/|___|  /____  > /______  /
        \/     \/         \/     \/                    \/     \/         \/ 
 */
  //animation heure
  if (green_value==260)
  {
    if(init_info==1)
    {
      init_info=0;
      int_update=1;
      Serial.println("mode horloge");   
      all_led_off();
    }  
    
    if(int_update==1)
    {
      int_update=0;
  
      all_led_off();
    
      //heure
      global_hour=(global_hour%12);
      pixels.setPixelColor(global_hour, pixels.Color(10,0,0));
  
      //minute
      global_min=((global_min*12/60)%12);
      if(global_hour==global_min)
      {
        pixels.setPixelColor(global_min, pixels.Color(10,0,10));
      }
      else
      {
        pixels.setPixelColor(global_min, pixels.Color(0,0,10));
      }
      
      //seconde
      global_sec=((global_sec*12/60)%8);
      pixels.setPixelColor(global_sec+12, pixels.Color(0,0,10));  

      Serial.print(" - ");
      Serial.print(global_hour);
       
      Serial.print(":");
      Serial.print(global_min);
       
      Serial.print(":");
      Serial.println(global_sec);
            
      pixels.show(); // This sends the updated pixel color to the hardware.
      delay(delayval);
    }
  }  

/*
   _____         .__                __  .__                         ________
  /  _  \   ____ |__| _____ _____ _/  |_|__| ____   ____   ______  /  _____/
 /  /_\  \ /    \|  |/     \\__  \\   __\  |/  _ \ /    \ /  ___/ /   __  \ 
/    |    \   |  \  |  Y Y  \/ __ \|  | |  (  <_> )   |  \\___ \  \  |__\  \
\____|__  /___|  /__|__|_|  (____  /__| |__|\____/|___|  /____  >  \_____  /
        \/     \/         \/     \/                    \/     \/         \/ 
 */
  //slide color
  if (green_value==261)
  {
    if(init_info==1)
    {
      init_info=0;
      Serial.println("mode slide color");   
      all_led_off();
    }  
    
    int_random_val=random(6)+1;
    
    switch (int_random_val) 
      {
          case 1:
            //do something when var equals 1
            red_value1 = 0;
            green_value1 = 0;
            blue_value1 = 255;
            break;
          case 2:
            //do something when var equals 1
            red_value1 = 0;
            green_value1 = 255;
            blue_value1 = 0;
            break;
          case 3:
            //do something when var equals 1
            red_value1 = 0;
            green_value1 = 255;
            blue_value1 = 255;
            break;
          case 4:
            //do something when var equals 1
            red_value1 = 255;
            green_value1 = 0;
            blue_value1 = 0;
            break;
          case 5:
            //do something when var equals 1
            red_value1 = 255;
            green_value1 = 0;
            blue_value1 = 255;
            break;
          case 6:
            //do something when var equals 1
            red_value1 = 255;
            green_value1 = 255;
            blue_value1 = 0;
            break;
          case 7:
            //do something when var equals 1
            red_value1 = 255;
            green_value1 = 255;
            blue_value1 = 255;
            break;
          default:
            // if nothing else matches, do the default
            // default is optional
            red_value1 = 0;
            green_value1 = 0;
            blue_value1 = 0;
          break;
        }  // fin switch

        for(int i=0;i<NUMPIXELS;i++)
          { 
          pixels.setPixelColor(i, pixels.Color(red_value1,green_value1,blue_value1)); // Moderately bright green color.    
          pixels.show(); // This sends the updated pixel color to the hardware.   
          delay(25); // Delay for a period of time (in milliseconds).
          } // fin for
          
  }// fin if slide color

/*
   _____         .__                __  .__                          _____  
  /  _  \   ____ |__| _____ _____ _/  |_|__| ____   ____   ______   /  |  | 
 /  /_\  \ /    \|  |/     \\__  \\   __\  |/  _ \ /    \ /  ___/  /   |  |_
/    |    \   |  \  |  Y Y  \/ __ \|  | |  (  <_> )   |  \\___ \  /    ^   /
\____|__  /___|  /__|__|_|  (____  /__| |__|\____/|___|  /____  > \____   | 
        \/     \/         \/     \/                    \/     \/       |__| 
 */
  //animation aleatoire 2 leds blanches
  if (green_value==262)
  {
    if(init_info==1)
    {
      init_info=0;
      Serial.println("allumage aleatoire 2 led blanche");   
      all_led_off();
    }  
    varr=random(4);
    varg=random(30);
    varb=random(30);
    int_alea=random(NUMPIXELS-6);
    int_random_val=random(3);

    switch (int_random_val) 
      {
          case 1:
            //white
            pixels.setPixelColor(int_alea  , pixels.Color(255,255,255));
            pixels.setPixelColor(int_alea+1, pixels.Color(255,255,255));
            pixels.setPixelColor(int_alea+2, pixels.Color(255,255,255));
            pixels.setPixelColor(int_alea+3, pixels.Color(255,255,255));
            pixels.setPixelColor(int_alea+4, pixels.Color(255,255,255));
            pixels.setPixelColor(int_alea+5, pixels.Color(255,255,255));
            pixels.setPixelColor(int_alea+6, pixels.Color(255,255,255));
            break;
          case 2:
            //white
            pixels.setPixelColor(int_alea  , pixels.Color(255,0,0));
            pixels.setPixelColor(int_alea+1, pixels.Color(255,0,0));
            pixels.setPixelColor(int_alea+2, pixels.Color(255,0,0));
            pixels.setPixelColor(int_alea+3, pixels.Color(255,0,0));
            pixels.setPixelColor(int_alea+4, pixels.Color(255,0,0));
            pixels.setPixelColor(int_alea+5, pixels.Color(255,0,0));
            pixels.setPixelColor(int_alea+6, pixels.Color(255,0,0));
            break;
          case 3:
            //white
            pixels.setPixelColor(int_alea  , pixels.Color(0,255,255));
            pixels.setPixelColor(int_alea+1, pixels.Color(0,255,255));
            pixels.setPixelColor(int_alea+2, pixels.Color(0,255,255));
            pixels.setPixelColor(int_alea+3, pixels.Color(0,255,255));
            pixels.setPixelColor(int_alea+4, pixels.Color(0,255,255));
            pixels.setPixelColor(int_alea+5, pixels.Color(0,255,255));
            pixels.setPixelColor(int_alea+6, pixels.Color(0,255,255));
            break;
          default:
            // if nothing else matches, do the default
            //white
            pixels.setPixelColor(int_alea  , pixels.Color(0,0,255));
            pixels.setPixelColor(int_alea+1, pixels.Color(0,0,255));
            pixels.setPixelColor(int_alea+2, pixels.Color(0,0,255));
            pixels.setPixelColor(int_alea+3, pixels.Color(0,0,255));
            pixels.setPixelColor(int_alea+4, pixels.Color(0,0,255));
            pixels.setPixelColor(int_alea+5, pixels.Color(0,0,255));
            pixels.setPixelColor(int_alea+6, pixels.Color(0,0,255));
          break;
        }  // fin switch

                   
    pixels.setPixelColor(int_indice, pixels.Color(0,0,0));
    pixels.show(); // This sends the updated pixel color to the hardware.

    delay(5); // Delay for a period of time (in milliseconds).
    all_led_off();
    
  }  
/*
   _____         .__                __  .__                       _________ 
  /  _  \   ____ |__| _____ _____ _/  |_|__| ____   ____   ______ \______  \
 /  /_\  \ /    \|  |/     \\__  \\   __\  |/  _ \ /    \ /  ___/     /    /
/    |    \   |  \  |  Y Y  \/ __ \|  | |  (  <_> )   |  \\___ \     /    / 
\____|__  /___|  /__|__|_|  (____  /__| |__|\____/|___|  /____  >   /____/  
        \/     \/         \/     \/                    \/     \/            
 */
  //animation bougie
  // note : the animation was not active because very slow because 
  
  if (green_value==263)
  { 
    if(init_info==1)
    {
      init_info=0;
      Serial.println("Animation bougie");   
      all_led_off();
    }
  
    burn(10);
    flicker(5);
    burn(8);
    flutter(6);
    burn(3);
    on(10);
    burn(10);
    flicker(10);  
  }
  
}  //fin fonction special_animation

/*
_________                    .___.__             _____                    __  .__               
\_   ___ \_____    ____    __| _/|  |   ____   _/ ____\_ __  ____   _____/  |_|__| ____   ____  
/    \  \/\__  \  /    \  / __ | |  | _/ __ \  \   __\  |  \/    \_/ ___\   __\  |/  _ \ /    \ 
\     \____/ __ \|   |  \/ /_/ | |  |_\  ___/   |  | |  |  /   |  \  \___|  | |  (  <_> )   |  \
 \______  (____  /___|  /\____ | |____/\___  >  |__| |____/|___|  /\___  >__| |__|\____/|___|  /
        \/     \/     \/      \/           \/                   \/     \/                    \/ 
 */
//************************************************************************************************
//fonction animation bougie
// In loop, call CANDLE STATES, with duration in seconds
// 1. on() = solid yellow
// 2. burn() = candle is burning normally, flickering slightly
// 3. flicker() = candle flickers noticably
// 4. flutter() = the candle needs air!



// basic fire funciton - not called in main loop
void fire(int grnLow) {
  for (int grnPx = grnHigh; grnPx > grnLow; grnPx--) {
    pixels.setPixelColor(0,pixels.Color(redPx,grnPx,bluePx));
    pixels.show();
    delay(fDelay);
  }  
  for (int grnPx = grnLow; grnPx < grnHigh; grnPx++) {
    pixels.setPixelColor(0,pixels.Color(redPx,grnPx,bluePx));
    pixels.show();
    delay(fDelay);
  }
}

// fire animation
void on(int f) {
  fRep = f * 1000;
  int grnPx = grnHigh - 5;
  pixels.setPixelColor(0,pixels.Color(redPx,grnPx,bluePx));
  pixels.show();
  delay(fRep);
}

void burn(int f) {
  fRep = f * 8;
  fDelay = burnDelay;
  for (int var = 0; var < fRep; var++) {
    fire(burnLow);
  }  
}

void flicker(int f) {
  fRep = f * 8;
  fDelay = burnDelay;
  fire(burnLow);
  fDelay = flickDelay;
  for (int var = 0; var < fRep; var++) {
    fire(flickLow);
  }
  fDelay = burnDelay;
  fire(burnLow);
  fire(burnLow);
  fire(burnLow);
}

void flutter(int f) {
  fRep = f * 8;  
  fDelay = burnDelay;
  fire(burnLow);
  fDelay = flickDelay;
  fire(flickLow);
  fDelay = flutDelay;
  for (int var = 0; var < fRep; var++) {
    fire(flutLow);
  }
  fDelay = flickDelay;
  fire(flickLow);
  fire(flickLow);
  fDelay = burnDelay;
  fire(burnLow);
  fire(burnLow);
}

//fin fonctions bougie
//************************************************************************************************

void all_led_off(void)
{
  for (int_i=0;int_i<NUMPIXELS;int_i++)
  {
    pixels.setPixelColor(int_i,pixels.Color(0,0,0)); 
  }
  pixels.show();    
}

/*
_________ .____    ________  _________  ____  __.   _____                    __  .__                       
\_   ___ \|    |   \_____  \ \_   ___ \|    |/ _| _/ ____\_ __  ____   _____/  |_|__| ____   ____   ______ 
/    \  \/|    |    /   |   \/    \  \/|      <   \   __\  |  \/    \_/ ___\   __\  |/  _ \ /    \ /  ___/ 
\     \___|    |___/    |    \     \___|    |  \   |  | |  |  /   |  \  \___|  | |  (  <_> )   |  \\___ \  
 \______  /_______ \_______  /\______  /____|__ \  |__| |____/|___|  /\___  >__| |__|\____/|___|  /____  > 
        \/        \/       \/        \/        \/                  \/     \/                    \/     \/  
 */
//fonction NTP
void requete_time(void) {
  sendNTPpacket(timeServer); // send an NTP packet to a time server
  // wait to see if a reply is available
  delay(1000);
  if (Udp.parsePacket()) {
    //Serial.println("packet received");
    // We've received a packet, read the data from it
    Udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    //Serial.print("Seconds since Jan 1 1900 = ");
    //Serial.println(secsSince1900);

    // now convert NTP time into everyday time:
    //Serial.print("Unix time = ");
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;
    // print Unix time:
    //Serial.println(epoch);


    // print the hour, minute and second:
    //Serial.print("The UTC time is ");       // UTC is the time at Greenwich Meridian (GMT)
    global_hour=((((epoch  % 86400L) / 3600)+2));
    Serial.print(" - ");   
    Serial.print((((epoch  % 86400L) / 3600)+2)); // print the hour (86400 equals secs per day)
    Serial.print(':');
    if (((epoch % 3600) / 60) < 10) {
      // In the first 10 minutes of each hour, we'll want a leading '0'
      Serial.print('0');
    }
    global_min=((epoch  % 3600) / 60);
    Serial.print((epoch  % 3600) / 60); // print the minute (3600 equals secs per minute)
    Serial.print(':');
    if ((epoch % 60) < 10) {
      // In the first 10 seconds of each minute, we'll want a leading '0'
      Serial.print('0');
    }
    global_sec=(epoch % 60);
    Serial.println(epoch % 60); // print the second
  }

  // wait ten seconds before asking for the time again
  //delay(10000);
}


// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address) {
  //Serial.println("1");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  //Serial.println("2");
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  //Serial.println("3");

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  //Serial.println("4");
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  //Serial.println("5");
  Udp.endPacket();
  //Serial.println("6");
}


/*
.__             .___           __          __                
|  |   ____   __| _/   _______/  |______ _/  |_ __ __  ______
|  | _/ __ \ / __ |   /  ___/\   __\__  \\   __\  |  \/  ___/
|  |_\  ___// /_/ |   \___ \  |  |  / __ \|  | |  |  /\___ \ 
|____/\___  >____ |  /____  > |__| (____  /__| |____//____  >
          \/     \/       \/            \/                \/ 
 */
//fonction led status
//Use the first led of the neopixel for indicate the status connexion if wifi or mqtt failed
void green_status(void)
{
  //The first led is green if wifi and MQTT OK
  pixels.setPixelColor(0, pixels.Color(0,20,0));  // Moderately bright green color.       
  pixels.show();                                  // This sends the updated pixel color to the hardware.   
  delay(delayval);                                // Delay for a period of time (in milliseconds).
}

void blue_status(void)
{
  //The first led is blue if wifi is ok but mqtt failed
  pixels.setPixelColor(0, pixels.Color(0,0,20));  // Moderately bright green color.       
  pixels.show();                                  // This sends the updated pixel color to the hardware.   
  delay(delayval);                                // Delay for a period of time (in milliseconds).
}

void red_status(void)
{
  //The first led is red if wifi ko and the mqtt don't work because no network connexion
  pixels.setPixelColor(0, pixels.Color(20,0,0));  // Moderately bright green color.      
  pixels.show();                                  // This sends the updated pixel color to the hardware.   
  delay(delayval);                                // Delay for a period of time (in milliseconds).
}
