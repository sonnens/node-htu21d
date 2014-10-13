# htu21d

node interface for the HTU21D temperature and humidity sensor for Raspberry Pi

## Install
````bash
$ npm install htu21d
````

## Usage

```javascript
var htu21d = require('htu21d');

var device = "/dev/i2c-1"
var address = 0x40;

var sensor = new htu21d.Htu21d(device, address);

var temp = sensor.temperature();

var humidity = sensor.humidity();

````
the htu21d object contains fields for sensor resolution, lowbattery status, and
on-chip heater, as per the chip's capabilities, as well as devnode and chip
address

```javascript
> sensor

{ heater: false,
  lowbattery: false,
  resolution: { temperature: 14, humidity: 12 },
  devnode: '/dev/i2c-1',
  address: 64 }
````

## Raspberry Pi Setup


````bash
$ sudo vi /etc/modules
````

Add these two lines

````bash
i2c-bcm2708 
i2c-dev
````

````bash
$ sudo vi /etc/modprobe.d/raspi-blacklist.conf
````

Comment out blacklist i2c-bcm2708

````
#blacklist i2c-bcm2708
````

Load kernel module

````bash
$ sudo modprobe i2c-bcm2708

````

## Questions?

http://www.twitter.com/johnnysunshine

