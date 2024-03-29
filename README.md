# KitchenThatSaysBEEP
Like a lot of daddy's my girl has an DUKTIG IKEA kitchen.
We gave it already nicer colours but we still missed some features.
Till I came along the following link made by [roaldh]; 
https://www.instructables.com/id/Kids-Kitchen-That-Says-BEEP/

I felt in love with the concept, and love the beep.
I found also [Myles Eftos] that did the same with the microwave above:
https://www.hackster.io/madpilot/a-microwave-interface-for-the-ikea-duktig-kids-kitchen-804740

I took best Ideas of both and made a remix.


## Bill of material

For less than 10€ it's possible to make a nicer kitchen;

* Arduino pro mini or compatible: €1,5  
* TM1637 - 4 digit display: €0,6 - https://www.aliexpress.com/item/1969258031.html 
* KY-40 360 Degrees Rotary Encoder Button: €0,50 - https://www.aliexpress.com/item/32224563961.html
* DS3231 RTC: € 0,90 - https://www.aliexpress.com/item/32533518502.html
* Buzzer: €0,5 - https://www.aliexpress.com/item/32787576109.html
* 16 LED Neopixel Ring: €2,1 - https://www.aliexpress.com/item/32889368254.html  
* old broken usb cable
* old usb charger

Special:
* 3D printed, Lasercut or selfmade box for everything 

## Pin Connection

DS3231 --> Arduino (I2C):
* Gnd + 5V --> Gnd + 5V
* SDA --> A4 
* SCL --> A5 

Buzzer --> Arduino:
* Gnd --> Gnd 
* + --> Pin 4

TM1637 --> Arduino:
* Gnd + 5V --> Gnd + 5V
* TM1637CLK --> Pin 3
* TM1637Data --> Pin 2

16 LED Neopixel Ring --> Arduino
* Gnd + 5V --> Gnd + 5V
* DI --> Pin 6

Rotary Encoder
* Gnd + 5V --> Gnd + 5V
* ENCODER_BUTTON (SW) --> Pin 10
* ENCODER_A (DT) --> Pin 11
* ENCODER_B (CLK) --> Pin 12




## Code:

I used following libraries:
* Adafruit_NeoPixel 
* ClickEncoder: https://github.com/0xPIT/encoder
* TimerOne (needed for ClickEncoder)
* DS1307RTC (for the realtime clock)
* Wire (needed for DS1207RTC )
* TM1637Display

