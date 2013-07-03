DO-IT-YOURSELF TURBIDIMETER
===========================
===========================

This setup will allow you to interact with a command-prompt turbidimeter over a serial connection.

Required Hardware
=================

1 Arduino Uno microcontroller (or equivalent)
2 TLS2561 luminosity sensor [http://www.adafruit.com/products/439]
1 Electronics case [http://www.alliedelec.com/search/productdetail.aspx?SKU=70149027]
1 Bluetooth slave module [http://www.amazon.com/BT2S-Bluetooth-to-Serial-Slave/dp/B006RBK9ZW?]
1 5mm precision red LED [https://www.sparkfun.com/products/11393]
  Jumper cables
  Soldering iron

Required Software
==================
Arduino IDE

Additional information for Bluetooth
==================

 Any device that can access a serial tty will work; we recommend using an Android phone with the Blueterm app (a tablet works with all features except data transmittal over SMS).

Other
===========

 To calibrate your turbidimeter to output NTUs (nephelometric turbidity standard units), you will either need to benchmark against a commercial model or purchase calibration materials from a vendor such as Procal. If that's not  an option, you'll have to make do with an arbitrary unit scale.

----The Open Turbidimeter Project is a develop4wash initiative. Vist wash4all.org for more information. ------
