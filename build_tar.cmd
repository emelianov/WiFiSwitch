cd data
erase -y firmware.bin
erase -y index.html.gz
erase -y fw.tar
copy /y ..\WiFiSwitch.ino.d1_mini.bin .
ren WiFiSwitch.ino.d1_mini.bin firmware.bin
"C:\Program Files\7-Zip\7z.exe" a -tgzip index.html.gz index.html
"C:\Program Files\7-Zip\7z.exe" a -ttar fw.tar firmware.bin index.html.gz