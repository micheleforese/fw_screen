use clap::Parser;
use rumqttc::{AsyncClient, Event, Incoming, LastWill, MqttOptions, QoS, SubscribeFilter};
use serde_json::json;
use std::error::Error;
use std::io::Write;
use std::sync::Arc;
use std::time::Duration;
use tokio::sync::Mutex;
use tokio::sync::mpsc::{self, Sender};
use tokio::{task, time};
use tokio_serial::SerialPort;

#[derive(Parser, Debug)]
#[command(
    author,
    version,
    about = "Serial JSON sender: listen first, then periodically send two example JSON messages"
)]
struct Args {
    /// Communication Port
    #[arg(short, long)]
    port: String,

    /// Baud rate
    #[arg(short, long, default_value_t = 115200)]
    baud: u32,

    #[arg(long)]
    mqtt_id: String,

    #[arg(long)]
    mqtt_host: String,

    #[arg(long)]
    mqtt_port: u16,

    #[arg(long)]
    n: u16,
}

fn match_topic(topic: &str) -> TopicType {
    match topic {
        "anemometer" => TopicType::Anemometer,
        "sps30" => TopicType::SPS30,
        "imu" => TopicType::Imu,
        "status" => TopicType::Status,
        _ => TopicType::Unknown,
    }
}

#[derive(PartialEq)]
enum TopicType {
    Anemometer,
    SPS30,
    Imu,
    Status,
    Unknown,
}

async fn mqtt_anemometer_topic_callback(json: &serde_json::Value, tx: &Sender<String>) {
    let mut json = json.clone();
    if let Some(obj) = json.as_object_mut() {
        obj.insert("topic".to_string(), json!("anm"));
        let pretty_json_string = serde_json::to_string_pretty(&json).unwrap();
        if let Err(e) = tx.send(pretty_json_string).await {
            eprintln!("Failed to send message: {e}");
        }
    }
}

async fn mqtt_sps30_topic_callback(json: &serde_json::Value, tx: &Sender<String>) {
    let mut json = json.clone();
    if let Some(obj) = json.as_object_mut() {
        obj.insert("topic".to_string(), json!("sps"));
        let pretty_json_string = serde_json::to_string_pretty(&json).unwrap();
        if let Err(e) = tx.send(pretty_json_string).await {
            eprintln!("Failed to send message: {e}");
        }
    }
}

async fn mqtt_imu_topic_callback(json: &serde_json::Value, tx: &Sender<String>) {
    let mut json = json.clone();
    if let Some(obj) = json.as_object_mut() {
        obj.insert("topic".to_string(), json!("imu"));
        let pretty_json_string = serde_json::to_string_pretty(&json).unwrap();
        if let Err(e) = tx.send(pretty_json_string).await {
            eprintln!("Failed to send message: {e}");
        }
    }
}

async fn mqtt_status_topic_callback(json: &serde_json::Value, tx: &Sender<String>) {
    let mut json = json.clone();
    if let Some(obj) = json.as_object_mut() {
        obj.insert("topic".to_string(), json!("status"));
        let pretty_json_string = serde_json::to_string_pretty(&json).unwrap();
        if let Err(e) = tx.send(pretty_json_string).await {
            eprintln!("Failed to send message: {e}");
        }
    }
}

async fn mqtt_to_serial_channel_task(
    mut rx: mpsc::Receiver<String>,
    port: Arc<Mutex<Box<dyn SerialPort>>>,
) -> Result<(), Box<dyn Error>> {
    // Consumer loop: wait for messages from any worker
    while let Some(message) = rx.recv().await {
        println!("Writer sending: {message}");
        {
            let mut port_guard = port.lock().await;
            if let Err(e) = port_guard.write_all((message.clone() + "\n").as_bytes()) {
                eprintln!("[ERROR] Failed to write: {}", e);
            } else {
                if let Err(e) = port_guard.flush() {
                    eprintln!("[ERROR] Failed to flush: {}", e);
                }
                println!("\n[SEND] to port\n{}", message.clone());
            }
        }
    }

    Ok(())
}

async fn mqtt_sender_task(mut rx: mpsc::Receiver<String>, mqtt_client: AsyncClient) {
    while let Some(json_str) = rx.recv().await {
        let topic = "command";

        match serde_json::from_str::<serde_json::Value>(&json_str) {
            Ok(json_value) => {
                if let Some(command_value) = json_value.get("command") {
                    // Estraiamo la stringa interna senza virgolette
                    if let Some(command_str) = command_value.as_str() {
                        let mqtt_message = format!("{}", command_str);

                        if let Err(e) = mqtt_client
                            .publish(topic, QoS::AtMostOnce, false, mqtt_message.clone())
                            .await
                        {
                            eprintln!("Failed to publish MQTT message: {e}");
                        } else {
                            println!("Published to MQTT: {}", mqtt_message);
                        }
                    } else {
                        eprintln!("'command' is not a string in JSON: {}", json_str);
                    }
                } else {
                    eprintln!("JSON does not contain 'command' field: {}", json_str);
                }
            }
            Err(e) => {
                eprintln!("Failed to parse JSON: {e}, input: {}", json_str);
            }
        }
    }
}

async fn serial_to_mqtt_channel_task(
    port: Arc<Mutex<Box<dyn SerialPort>>>,
    tx: mpsc::Sender<String>,
) -> Result<(), Box<dyn Error>> {
    let mut buffer = String::new();
    loop {
        let mut port = port.lock().await;
        let mut tmp_buf = [0u8; 1024*20];
        match port.read(&mut tmp_buf) {
            Ok(n) if n > 0 => {
                buffer.push_str(&String::from_utf8_lossy(&tmp_buf[..n]));
                while let Some(pos) = buffer.find('\n') {
                    let line = buffer.drain(..=pos).collect::<String>();
                    let line = line.trim();
                    if !line.is_empty() {
                        let json = serde_json::from_str::<serde_json::Value>(line);

                        match json {
                            Ok(json_val) => {
                                tx.send(json_val.to_string()).await.unwrap();
                            }
                            Err(_) => {
                                eprintln!("ERROR Parse JSON from serial port.")
                            }
                        }
                    }
                }
            }
            Ok(_) => {}
            Err(ref e) if e.kind() == std::io::ErrorKind::TimedOut => {}
            Err(e) => eprintln!("Serial read error: {e}"),
        }
        tokio::time::sleep(Duration::from_millis(50)).await;
    }
}

#[tokio::main(flavor = "current_thread")]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    let args = Args::parse();

    println!("Serial JSON sender");
    println!("Port: {}", args.port);
    println!("Baud Rate: {}", args.baud);
    println!("MQTT id: {}", args.mqtt_id);
    println!("MQTT Host: {}", args.mqtt_host);
    println!("MQTT Port: {}", args.mqtt_port);
    println!("N: {}", args.n);

    let (tx, rx) = mpsc::channel::<String>(100);
    let (tx_serial_to_mqtt, rx_serial_to_mqtt) = mpsc::channel::<String>(100);

    let port: Box<dyn SerialPort> = tokio_serial::new(&args.port, args.baud)
        .timeout(Duration::from_millis(100))
        .open()
        .expect("Failed to open serial port");

    let mut mqttoptions = MqttOptions::new(args.mqtt_id, args.mqtt_host, args.mqtt_port);
    let will = LastWill::new("hello/world", "good bye", QoS::AtMostOnce, false);
    mqttoptions
        .set_keep_alive(Duration::from_secs(5))
        .set_last_will(will);

    let (client, mut eventloop) = AsyncClient::new(mqttoptions, 10);

    client
        .subscribe_many([
            SubscribeFilter {
                path: "anemometer".to_string(),
                qos: QoS::AtLeastOnce,
            },
            SubscribeFilter {
                path: "sps30".to_string(),
                qos: QoS::AtLeastOnce,
            },
            SubscribeFilter {
                path: "imu".to_string(),
                qos: QoS::AtLeastOnce,
            },
            SubscribeFilter {
                path: "status".to_string(),
                qos: QoS::AtLeastOnce,
            },
        ])
        .await
        .expect("[MQTT] Subscribing failed");

    let port = Arc::new(Mutex::new(port));

    let port_write = Arc::clone(&port);
    let port_read = Arc::clone(&port);

    tokio::spawn(async move {
        if let Err(e) = mqtt_to_serial_channel_task(rx, port_write).await {
            eprintln!("Writer task failed: {e}");
        }
    });

    let tx_clone = tx_serial_to_mqtt.clone();
    tokio::spawn(async move {
        if let Err(e) = serial_to_mqtt_channel_task(port_read, tx_clone).await {
            eprintln!("Writer task failed: {e}");
        }
    });

    let client_clone = client.clone();

    tokio::spawn(async move {
        mqtt_sender_task(rx_serial_to_mqtt, client_clone).await;
    });

    let mut counter = 0;

    loop {
        let event = eventloop.poll().await;

        match event {
            Ok(Event::Incoming(incoming)) => {
                match incoming {
                    Incoming::Publish(publish) => {
                        let topic = publish.topic;
                        let payload = publish.payload;

                        let topic_type = match_topic(&topic);

                        if topic_type == TopicType::Unknown {
                            continue;
                        }

                        // Convert payload to string (if UTF-8)
                        if let Ok(text) = std::str::from_utf8(&payload) {
                            println!("📩 Message received:");
                            println!("   Topic: {topic}");
                            println!("   Payload: {text}");

                            counter += 1;
                            if counter != args.n {
                                continue;
                            }
                            counter = 0;

                            match serde_json::from_str::<serde_json::Value>(text) {
                                Ok(json_val) => {
                                    match topic_type {
                                        TopicType::Anemometer => {
                                            mqtt_anemometer_topic_callback(&json_val, &tx).await
                                        }
                                        TopicType::SPS30 => {
                                            mqtt_sps30_topic_callback(&json_val, &tx).await
                                        }
                                        TopicType::Imu => {
                                            mqtt_imu_topic_callback(&json_val, &tx).await
                                        }
                                        TopicType::Status => {
                                            mqtt_status_topic_callback(&json_val, &tx).await
                                        }
                                        TopicType::Unknown => todo!(),
                                    };
                                }
                                Err(_) => {
                                    // not JSON
                                }
                            }
                        } else {
                            println!("Binary payload received on {topic}: {payload:?}");
                        }
                    }

                    Incoming::SubAck(ack) => {
                        println!("Subscription ACK: {:?}", ack);
                    }

                    Incoming::PingResp => {
                        println!("Ping response");
                    }

                    // Catch all other events
                    other => {
                        println!("Other incoming event: {:?}", other);
                    }
                }
            }

            Ok(Event::Outgoing(out)) => {
                println!("Outgoing event: {:?}", out);
            }

            Err(e) => {
                eprintln!("MQTT error: {e:?}");
                break;
            }
        }
    }
    Ok(())
}
