use gilrs::{Axis, Button, Event, Gilrs};
use std::{collections::HashMap, fs};
use std::{net::UdpSocket, time::SystemTime};

const DEFAULT_CONFIG: &str = "target=192.168.1.101:1237";

fn alert_about_malformed_config(message: &str) {
    println!("\n\n-----------------");
    println!("{}", message);
    println!("The config should be a single line as folows: ");
    println!("\ttarget=ARDUINO_IP:PORT");
    println!("Will assume default, which is {}", &DEFAULT_CONFIG);
    println!("-----------------\n");
}

//Given the map of axes to their positions, and an axis you want
//Will retrieve the corresponding joystick position from the map,
//And if the number is negative, will subtract it from 256 to
//send the two's complement instead

#[allow(overflowing_literals)]
fn format_axis_to_send(axis: Axis, map: &HashMap<&Axis, f32>) -> u8 {
    let motor_value = (map.get(&axis).unwrap_or(&0.0) * 90.0).floor() as i16;
    println!("Given motor value {}", motor_value);
    if motor_value < 0 {
        //plus motor value because motor value is negative.
        //so this will subtract the absolute value of motor value from 256
        return (256 + motor_value) as u8;
    } else {
        //value is positive, so we don't need to do anything special
        return motor_value as u8;
    }
}

fn main() {
    let mut gilrs = Gilrs::new().unwrap();

    let config_file = fs::read_to_string("./udp_mc_drive.conf");
    let mut config_file = match config_file {
        Ok(data) => data.trim().to_string(),
        Err(err) => match err.kind() {
            std::io::ErrorKind::NotFound => {
                println!("Could not find config file, writing one and initializing with default");
                fs::write("./udp_mc_drive.conf", &DEFAULT_CONFIG).unwrap();
                fs::read_to_string("./udp_mc_drive.conf").unwrap()
            }
            _ => {
                panic!("Unknown error: {:?}", err.kind())
            }
        },
    };

    let num_lines_in_config_file = config_file.lines().count();
    if num_lines_in_config_file > 1 {
        alert_about_malformed_config(
            "The config file is more than one line\nYou are most likely using an old config",
        );
        config_file = DEFAULT_CONFIG.to_string();
    }

    println!("Config file reads: {:#?}", config_file);
    match config_file.find("target=") {
        Some(index) => {
            if index != 0 {
                alert_about_malformed_config(
                    "Cannot find target in config file. Make sure there are no spaces.",
                );
            }
            "target=".len()
        }
        None => {
            alert_about_malformed_config(
                "Cannot find target in config file. Make sure there are no spaces.",
            );
            "target=".len()
        }
    };

    // Iterate over all connected gamepads
    for (_id, gamepad) in gilrs.gamepads() {
        println!("Using controller: {:?}", gamepad.name());
    }

    let mut active_gamepad = None;

    let mut time_of_last_send = SystemTime::now();

    let socket = UdpSocket::bind("0.0.0.0:43257").expect("Couldn't bind to addr");

    loop {
        // Examine new events
        while let Some(Event {
            id,
            event: _,
            time: _,
        }) = gilrs.next_event()
        {
            // println!("{:?} New event from {}: {:?}", time, id, event);
            active_gamepad = Some(id);
            continue;
        }

        if let Some(gamepad) = active_gamepad.map(|id| gilrs.gamepad(id)) {
            let axes = [
                Axis::RightStickY,
                Axis::RightStickX,
                Axis::LeftStickY,
                Axis::LeftStickX,
            ];

            //creates a map from each axis to its value
            let mut map = HashMap::new();

            for axis in axes.iter() {
                if let Some(data) = gamepad.axis_data(axis.to_owned()) {
                    if data.value().abs() > 0.1 {
                        // println!("{:#?}: {}", axis, data.value());
                        map.insert(axis, data.value());
                    } else {
                        map.insert(axis, 0.0);
                    }
                }
            }

            //sends a message every 100ms
            if SystemTime::now()
                .duration_since(time_of_last_send)
                .unwrap()
                .as_millis()
                > 100
            {
                time_of_last_send = SystemTime::now();
                let mut buf = [
                    255 - 127 + 1,
                    0,
                    0,
                    format_axis_to_send(Axis::LeftStickY, &map),
                    format_axis_to_send(Axis::RightStickY, &map),
                    0,
                    0,
                    255, //dummy value, will be overwritten with the hash
                ];
                //this overwrite is nice so that we can reference elements of buf instead of copy
                //pasting format_axis_to_send calls twice.
                buf[buf.len() - 1] = ((buf[1] + buf[2] + buf[3] + buf[4] + buf[5]) / 5) as u8;
                socket
                    .send_to(&buf, "192.168.1.231:4242")
                    .expect("Couldn't send data");
            }
        }
    }
}
