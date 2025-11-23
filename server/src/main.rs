use clap::Parser;
use serde::{Deserialize, Serialize};
use serde_json::json;
use serialport::SerialPort;
use std::io::{self, BufRead, BufReader, Write};
use std::sync::{Arc, Mutex};
use std::thread;
use std::time::{Duration, SystemTime, UNIX_EPOCH};

#[derive(Parser, Debug)]
#[command(author, version, about = "Serial JSON sender: listen first, then periodically send two example JSON messages")]
struct Args {
    /// Serial port path (e.g. /dev/ttyUSB0 or COM3)
    #[arg(short, long)]
    port: String,

    /// Time between sending the two messages, in milliseconds
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

fn now_secs_f64() -> f64 {
    SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_secs_f64()
}

fn now_secs_u64() -> u64 {
    SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_secs()
}

fn create_anm_message() -> AnmMessage {
    AnmMessage {
        topic: "anm".to_string(),
        timestamp: now_secs_f64(),
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
        timestamp: now_secs_u64(),
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

    println!("Serial JSON sender");
    println!("Port: {}", args.port);
    println!("Send interval (ms): {}", args.time);

    // Open serial port
    let port = serialport::new(&args.port, 115200)
        .timeout(Duration::from_millis(100))
        .open()
        .expect("Failed to open serial port");

    // Wrap in Arc<Mutex<>> so both threads can use it
    let port = Arc::new(Mutex::new(port));

    // START READER THREAD FIRST
    {
        let port_reader = Arc::clone(&port);

        thread::spawn(move || {
            // Acquire a clone of the underlying serial port for reading
            // We lock briefly to call try_clone() which clones the OS handle.
            let cloned = {
                let guard = port_reader.lock().expect("port lock poisoned");
                guard.try_clone().expect("Failed to clone serial port for reading")
            };

            let mut reader = BufReader::new(cloned);
            println!("[INFO] Reader thread started — listening on serial port...");

            loop {
                let mut line = String::new();
                match reader.read_line(&mut line) {
                    Ok(0) => {
                        // No data available (EOF) — small sleep to avoid busy loop
                        thread::sleep(Duration::from_millis(10));
                        continue;
                    }
                    Ok(_) => {
                        let trimmed = line.trim();
                        if trimmed.is_empty() {
                            continue;
                        }

                        // Print raw line and try to pretty-print JSON if possible
                        println!("\n[RECV] {}", trimmed);

                        match serde_json::from_str::<serde_json::Value>(trimmed) {
                            Ok(json_val) => {
                                println!("{}", serde_json::to_string_pretty(&json_val).unwrap());
                            }
                            Err(_) => {
                                // not JSON
                            }
                        }
                    }
                    Err(ref e) if e.kind() == io::ErrorKind::TimedOut => {
                        // timeout — continue reading
                        continue;
                    }
                    Err(e) => {
                        eprintln!("[ERROR] Reader error: {}", e);
                        // short pause before retrying to avoid tight loop on fatal errors
                        thread::sleep(Duration::from_millis(500));
                    }
                }
            }
        });
    }

    // Give the reader a short moment to initialize and start listening.
    thread::sleep(Duration::from_millis(500));

    // SENDER LOOP
    loop {
        // Wait interval before sending next pair
        thread::sleep(Duration::from_millis(args.time));

        // prepare messages
        let anm = create_anm_message();
        let sps = create_sps_message();

        // serialize
        let anm_json = serde_json::to_string(&anm).expect("Failed to serialize ANM");
        let sps_json = serde_json::to_string(&sps).expect("Failed to serialize SPS");

        // Acquire lock for writing (short-lived)
        {
            let mut port_guard = port.lock().expect("port lock poisoned");

            // write ANM with newline
            if let Err(e) = port_guard.write_all(anm_json.as_bytes()) {
                eprintln!("[ERROR] Failed to write ANM: {}", e);
            } else {
                // ensure newline and flush
                if let Err(e) = port_guard.write_all(b"\n") {
                    eprintln!("[ERROR] Failed to write newline after ANM: {}", e);
                }
                if let Err(e) = port_guard.flush() {
                    eprintln!("[ERROR] Failed to flush after ANM: {}", e);
                }
                println!("\n[SEND] ANM\n{}", anm_json);
            }

            // write SPS with newline
            if let Err(e) = port_guard.write_all(sps_json.as_bytes()) {
                eprintln!("[ERROR] Failed to write SPS: {}", e);
            } else {
                if let Err(e) = port_guard.write_all(b"\n") {
                    eprintln!("[ERROR] Failed to write newline after SPS: {}", e);
                }
                if let Err(e) = port_guard.flush() {
                    eprintln!("[ERROR] Failed to flush after SPS: {}", e);
                }
                println!("\n[SEND] SPS\n{}", sps_json);
            }
        }

        // loop continues...
    }
}
