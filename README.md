# vat1300
Arduino nRF24L01+ data logger for Juntek VAT-1300 wireless battery monitor

Tested on Arduino ESP8266 (LOLIN(WEMOS) D1 R2 & mini)


//Change the following lines to match your setup:

const char* ssid = "change this to your Wifi access point SSID";

const char* password = "change this to your Wifi access point password";


//Change the following to match the address and frequency as set on the Juntek controller

const int address = 17;

const char frequency = 'A';


Watch the following YouTube videos for more information

Part 1: https://www.youtube.com/watch?v=DNORE1HBbRo

Part 2: https://www.youtube.com/watch?v=bQKVNZRaFuQ&t=1s

Part 3: https://www.youtube.com/watch?v=S0-O9cTS9Uw

Part 4: https://youtu.be/nuUWXwzuHhk


Part 4: Storing and displaying the data: Adding a Juntek VAT-1300 Wireless DC Ammeter, Watt-hour Meter, Coulomb counter to your network using an Arduino ESP8266 D1 Mini and a nRF24L01+ radio module.

Part 4 shows how to log data over Wifi via TCP and then store and display the data on a Raspberry Pi using the Influxdb (database), Node-red (to grab and store the data), and Grafana (to host web access to graphically display the data).

Arduino ESP8266 D1 Mini, Source code is located at: https://github.com/rjflatley/vat1300


Basic setup steps on the Raspberry Pi (Raspbian Buster):

  $sudo apt update

  $sudo apt full-upgrade

  $curl -sL https://repos.influxdata.com/influxdb... | sudo apt-key add -

  $echo "deb https://repos.influxdata.com/debian buster stable" | sudo tee /etc/apt/sources.list.d/influxdb.list

  $sudo apt update

  $sudo apt-get install influxdb

  $sudo systemctl enable --now influxdb

  $influx

     -CREATE USER root WITH PASSWORD 'root' WITH ALL PRIVILEGES

     -CREATE DATABASE "battsoc"

     -exit

  $bash <(curl -sL https://raw.githubusercontent.com/node-red/linux-installers/master/deb/update-nodejs-and-nodered)

  $node-red-start

  ctrl-c

  $sudo systemctl enable nodered.service

From remote computer use browser to connect to Node-RED at the ip_address_of_the_rpi:1880

Import the following.  Change the IP address of TCP input to match the address of your raspberry pi

[{"id":"3484cf5b.784ca","type":"tab","label":"Flow 1","disabled":false,"info":""},{"id":"59bc08ea.500ef8","type":"tcp in","z":"3484cf5b.784ca","name":"","server":"client","host":"192.168.1.32","port":"23","datamode":"stream","datatype":"utf8","newline":"","topic":"","base64":false,"x":140,"y":260,"wires":[["fac80c1d.a2f52","863d0475.cfb1a8"]]},{"id":"fac80c1d.a2f52","type":"debug","z":"3484cf5b.784ca","name":"","active":true,"tosidebar":true,"console":false,"tostatus":false,"complete":"false","x":420,"y":280,"wires":[]},{"id":"863d0475.cfb1a8","type":"http request","z":"3484cf5b.784ca","name":"","method":"POST","ret":"txt","paytoqs":false,"url":"http://localhost:8086/write?db=battsoc","tls":"","persist":false,"proxy":"","authType":"basic","x":640,"y":580,"wires":[["e06f6cba.0c32"]]},{"id":"e06f6cba.0c32","type":"debug","z":"3484cf5b.784ca","name":"","active":true,"tosidebar":true,"console":false,"tostatus":false,"complete":"payload","targetType":"msg","x":890,"y":580,"wires":[]}]

Make sure you click "Deploy"

Return to SSH of the Raspberry Pi

   $influx

     -use battsoc

     -show measurements

     -show field keys

     -select * from SOC

     -exit

  $wget https://dl.grafana.com/oss/release/gr...

  $sudo dpkg -i grafana_6.7.2_armhf.deb

  $sudo /bin/systemctl daemon-reload

  $sudo /bin/systemctl enable grafana-server

  $sudo /bin/systemctl start grafana-server

  $systemctl status grafana-server

From remote computer use browser to connect to Grafana at ip_address_of_rpi:3000

Log in user:admin password:admin

set up database and dashboard

See Youtube video for setup example

Don't forget to safe the dashboard
