<?php
/*
PHP file to send text receive from IFTTT and transmit it to NODE RED websocket
Author : Sully
Date : 17/01/2020
Version : E0C0R0
*/

// VARIABLES
$ip_nodered='192.168.0.17';
$port_websocket=1222;

$text_pattern  = $_GET['state'];
echo "Page loaded <br>";

if (empty($text_pattern)) {
	
	//Text frame empty
	$text_pattern="";
	echo "Received frame empty.";
	
}
else{
	
	//Text frame not empty
	echo "Received frame :  $text_pattern ";
	
	//create socket to send text to NODE RED 
	//we must fill the IP adress of Node Red server and the socket port mumber 
	$socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
	socket_connect($socket, $ip_nodered, $port_websocket);


	$st=$text_pattern;
	$length = strlen($st);
		   
		while (true) {
		   
			$sent = socket_write($socket, $st, $length);
			   
			if ($sent === false) {
		   
				break;
			}
			   
			// Check if the entire message has been sented
			if ($sent < $length) {
				   
				// If not sent the entire message.
				// Get the part of the message that has not yet been sented as message
				$st = substr($st, $sent);
				   
				// Get the length of the not sented part
				$length -= $sent;

			} else {
			   
				break;
			}          
		}

	//close the socket after sending the text frame
	socket_close($socket);
	
} //else text frame not empty
                   
?>
