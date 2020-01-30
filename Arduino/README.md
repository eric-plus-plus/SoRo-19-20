# Arduino Code

## Testing Tools

### servo_test.ino

This program is good for calibrating/testing a single talon (our motor controller).
To use it, change `#define PIN_SERVO 31` to whatever Arduino pin you're connecting the signal wire to, upload the code, and then open the serial monitor.
When you send a number to the Arduino it will write it to the servo.
When calibrating, use the range 10 to 170 instead of the full 0 to 180 because it freaks out the talons. 90 degrees will be still.

### rover_arm_test.ino

This is similar to *servo_test.ino* but made specifically for the rover's arm. It works the same way, except you can select which angle you want to drive.
What's new in this one is you can just spam the enter key in the console if you want it to stop moving in case you accidentally type 9 instead of 90.

## Rover Programs

### wheel_controller.ino

This should be uploaded to the Arduino Nano in the e-box. The macro `#define DEBUG_MODE 0` can enable serial debugging messages. Don't use this when running the rover because it can cause the Arduino to randomly reset.

### control_arm.ino

For the control arm's Arduino Mega (with ethernet shield).

### rover_arm.ino

For the rover arm's Arduino Mega (with ethernet shield). Incomplete, currently behind in updates compared to *rover_arm_test.ino*
