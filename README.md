# ifttt_php_nodered
Domotic voice control

This example is to have information for voice control his DIY domotic system.

The aim is to use "OK Google" and tell "domotic light on" and then you can switch IO on the rapsberrypi interface.

The evolution of this tutorial is to use MQTT to send command to an arduino board for controlling Ligthing. 

## TO DO

 * Add arduino file example

 
# PREREQUISITES
 
Before using this example, it's necessary to have these differents elements :

1. A raspberrypi with raspbian OS and these software (node red, apache with php support, MQTT mosquitto broker)
```
 * sudo apt-get install mosquitto
 * sudo apt-get install apache2
 * sudo apt-get install php
 * sudo apt-get install nodered
```

2. Some external diy domotic system (for example light neopixel control by MQTT using ESP32 or ESP8266 module)
 - I will put and arduino example (neopixel control)

3. A free account open on IFTTT (https://ifttt.com/)

4. A valid google account for generate the voice command with "OK google..."

# IFTTT configuration

After connected to your IFTTT account, create a new applet 

![Create new applet and click on the first +](images/ifttt_p1.JPG)

Search the service "Google asssitant"

![Search and click the google assistance channel (step 1/6)](images/ifttt_p2.JPG)

Choose the "Say phrase with the text ingredient"

![Choose the "Say phrase with the text ingredient" (step 2/6)](images/ifttt_p4.JPG)

Set the passphrase key before the $ character. After tell OK Google, tell the passphrasekey, google assistant transmit the text after to parameter 

Fill the filed "What do you want the -assistant to say in response" to have the call back of your voice command.

![Fill the step 2 of 6 ](images/ifttt_p5.JPG)

After click of the second + after the "then"
 
![Click on the second + character (step 3/6)](images/ifttt_p6.JPG)

Choose the action Webhook (Make a web request)
 
![Click on the second + charecter (step 4/6)](images/ifttt_p7.JPG)

Complete the action with the php link of the webserver on the rapsberrypi and add the "TextField" for transmit the google text to the php file.

![Complete action fields (step 5/6)](images/ifttt_p8.JPG)

Select Content type with "text/plain".

![Complete action fields (step 5/6)](images/ifttt_p9.JPG)

Select FINISH to validate the applet (Google assistant -> webhook).

![Click finish to terminate the applet (step 6/6)](images/ifttt_p10.JPG)

Now the applet is ready to transmit google voice text to the php file.

If you use your phone with google voice and you tell "Ok google domotic light on". Then your phone return "light on".

![The applet is now ready](images/ifttt_p11.JPG)

# NODE RED script

After running nodered on the raspberrypi, open the link for adding the script to capture the google voice text.

The NodeRed interface is at this link : http://raspberrypi_adress:1880

![Node red interface with the websocket script](images/nodered_p1.jpg)

Open the script test with notepad and add the script by copy paste the text to import/clipboard.

![Access for import clipboard](images/nodered_p2.jpg)

![Paste the text in the import node](images/nodered_p3.jpg)

# PHP file

Add the php file "ifttt_websocket_nodered_gateway.php" on the raspberrypi webserver.

The file will be copied in the /var/www/html/ifttt_websocket_nodered_gateway.php


