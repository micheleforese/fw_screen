use clap::Parser;
use std::io::{BufRead, BufReader};
use std::time::Duration;
use std::{io, process};

#[derive(Parser, Debug)]
#[command(author, version, about, long_about = None)]
struct Args {
    /// Serial port path (e.g., /dev/ttyUSB0)
    #[arg(short, long)]
    port: String,

    /// Baud rate (default: 115200)
    #[arg(short, long, default_value_t = 115200)]
    baud: u32,
}

fn main() -> io::Result<()> {
    let args = Args::parse();

    println!("═══════════════════════════════════════");
    println!("  Serial Port Listener");
    println!("═══════════════════════════════════════");
    println!("Port: {}", args.port);
    println!("Baud rate: {}", args.baud);
    println!("═══════════════════════════════════════\n");
    println!("Listening for messages... (Press Ctrl+C to exit)\n");

    // Open serial port
    let port = match serialport::new(&args.port, args.baud)
        .timeout(Duration::from_millis(1000))
        .open()
    {
        Ok(p) => p,
        Err(e) => {
            eprintln!("❌ Failed to open serial port: {}", e);
            eprintln!("\nAvailable ports:");
            if let Ok(ports) = serialport::available_ports() {
                for port in ports {
                    eprintln!("  - {}", port.port_name);
                }
            }
            process::exit(1);
        }
    };

    let mut reader = BufReader::new(port);
    let mut message_count = 0;

    loop {
        let mut line = String::new();
        match reader.read_line(&mut line) {
            Ok(0) => {
                // EOF - port might be closed
                eprintln!("\n❌ Port closed or EOF reached");
                break;
            }
            Ok(bytes) => {
                let trimmed = line.trim();
                if !trimmed.is_empty() {
                    message_count += 1;
                    println!("\n─────────────────────────────────────");
                    println!("📨 Message #{} ({} bytes)", message_count, bytes);
                    println!("─────────────────────────────────────");

                    // Try to parse as JSON for pretty printing
                    if let Ok(json) = serde_json::from_str::<serde_json::Value>(trimmed) {
                        // Check if it has a topic field
                        if let Some(topic) = json.get("topic").and_then(|t| t.as_str()) {
                            println!("📋 Type: {} message", topic.to_uppercase());
                        }
                        println!("\n{}", serde_json::to_string_pretty(&json).unwrap());
                    } else {
                        // Not JSON, print as raw text
                        println!("📄 Raw text:");
                        println!("{}", trimmed);
                    }
                }
            }
            Err(ref e) if e.kind() == io::ErrorKind::TimedOut => {
                // Timeout is normal, just continue listening
                continue;
            }
            Err(e) => {
                eprintln!("\n❌ Error reading from serial port: {}", e);
                break;
            }
        }
    }

    println!("\nTotal messages received: {}", message_count);
    Ok(())
}