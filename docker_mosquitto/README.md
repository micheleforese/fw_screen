# Example Mosquitto Docker Server

1. Navigate to this folder
2. Type:

    ```shell
    docker run -it -p 1883:1883 -v "./mosquitto.conf:/mosquitto/config/mosquitto.conf" eclipse-mosquitto:2.0.22
    ```
