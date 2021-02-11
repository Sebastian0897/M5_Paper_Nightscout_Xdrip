# M5-Paper-Nightscout-Xdrip
Using an M5-Paper to show various things. From the Current time, to weather but also the current Blood Glucose, via Nightscout and in Future over Bluetooth from Xdrip Directly.
And all of that as Energy Saving as Possible. You dont need to use the Blood Glucose Display if you dont have any use for it.
I am  also planing  implementing Features like Smart Home Control. But it is currently not a Priority. 

# How it works
The Programm starts every minute to update the clock and update other information. That is all that is needed as the M5 Paper has an E-ink Display that retains the information without using power.
What to update when is currently hard coded, but I am Planing to implement a config file to make that easier to configure.
After all the Updates the devices calculates how long to shut off (and set a timer via the RTC) to reboot in time for the Clock to switch to the next minute.

# Using the Code
For the Code to Work Correctly the RTC time needs to be set before installing my Code. Also you need to fill in your wifi Credentials, Openweathermap APi key (if you want the Temperatur Display), Nightscout server adress(if you want the Blood Glucose Display). 
You can also set alternative Wifi Credetials further down the Code in ConnectWifi(). At the bottom of the code is the main loop were you can set what to run when. I am planing on Improving how that works to make it configurable in a Config file.


# Currently working:
- Time and Date               
- On/Off Notification         
- WIFI Indicator              
- Battery Indicator           
- Rudimentry Energy saving    
- Temperatur Display for a given location            
- Display for Blood Glucose over Nightscout 
- Multiple Wifi Credentials Possible


# TODO:

GUI:
- Better Arrows to Indicate Blood Glucose Direction
- Better weather Display
- Graphing the Blood Glucose
- maybe a Different Font

Features:
- Working on the Energy saving -> dont update Blood glucose every minute as it requires a Wifi connection 
- Config file for the parameters that need to be set
- Change the main loop to make i configurable wit a config file
- Multiple Languages (currently just German GUI)
- Find something usefull to do with the half empty Display
- Get Blood Glucose Directly per Bluetooth from Xdrip
- Maybe also Phone Notifications per Bluetooth
- Alarms for Critical Blood Glucose (maybe also Predictive Alarms)
- implement Location Finding via wifi networks 
- Make use of the Buttons and Touch Display 
- You tell me
 
 Ideas:
 - Maybe implement different modes like energy saving and night mode, or more often Updates
 - Maybe implement some Smart Home controlls (Philips hue/Hubitat/AVM)

Bugs:
- You tell me


# Credits
I am using code Snippets from:
- https://github.com/m5stack/M5EPD
- https://randomnerdtutorials.com/esp32-http-get-open-weather-map-thingspeak-arduino/


An got some Ideas and Inspiration From:
- https://github.com/enny1170/BtDisplay
- https://github.com/mlukasek/M5_NightscoutMon
- https://github.com/nightscout/cgm-remote-monitor
- https://github.com/Fabian-Schmidt/PlaygroundArduino/tree/master/ESP32/xDrip-LeFun



