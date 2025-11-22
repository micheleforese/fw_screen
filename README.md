# Anemometer Screen APP

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
