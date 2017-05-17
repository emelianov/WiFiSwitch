# WiFiSwitch

## Configuration files

### Network settings

Parameter | Description | Default
----------|-------------|---------
ip        | IP address of controller | 192.168.1.111
mask      | Network mask | 255.255.255.0
gw        | Default gateway | 192.168.1.1
dns       | IP address of DNS | 192.168.1.1
ssid      | Access Point SSID | empty
ssidpass  | Access Point password | empty
ntps      | Name of NTP server | pool1.ntp.org
timezone  | Local time zone offset | 0

### *Example:*
```xml
<config>
<name>ehcontrol Master</name>
<ip>192.168.0.88</ip>
<mask>255.255.255.0</mask>
<gw>192.168.0.1</gw>
<dns>192.168.0.1</dns>
<ssid>SSID</ssid>
<ssidpass>PASSWORD</ssidpass>
<ntps>192.168.0.1</ntps>
<ntps>pool1.ntp.org</ntps>
<ntps>pool2.ntp.org</ntps>
<timezone>-4</timezone>
</config>
```