# WiFiSwitch

### Hardware reset settings

D8 - Long (3 sec) PullUp for reset Network Settings to Defaults and switch to AP mode. 

### Pinout map
PIN | GPIO | Default Function | Project Function
----|------|----------|---------
D0  | 16 | | S0
D1  | 5  | I2C SDL | S1
D2  | 4  | I2C SDA | SDA
D3  | 0  | | SCL
D4  | 2  | ESP LED | S2
D5  | 14 | SPI CLK | S3
D6  | 12 | SPI MISO | S4
D7  | 13 | SPI MOSI / alt RX | S5
D8  | 15 | SPI SS / alt TX | RESET
D9  | 3  | RX / I2S | S6
D10 | 1  | TX | S7
D11 | 9  | | N/A
D12 | 10 | | N/A
