#include <Arduino_JSON.h>


/* General data structs */

struct stick_metadata {
  int pin_x;
  int pin_y;
  int pin_button;
};

struct stick_data {
  int x;
  int y;
  int button;
};

/* Sticks data */
stick_metadata stick_1 = {A0, A1, 2};
stick_metadata stick_2 = {A2, A3, 3};

/*
This function reads data from stick and returns it in a struct.
Input:
  stick - struct containing the pin numbers for the x, y, and button inputs of the stick.
Output: struct containing the x and y values of the stick, and the state of the button.
*/
stick_data get_stick_data(stick_metadata stick) {
  stick_data data;

  // read the x and y values from the analog pins
  int val_x = analogRead(stick.pin_x);
  int val_y = analogRead(stick.pin_y);
  int button_state = digitalRead(stick.pin_button);

  // map the x and y values to -1024 to 1024
  data.x = map(val_x, 0, 1023, -1024, 1024);
  data.y = map(val_y, 0, 1023, -1024, 1024);
  data.button = button_state == LOW ? 1 : 0;

  return data;
}

/*
This function reads data from the stick and returns it in a JSON object.
Input:
  stick - struct containing the pin numbers for the x, y, and button inputs of the stick.
Output: JSON object containing the x and y values of the stick, and the state of the button.
*/
void setup() {
  Serial.begin(9600);

  // initialize the pins for the sticks
  pinMode(stick_1.pin_button, INPUT_PULLUP);
  pinMode(stick_2.pin_button, INPUT_PULLUP);
}

/*
This function runs in a loop and reads data from the sticks.
Input: none
Output: none
*/
void loop() {
  JSONVar data;

  // read the data from the sticks
  stick_data stick_1_data = get_stick_data(stick_1);
  stick_data stick_2_data = get_stick_data(stick_2);

  // create a JSON object and add the data to it
  data["x1"] = stick_1_data.x;
  data["y1"] = stick_1_data.y;
  data["buttonState1"] = stick_1_data.button;

  // add the data to the JSON object 
  data["x2"] = stick_2_data.x;
  data["y2"] = stick_2_data.y;
  data["buttonState2"] = stick_2_data.button;

  // print the JSON object to the serial monitor
  Serial.println(JSON.stringify(data));
}
