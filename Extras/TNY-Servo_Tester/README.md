# TNY-Servo Tester

This project is for people building their own TNY Servomotors.

It provides a simple way to ensure the servomotors work correctly before they are installed in the robot.

---

> [!NOTE]  
> The code is dirty. It is not meant to be used as a reference for good coding practices, but rather as a quick and dirty solution to test the servomotors.

## Hardware

The tester is made to work on an ESP32-S3 Dev Board, here's the pinout:

| ESP32-S3 Pin | Function                  |
| ------------ | ------------------------- |
| GPIO 48      | LED Control (WS2812)      |
| GPIO 42      | Servo Signal (PWM)        |
| GPIO 02      | Servo Feedback (ADC1_CH1) |

While powering up a motor from the ESP32-S3 Dev Board is not recommended, it's acceptable to power the servo from the board's **5V** & **GND** pins for testing purposes. The servo will only draw a small amount of current during the test (as long as you don't block its movement), so it should be safe to power it from the board.

For easier testing, connect a 4-Pin Dupont 2.54mm header to the ESP32-S3 Dev Board, and then connect the TNY-Servo's JST-XH connector to the header. This will allow you to easily connect and disconnect the servo for testing.

> [!WARNING]  
> Make sure to connect the **GROUND** pin of the servo first (slightly tilt the connector when plugging it in) to avoid damaging the servo or the ESP32-S3 Dev Board. If power is applied to the servo before the ground connection is made, it can cause the current to come back through the servo's internal circuitry. 

## Software

The code is written in C++ using the ESP-IDF framework. It uses the LEDC peripheral to generate the PWM signal for the servo and the ADC to read the feedback from the servo.

## Usage

1. Connect the servo to the ESP32-S3 Dev Board according to the pinout above.
2. Power on or reset the ESP32-S3 Dev Board.
3. Wait for the code to run and observe the LED color and the servo movement.
4. For more detailed information on the test, check the serial output using a serial monitor.

### Testing sequence

The testing sequence is simple :
1. The servo will move to its minimum position (0°) and wait a little bit for the ADC reading to stabilize.
2. The servo will then move to its maximum position (180°) and wait again for the ADC reading to stabilize.
3. Finally, the servo will move back to its center position (90°) and wait for the ADC reading to stabilize.

At the end of the test, the code will check if the ADC readings are within the expected range, noise & order for the servo to be considered working correctly. Even if the servo is moving during the test, the feedback could still be incorrect, so wait for the test to finish and check the LED color to know if the servo is fully functional or not.

### LED Color Indication

While testing, the LED will display in **BLUE**. This indicates that the tester is running and the servo is being tested.

At the end of the test, the LED will change to **GREEN** if the servo is working correctly, or **RED** if there is an issue with the servo.

For more information on the test results, check the serial output.