use clap::Parser;
use serde::{Deserialize, Serialize};
use std::io::{BufRead, BufReader, Write};
use std::sync::{Arc, Mutex};
use std::time::{Duration, SystemTime, UNIX_EPOCH};
use std::{io, thread};

#[derive(Parser, Debug)]
#[command(author, version, about, long_about = None)]
struct Args {
    /// Serial port path (e.g., /dev/ttyUSB0)
    #[arg(short, long)]
    port: String,

    /// Time between messages in milliseconds
    #[arg(short, long, default_value_t = 3000)]
    time: u64,
}

#[derive(Serialize, Deserialize, Debug)]
struct AnmMessage {
    topic: String,
    timestamp: f64,
    x_kalman: f64,
    x_axe_autocalibration: bool,
    x_measure_autocalibration: bool,
    x_sonic_temp: f64,
    y_kalman: f64,
    y_axe_autocalibration: bool,
    y_measure_autocalibration: bool,
    y_sonic_temp: f64,
    z_kalman: f64,
    z_axe_autocalibration: bool,
    z_measure_autocalibration: bool,
    z_sonic_temp: f64,
}

#[derive(Serialize, Deserialize, Debug)]
struct MassDensity {
    #[serde(rename = "pm1.0")]
    pm1_0: f64,
    #[serde(rename = "pm2.5")]
    pm2_5: f64,
    #[serde(rename = "pm4.0")]
    pm4_0: f64,
    pm10: f64,
}

#[derive(Serialize, Deserialize, Debug)]
struct ParticleCount {
    #[serde(rename = "pm0.5")]
    pm0_5: f64,
    #[serde(rename = "pm1.0")]
    pm1_0: f64,
    #[serde(rename = "pm2.5")]
    pm2_5: f64,
    #[serde(rename = "pm4.0")]
    pm4_0: f64,
    pm10: f64,
}

#[derive(Serialize, Deserialize, Debug)]
struct SensorData {
    mass_density: MassDensity,
    particle_count: ParticleCount,
    particle_size: f64,
    mass_density_unit: String,
    particle_count_unit: String,
    particle_size_unit: String,
}

#[derive(Serialize, Deserialize, Debug)]
struct SpsMessage {
    topic: String,
    timestamp: u64,
    sensor_data: SensorData,
}

fn get_timestamp() -> f64 {
    SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_secs_f64()
}

fn create_anm_message() -> AnmMessage {
    AnmMessage {
        topic: "anm".to_string(),
        timestamp: get_timestamp(),
        x_kalman: 0.091875345469413672,
        x_axe_autocalibration: true,
        x_measure_autocalibration: false,
        x_sonic_temp: 21.218753282865862,
        y_kalman: 0.0583878954295078,
        y_axe_autocalibration: true,
        y_measure_autocalibration: false,
        y_sonic_temp: 21.28940703494186,
        z_kalman: -0.033768986510647636,
        z_axe_autocalibration: true,
        z_measure_autocalibration: false,
        z_sonic_temp: 21.285374117923027,
    }
}

fn create_sps_message() -> SpsMessage {
    SpsMessage {
        topic: "sps".to_string(),
        timestamp: SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .unwrap()
            .as_secs(),
        sensor_data: SensorData {
            mass_density: MassDensity {
                pm1_0: 7.512,
                pm2_5: 7.944,
                pm4_0: 7.944,
                pm10: 7.944,
            },
            particle_count: ParticleCount {
                pm0_5: 51.835,
                pm1_0: 59.757,
                pm2_5: 59.954,
                pm4_0: 59.968,
                pm10: 59.98,
            },
            particle_size: 0.443,
            mass_density_unit: "ug/m3".to_string(),
            particle_count_unit: "#/cm3".to_string(),
            particle_size_unit: "um".to_string(),
        },
    }
}

fn main() -> io::Result<()> {
    let args = Args::parse();

    println!("Opening serial port: {}", args.port);
    println!("Send interval: {}ms", args.time);

    // Open serial port
    let port = serialport::new(&args.port, 115200)
        .timeout(Duration::from_millis(100))
        .open()
        .expect("Failed to open serial port");

    let port = Arc::new(Mutex::new(port));

    // Clone for reader thread
    let port_reader = Arc::clone(&port);

    // Spawn reader thread
    let reader_handle = thread::spawn(move || {
        let port = port_reader.lock().unwrap();
        let mut reader = BufReader::new(port.try_clone().unwrap());
        
        loop {
            let mut line = String::new();
            match reader.read_line(&mut line) {
                Ok(0) => continue, // EOF
                Ok(_) => {
                    let trimmed = line.trim();
                    if !trimmed.is_empty() {
                        // Try to parse as JSON and identify message type
                        if let Ok(json) = serde_json::from_str::<serde_json::Value>(trimmed) {
                            if let Some(topic) = json.get("topic").and_then(|t| t.as_str()) {
                                match topic {
                                    "anm" => println!("\n[RECV] ANM message"),
                                    "sps" => println!("\n[RECV] SPS message"),
                                    _ => println!("\n[RECV] Unknown message type: {}", topic),
                                }
                            } else {
                                println!("\n[RECV] Message without topic");
                            }
                            println!("{}", serde_json::to_string_pretty(&json).unwrap());
                        } else {
                            println!("\n[RECV] Non-JSON data: {}", trimmed);
                        }
                    }
                }
                Err(ref e) if e.kind() == io::ErrorKind::TimedOut => continue,
                Err(e) => {
                    eprintln!("Error reading from serial port: {}", e);
                    break;
                }
            }
        }
    });

    // Main sending loop
    loop {
        thread::sleep(Duration::from_millis(args.time));

        let mut port = port.lock().unwrap();

        // Send ANM message
        let anm_msg = create_anm_message();
        let anm_json = serde_json::to_string(&anm_msg).unwrap();
        
        println!("\n[SEND] ANM message");
        println!("{}", anm_json);

        match port.write_all(format!("{}\n", anm_json).as_bytes()) {
            Ok(_) => port.flush().ok(),
            Err(e) => {
                eprintln!("Error writing ANM to serial port: {}", e);
                break;
            }
        };

        // Send SPS message immediately after
        let sps_msg = create_sps_message();
        let sps_json = serde_json::to_string(&sps_msg).unwrap();
        
        println!("\n[SEND] SPS message");
        println!("{}", sps_json);

        match port.write_all(format!("{}\n", sps_json).as_bytes()) {
            Ok(_) => port.flush().ok(),
            Err(e) => {
                eprintln!("Error writing SPS to serial port: {}", e);
                break;
            }
        };
    }

    reader_handle.join().unwrap();
    Ok(())
}