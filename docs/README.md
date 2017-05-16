# WiFiSwitch

## Setup page

### Network settings

* DHCP on/off

(readonly filled with current IP if DHCP is on)

* IP Address 

* IP Mask

* Default gateway

* DNS server

(read/write)

* NTP server

* TimeZone

Save/Cancel/Reset to defaults buttons

### Hardware reset settings

D8 - Long (3 sec) PullUp for reset Network Settings to Defaults and switch to AP mode. 

### Pinout map
PIN | GPIO | HW Function | Project Function
----|------|----------|---------
D0  | 16 | | S0
D1  | 5  | Default I2C SDL | S1
D2  | 4  | Default I2C SDA | SDA
D3  | 0  | | SDL
D4  | 2  | ESP LED | S2
D5  | 14 | SPI CLK | S3
D6  | 12 | SPI MISO | S4
D7  | 13 | SPI MOSI / alt RX | S5
D8  | 15 | SPI SS / alt TX | RESET
D9  | 3  | RX / I2S | S6
D10 | 1  | TX | S7
D11 | 9  | | N/A
D12 | 10 | | N/A
