use clap::Parser;
use rumqttc::{AsyncClient, Event, EventLoop, MqttOptions, Packet, QoS, SubscribeFilter};
use serde_json::json;
use std::sync::Arc;
use std::time::{Duration, Instant};
use tokio::sync::mpsc::{self, Sender};
use tokio::sync::{Mutex, watch};
use tokio::time::sleep;
use tokio_serial::SerialPort;

#[derive(Parser, Debug)]
#[command(author, version)]
struct Args {
    /// Communication Port
    #[arg(short, long, default_value_t = String::from("/dev/ttyACM1"))]
    port: String,

    /// Baud rate
    #[arg(short, long, default_value_t = 115200)]
    baud: u32,

    #[arg(long, default_value_t = String::from("server"),)]
    mqtt_id: String,

    #[arg(long, default_value_t = String::from("localhost"),)]
    mqtt_host: String,

    #[arg(long, default_value_t = 1883)]
    mqtt_port: u16,

    #[arg(long, default_value_t = 0)]
    filter_seconds: u64,

    #[arg(
        long,
        default_value_t = 5_000,
        help = "MQTT Re-connection delay in ms (milliseconds)."
    )]
    mqtt_reconnection_delay_ms: u64,

    #[arg(
        long,
        default_value_t = 5_000,
        help = "Serial Re-connection delay in ms (milliseconds)."
    )]
    serial_reconnection_delay_ms: u64,
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

#[tokio::main(flavor = "current_thread")]
async fn main() {
    let args = Args::parse();

    println!("Serial JSON sender");
    println!("Port: {}", args.port);
    println!("Baud Rate: {}", args.baud);
    println!("MQTT id: {}", args.mqtt_id);
    println!("MQTT Host: {}", args.mqtt_host);
    println!("MQTT Port: {}", args.mqtt_port);
    println!("Filter Seconds: {}", args.filter_seconds);

    let (mqtt_watch_channel_tx, mqtt_watch_channel_rx) = watch::channel(false);
    let (serial_watch_channel_tx, serial_watch_channel_rx) = watch::channel(false);
    let (mqtt_serial_queue_tx, mqtt_serial_queue_rx) = mpsc::channel::<String>(100);
    let (serial_mqtt_queue_tx, serial_mqtt_queue_rx) = mpsc::channel::<String>(100);

    let mut mqttoptions = MqttOptions::new(args.mqtt_id, args.mqtt_host, args.mqtt_port);
    mqttoptions.set_keep_alive(Duration::from_secs(30));
    let (client, eventloop) = AsyncClient::new(mqttoptions.clone(), 10);

    let mqtt_client = Arc::new(Mutex::new(client));
    let mqtt_eventloop = Arc::new(Mutex::new(eventloop));

    let serial_port = Arc::new(Mutex::new(None::<Box<dyn SerialPort>>));

    // MQTT TASK
    let mqtt_eventloop_clone = mqtt_eventloop.clone();
    let mqtt_client_clone = mqtt_client.clone();
    let mqtt_watch_channel_tx_clone = mqtt_watch_channel_tx.clone();
    let mqtt_task_handle = tokio::spawn(async move {
        mqtt_task(
            mqtt_eventloop_clone,
            mqtt_client_clone,
            &mqtt_serial_queue_tx,
            mqtt_watch_channel_tx_clone,
            Duration::from_secs(args.filter_seconds),
            Duration::from_millis(args.mqtt_reconnection_delay_ms),
        )
        .await;
    });

    // SERIAL Writer Task
    let serial_port_clone: Arc<Mutex<Option<Box<dyn SerialPort>>>> = serial_port.clone();
    let serial_watch_channel_tx_clone = serial_watch_channel_tx.clone();
    let serial_watch_channel_rx_clone = serial_watch_channel_rx.clone();
    let serial_writer = tokio::spawn(async move {
        serial_writer_task(
            mqtt_serial_queue_rx,
            serial_port_clone,
            serial_watch_channel_tx_clone,
            serial_watch_channel_rx_clone,
        )
        .await
    });

    // SERIAL -> MQTT
    // SERIAL Reconnection
    let serial_port_clone = serial_port.clone();
    let serial_watch_channel_tx_clone = serial_watch_channel_tx.clone();
    let serial_watch_channel_rx_clone = serial_watch_channel_rx.clone();
    let serial_reconnect = tokio::spawn(async move {
        serial_reconnect_task(
            &args.port,
            args.baud,
            serial_port_clone,
            serial_watch_channel_tx_clone,
            serial_watch_channel_rx_clone,
            Duration::from_millis(args.serial_reconnection_delay_ms),
        )
        .await;
    });

    // SERIAL Port -> SERIAL-MQTT Queue
    let serial_port_clone: Arc<Mutex<Option<Box<dyn SerialPort>>>> = serial_port.clone();
    let serial_watch_channel_tx_clone = serial_watch_channel_tx.clone();
    let serial_watch_channel_rx_clone = serial_watch_channel_rx.clone();
    let serial_listener = tokio::spawn(async move {
        serial_listener_task(
            serial_port_clone,
            serial_mqtt_queue_tx,
            serial_watch_channel_tx_clone,
            serial_watch_channel_rx_clone,
        )
        .await;
    });

    // MQTT Writer Task
    let mqtt_client_clone = mqtt_client.clone();
    let mqtt_watch_channel_tx_clone = mqtt_watch_channel_tx.clone();
    let mqtt_watch_channel_rx_clone = mqtt_watch_channel_rx.clone();
    let mqtt_writer = tokio::spawn(async move {
        mqtt_writer_task(
            serial_mqtt_queue_rx,
            mqtt_client_clone,
            mqtt_watch_channel_tx_clone,
            mqtt_watch_channel_rx_clone,
        )
        .await;
    });

    // Wait for all tasks
    let _ = tokio::join!(
        mqtt_task_handle,
        serial_writer,
        serial_reconnect,
        serial_listener,
        mqtt_writer,
    );
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

async fn serial_writer_task(
    mut serial_queue_rx: mpsc::Receiver<String>,
    port: Arc<Mutex<Option<Box<dyn SerialPort>>>>,
    serial_flag_tx: watch::Sender<bool>,
    serial_flag_rx: watch::Receiver<bool>,
) {
    println!("[TASK] SERIAL writer: START");

    while let Some(message) = serial_queue_rx.recv().await {
        println!("Writer sending: {}", &message);

        loop {
            while !*serial_flag_rx.borrow() {
                sleep(Duration::from_millis(100)).await;
            }

            let mut should_break = false;
            if let Some(port) = port.lock().await.as_mut() {
                if let Err(e) = port.write_all((message.clone() + "\n").as_bytes()) {
                    eprintln!("[ERROR] Failed to write: {}", e);
                    let _ = serial_flag_tx.send(false);
                } else {
                    if let Err(e) = port.flush() {
                        eprintln!("[ERROR] Failed to flush: {}", e);
                    }
                    println!("\n[SEND] to port\n{}", message.clone());
                    should_break = true;
                }
            }

            if should_break {
                break;
            }
        }
    }
}

async fn mqtt_writer_task(
    mut mqtt_queue_rx: mpsc::Receiver<String>,
    mqtt_client: Arc<Mutex<AsyncClient>>,
    flag_tx: watch::Sender<bool>,
    flag_rx: watch::Receiver<bool>,
) {
    println!("[TASK] MQTT Writer: START");

    while let Some(json_str) = mqtt_queue_rx.recv().await {
        let topic = "command";

        println!("[TASK] MQTT QUEUE -> MQTT server.");

        loop {
            // Wait until connected
            while !*flag_rx.borrow() {
                sleep(Duration::from_millis(100)).await;
            }

            let json_parsed = serde_json::from_str::<serde_json::Value>(&json_str);

            match json_parsed {
                Ok(json_value) => {
                    if let Some(command_value) = json_value.get("command") {
                        if let Some(command_str) = command_value.as_str() {
                            let mqtt_message = format!("{}", command_str);

                            let mqtt_client_guard = mqtt_client.lock().await;

                            if let Err(e) = mqtt_client_guard
                                .publish(topic, QoS::AtMostOnce, false, mqtt_message.clone())
                                .await
                            {
                                eprintln!("Failed to publish MQTT message: {e}");
                                let _ = flag_tx.send(false);
                            } else {
                                println!("Published to MQTT: {}", mqtt_message);
                                break;
                            }
                        } else {
                            eprintln!("'command' is not a string in JSON: {}", json_str);
                            break;
                        }
                    } else {
                        eprintln!("JSON does not contain 'command' field: {}", json_str);
                        break;
                    }
                }
                Err(e) => {
                    eprintln!("Failed to parse JSON: {e}, input: {}", json_str);
                    break;
                }
            }
        }
    }
}

async fn serial_listener_task(
    serial_port: Arc<Mutex<Option<Box<dyn SerialPort>>>>,
    tx: mpsc::Sender<String>,
    serial_flag_tx: watch::Sender<bool>,
    serial_flag_rx: watch::Receiver<bool>,
) {
    println!("[TASK] SERIAL Listener: START");
    let mut buffer = String::new();

    loop {
        // Wait until we have a connection
        while !*serial_flag_rx.borrow() {
            sleep(Duration::from_millis(100)).await;
        }

        println!("📡 Serial listener task active - reading data...");

        loop {
            let mut port_guard = serial_port.lock().await;
            if let Some(port) = port_guard.as_mut() {
                let mut tmp_buf = [0u8; 1024 * 20];

                let result = port.read(&mut tmp_buf);

                // Release lock during read
                drop(port_guard);

                match result {
                    Ok(n) if n > 0 => {
                        buffer.push_str(&String::from_utf8_lossy(&tmp_buf[..n]));

                        while let Some(pos) = buffer.find('\n') {
                            let line = buffer.drain(..=pos).collect::<String>();
                            let line = line.trim();
                            if !line.is_empty() {
                                let json = serde_json::from_str::<serde_json::Value>(line);

                                match json {
                                    Ok(json_val) => {
                                        match tx.send(json_val.to_string()).await {
                                            Ok(_) => {}
                                            Err(e) => {
                                                println!("MQTT Queue Sending error: {:?}", e);
                                                break;
                                            }
                                        };
                                    }
                                    Err(_) => {
                                        eprintln!("ERROR Parse JSON from serial port.")
                                    }
                                }
                            }
                        }
                    }
                    Ok(_) => {
                        sleep(Duration::from_millis(100)).await;
                    }
                    Err(ref e) if e.kind() == std::io::ErrorKind::TimedOut => {
                        sleep(Duration::from_millis(100)).await;
                    }
                    Err(e) => {
                        eprintln!("Serial read error: {e}");
                        let _ = serial_flag_tx.send(false);
                        break;
                    }
                }
                tokio::time::sleep(Duration::from_millis(50)).await;
            } else {
                drop(port_guard);
                let _ = serial_flag_tx.send(false);
                break;
            }
        }
    }
}

fn has_elapsed_between(
    old_timestamp: Instant,
    current_timestamp: Instant,
    duration: Duration,
) -> bool {
    let elapsed = current_timestamp.duration_since(old_timestamp);
    elapsed >= duration
}

async fn serial_reconnect_task(
    serial_option_port: &str,
    serial_option_baud_rate: u32,
    serial_port: Arc<Mutex<Option<Box<dyn SerialPort>>>>,
    serial_flag_tx: watch::Sender<bool>,
    serial_flag_rx: watch::Receiver<bool>,
    reconnection_delay: Duration,
) {
    println!("[TASK] SERIAL Reconnect: START");

    loop {
        // Wait until disconnected
        while *serial_flag_rx.borrow() {
            sleep(Duration::from_millis(100)).await;
        }

        println!("[SERIAL] 🔄 Reconnection Task: Active - attempting to connect...");

        match open_serial_port(serial_option_port, serial_option_baud_rate) {
            Ok(port) => {
                println!("[SERIAL] Connected to port: {}", serial_option_port);
                *serial_port.lock().await = Some(port);
                let _ = serial_flag_tx.send(true);
            }
            Err(e) => {
                println!(
                    "[SERIAL] Failed to open serial port: \"{}\". Retrying in {:?}...",
                    e, reconnection_delay
                );
                *serial_port.lock().await = None;
                let _ = serial_flag_tx.send(false);
                sleep(reconnection_delay).await;
            }
        }
    }
}

fn open_serial_port(
    serial_port: &str,
    baud_rate: u32,
) -> Result<Box<dyn SerialPort>, tokio_serial::Error> {
    tokio_serial::new(serial_port, baud_rate)
        .timeout(Duration::from_millis(100))
        .open()
}

// Combined MQTT connection and listener task
async fn mqtt_task(
    mqtt_eventloop: Arc<Mutex<EventLoop>>,
    mqtt_client: Arc<Mutex<AsyncClient>>,
    mqtt_queue_channel_tx: &Sender<String>,
    mqtt_flag_tx: watch::Sender<bool>,
    filter_duration: Duration,
    reconnection_delay: Duration,
) {
    println!("[TASK] MQTT Combined (Connect + Listen): START");

    let mut last_msg_anm_timestamp: Instant = Instant::now();
    let mut is_subscribed = false;

    loop {
        // Try to subscribe if not already subscribed
        if !is_subscribed {
            println!("[MQTT] 🔄 Attempting to subscribe to topics...");

            let mqtt_client_guard = mqtt_client.lock().await;
            let result = mqtt_client_guard
                .subscribe_many([
                    SubscribeFilter {
                        path: String::from("anemometer"),
                        qos: QoS::AtMostOnce,
                    },
                    SubscribeFilter {
                        path: String::from("sps30"),
                        qos: QoS::AtMostOnce,
                    },
                    SubscribeFilter {
                        path: String::from("imu"),
                        qos: QoS::AtMostOnce,
                    },
                    SubscribeFilter {
                        path: String::from("status"),
                        qos: QoS::AtMostOnce,
                    },
                ])
                .await;

            drop(mqtt_client_guard);

            match result {
                Ok(_) => {
                    println!("[MQTT] ✓ Successfully subscribed to topics");
                    is_subscribed = true;
                    let _ = mqtt_flag_tx.send(true);
                }
                Err(e) => {
                    println!(
                        "[MQTT] ✗ Failed to subscribe: {}. Retrying in {:?}...",
                        e, reconnection_delay
                    );
                    let _ = mqtt_flag_tx.send(false);
                    sleep(reconnection_delay).await;
                    continue;
                }
            }
        }

        // Poll MQTT events
        println!("[MQTT] 📡 Polling MQTT events...");

        let mut event_loop_guard = mqtt_eventloop.lock().await;
        let result = event_loop_guard.poll().await;
        drop(event_loop_guard);

        match result {
            Ok(Event::Incoming(Packet::Publish(publish))) => {
                let topic = publish.topic;
                let payload = publish.payload;

                let topic_type = match_topic(&topic);

                if topic_type == TopicType::Unknown {
                    continue;
                }

                if topic_type == TopicType::Anemometer {
                    let current_time = Instant::now();
                    if !has_elapsed_between(last_msg_anm_timestamp, current_time, filter_duration) {
                        continue;
                    }

                    last_msg_anm_timestamp = current_time;
                }

                // Convert payload to string (if UTF-8)
                if let Ok(text) = std::str::from_utf8(&payload) {
                    println!("📩 Message received:");
                    println!("   Topic: {topic}");
                    println!("   Payload: {text}");

                    match serde_json::from_str::<serde_json::Value>(text) {
                        Ok(json_val) => {
                            match topic_type {
                                TopicType::Anemometer => {
                                    mqtt_anemometer_topic_callback(
                                        &json_val,
                                        &mqtt_queue_channel_tx,
                                    )
                                    .await
                                }
                                TopicType::SPS30 => {
                                    mqtt_sps30_topic_callback(&json_val, &mqtt_queue_channel_tx)
                                        .await
                                }
                                TopicType::Imu => {
                                    mqtt_imu_topic_callback(&json_val, &mqtt_queue_channel_tx).await
                                }
                                TopicType::Status => {
                                    mqtt_status_topic_callback(&json_val, &mqtt_queue_channel_tx)
                                        .await
                                }
                                TopicType::Unknown => {}
                            };
                        }
                        Err(_) => {
                            println!("[ERROR] Not a JSON Value.");
                        }
                    }
                } else {
                    println!("Binary payload received on {topic}: {payload:?}");
                }
            }
            Ok(Event::Incoming(i)) => {
                println!("[MQTT] Event::Incoming: {:?}", i);
            }
            Ok(Event::Outgoing(o)) => {
                println!("[MQTT] Event::Outgoing: {:?}", o);
            }
            Err(e) => {
                println!(
                    "[MQTT] Connection error: {}. Will retry in {:?}...",
                    e, reconnection_delay
                );
                is_subscribed = false;
                let _ = mqtt_flag_tx.send(false);
                sleep(reconnection_delay).await;
            }
        }
    }
}
