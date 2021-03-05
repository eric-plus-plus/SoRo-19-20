use gilrs::{Axis, Button, Event, Gamepad, Gilrs};
use std::{collections::HashMap, convert::TryInto, fs};
use std::{net::UdpSocket, time::SystemTime};

const DEFAULT_CONFIG: &str = "target=10.0.0.1:1003";

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

fn to_twos_complement(n: i32) -> u8 {
    if n < 0 {
        return (256 + n) as u8;
    }
    return n as u8;
}


fn clamp(val: &f32, lower_bound: f32, upper_bound:f32) -> f32{
    lower_bound.max(*val).min(upper_bound)
}

fn calculate_shoulder_velocity(gamepad: &Gamepad) -> f32{
    if let Some(axis_val) = gamepad.axis_data(Axis::RightStickY) {
        return axis_val.value();
    }
    return 0.0;
}

fn calculate_elbow_velocity(gamepad: &Gamepad) -> f32{
    if let Some(axis_val) = gamepad.axis_data(Axis::LeftStickY) {
        return axis_val.value() * -1.0;
    }
    return 0.0;
}


fn calculate_claw_velocity(gamepad: &Gamepad, positive_button: Button, negative_button: Button) -> f32{
    let mut to_return = 0.0;
    if gamepad.is_pressed(positive_button) {
        to_return += 1.0;
    }
    if gamepad.is_pressed(negative_button) {
        to_return -= 1.0;
    }
    return to_return;
}

fn main() {
    let mut gilrs = Gilrs::new().unwrap();

    let config_file = fs::read_to_string("./udp_mc_arm.conf");
    let mut config_file = match config_file {
        Ok(data) => data.trim().to_string(),
        Err(err) => match err.kind() {
            std::io::ErrorKind::NotFound => {
                println!("Could not find config file, writing one and initializing with default");
                fs::write("./udp_mc_arm.conf", &DEFAULT_CONFIG).unwrap();
                fs::read_to_string("./udp_mc_arm.conf").unwrap()
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

    let split_config: Vec<&str> = config_file
        .strip_prefix("target=")
        .unwrap()
        .split(":")
        .collect();
    let arduino_ip = split_config
        .get(0)
        .expect("Could not find ip from split confi");
    let arduino_port = split_config
        .get(1)
        .expect("Could not find port from split config");
    println!("Will send to {}:{}", arduino_ip, arduino_port);

    // Iterate over all connected gamepads
    for (_id, gamepad) in gilrs.gamepads() {
        println!("Using controller: {:?}", gamepad.name());
    }

    let mut active_gamepad = None;

    let mut time_of_last_send = SystemTime::now();

    let socket = UdpSocket::bind("0.0.0.0:43257").expect("Couldn't bind to addr");

    let mut last_shoulder_value = 90.0;
    let mut last_elbow_value = 90.0;
    let mut last_claw_l_position = 75.0;
    let mut last_claw_r_position = 150.0;

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
                last_shoulder_value += calculate_shoulder_velocity(&gamepad);
                last_shoulder_value = clamp(&last_shoulder_value, 50.0, 140.0);


                last_elbow_value += calculate_elbow_velocity(&gamepad);
                last_elbow_value = clamp(&last_elbow_value, 50.0, 140.0);

                last_claw_l_position += calculate_claw_velocity(&gamepad, Button::DPadLeft, Button::DPadRight);
                last_claw_l_position = clamp(&last_claw_l_position, 35.0, 75.0);

                last_claw_r_position += calculate_claw_velocity(&gamepad, Button::DPadUp, Button::DPadDown);
                last_claw_r_position = clamp(&last_claw_r_position, 90.0, 150.0);



                let mut unconverted_values = [
                    -127,
                    1,
                    90, //base_speed
                    last_shoulder_value as i32, //shoulder position
                    last_elbow_value as i32, //elbow position
                    calculate_claw_velocity(&gamepad, Button::LeftTrigger, Button::RightTrigger) as i32, //wristtheta speed
                    calculate_claw_velocity(&gamepad, Button::LeftTrigger2, Button::RightTrigger2) as i32, //wrist phi speed 
                    last_claw_l_position as i32, //claw l pos
                    last_claw_r_position as i32, //claw r pos
                    255, //dummy value, will be overwritten with the hash
                ];
                //this overwrite is nice so that we can reference elements of buf instead of copyf
                //pasting format_axis_to_send calls twice.
                let mut hash: i32 = unconverted_values[2]
                    + unconverted_values[3]
                    + unconverted_values[4]
                    + unconverted_values[5]
                    + unconverted_values[6]
                    + unconverted_values[7]
                    + unconverted_values[8];
                hash /= 7;
                unconverted_values[unconverted_values.len() - 1] = hash;
                println!("Hash is {:}", hash);
                println!("{:#?}", unconverted_values);

                let mapped_vec: Vec<u8> = unconverted_values
                    .iter()
                    .map(|&x| to_twos_complement(x))
                    .collect();
                let converted_message: [u8; 10] = mapped_vec.try_into().unwrap_or([0; 10]);

                println!("{:#?}", converted_message);

                socket
                    .send_to(
                        &converted_message,
                        format!("{}:{}", arduino_ip, arduino_port),
                    )
                    .expect("Couldn't send data");
            }
        }
    }
}
