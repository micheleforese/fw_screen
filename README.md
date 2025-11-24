# Anemometer Screen APP

## Instructions

1. Clone the repository

    ```shell
    git clone https://github.com/micheleforese/fw_screen.git
    cd fw_screen
    ```

2. Open a new Terminal session:

    ```shell
    source $HOME/esp/v5.5.1/esp-idf/export.sh
    idf.py make flash
    ```

3. Press the Reset Button
4. Open another Session.
5. check for the 2 ports:

    ```shell
    ls /dev/ttyACM* -la
    ```

    example:

    ```shell
    crw-rw----@ 166,0 root 24 Nov 01:46 /dev/ttyACM0
    crw-rw----@ 166,1 root 24 Nov 01:48 /dev/ttyACM1
    ```

6. Try to send example JSON

    > Wind data

    ```shell
    echo '{"topic": "anm","x":0.0124,"y":-0.0421,"z":-0.0041,"timestamp":1763924107.7981,"x_kalman":-0.0054,"autocalibrazione_asse_x":true,"autocalibrazione_misura_x":false,"temp_sonica_x":20.2112,"y_kalman":-0.004,"autocalibrazione_asse_y":true,"autocalibrazione_misura_y":false,"temp_sonica_y":20.1449,"z_kalman":0.0,"autocalibrazione_asse_z":true,"autocalibrazione_misura_z":false,"temp_sonica_z":20.2773}' > /dev/ttyACM1
    ```

    > Particulate Matter Data

    ```shell
    echo '{"topic":"sps","timestamp":1762784698,"sensor_data":{"mass_density":{"pm1.0":7.512,"pm2.5":7.944,"pm4.0":7.944,"pm10":7.944},"particle_count":{"pm0.5":51.835,"pm1.0":59.757,"pm2.5":59.954,"pm4.0":59.968,"pm10":59.98},"particle_size":0.443,"mass_density_unit":"ug/m3","particle_count_unit":"#/cm3","particle_size_unit":"um"}}' > /dev/ttyACM1
    ```

## Flash The .bin file

1. Follow the instructions from [this](https://docs.espressif.com/projects/esptool/en/latest/esp32/installation.html) site.

```shell
sudo apt install python pipx
sudo pipx unsurepath

pipx install esptool
esptool --version
```

```shell
esptool --chip esp32s3 \
  --port /dev/ttyACM0 -b 460800 \
  --before default-reset \
  --after hard-reset \
  write-flash \
  --flash-mode dio \
  --flash-size 2MB \
  --flash-freq 80m \
  0x0 ./build/bootloader/bootloader.bin \
  0x8000 ./build/partition_table/partition-table.bin \
  0x10000 ./build/fw_screen.bin
```

## Connect the esp-idf tools

```shell
source $HOME/esp/v5.5.1/esp-idf/export.sh
```

## JSON Protocol (FOX -> Screen APP)

1. Wind Velocity

   ```json
   {
        "topic": "anm",

        "timestamp": 1750958699.7325091,

        "x_kalman": 0.091875345469413672,
        "x_axe_autocalibration": true,
        "x_measure_autocalibration": false,
        "x_sonic_temp": 21.218753282865862,

        "y_kalman": 0.0583878954295078,
        "y_axe_autocalibration": true,
        "y_measure_autocalibration": false,
        "y_sonic_temp": 21.28940703494186,

        "z_kalman": -0.033768986510647636,
        "z_axe_autocalibration": true,
        "z_measure_autocalibration": false,
        "z_sonic_temp": 21.285374117923027
   }
   ```

2. Particulate Matter SPS30

   ```json
   {
    "topic": "sps",
     "timestamp": 1762784698,
     "sensor_data": {
       "mass_density": {
         "pm1.0": 7.512,
         "pm2.5": 7.944,
         "pm4.0": 7.944,
         "pm10": 7.944
       },
       "particle_count": {
         "pm0.5": 51.835,
         "pm1.0": 59.757,
         "pm2.5": 59.954,
         "pm4.0": 59.968,
         "pm10": 59.98
       },
       "particle_size": 0.443,
       "mass_density_unit": "ug/m3",
       "particle_count_unit": "#/cm3",
       "particle_size_unit": "um"
     }
   }
   ```

3. Commands

   1. Power Off

   ```json
   {
     "type": "command",
     "command": "POWER OFF"
   }
   ```

   1. Measure START

   ```json
   {
     "type": "command",
     "command": "MEASURE START"
   }
   ```

   1. Measure STOP

   ```json
   {
     "type": "command",
     "command": "MEASURE STOP"
   }
   ```

## sdkconfig

1. `sdkconfig.defaults`

    ```config
    CONFIG_TINYUSB_CDC_ENABLED=y
    CONFIG_TINYUSB_CDC_COUNT=2
    ```
