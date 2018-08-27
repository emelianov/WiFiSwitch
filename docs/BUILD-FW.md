# WiFiSwitch

## How to build firware update file

### General information

WiFiSocket firmware update package is .tar archive. Archive can contain firmware image `firmware.bin` and other files that will be seamply extracted to local ESP filesystem.
If file with same name is exists it will be overwritten. Update package can contain firmware image or simple files along of each other.

### Update package creation under macOS

1. Create firmware binary file by selecting Sketch - Export compiled Binary

2. Rename exported binary to firmware.bin

3. Place files to be put to package to and exported binary to the same directory

4. `gzip -9 index.html`
 `tar -cf update.tar firmware.bin index.html.gz`

### Update package creation under Windows

0. Install [Z-Zip](https://7-zip.org)

1. Create firmware binary file by selecting Sketch - Export compiled Binary

2. Rename exported binary to firmware.bin

3. Place files to be put to package to and exported binary to the same directory

4. `"C:\Program Files\7-Zip\7z.exe" a -tgzip index.html.gz index.html`

   `"C:\Program Files\7-Zip\7z.exe" a -ttar fw.tar firmware.bin index.html.gz`