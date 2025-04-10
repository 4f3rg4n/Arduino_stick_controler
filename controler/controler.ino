#include <Arduino_JSON.h>

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

stick_metadata stick_1 = {A0, A1, 2};
stick_metadata stick_2 = {A2, A3, 3};

stick_data get_stick_data(stick_metadata stick) {
  stick_data data;
  int val_x = analogRead(stick.pin_x);
  int val_y = analogRead(stick.pin_y);
  int button_state = digitalRead(stick.pin_button);

  data.x = map(val_x, 0, 1023, -1024, 1024);
  data.y = map(val_y, 0, 1023, -1024, 1024);
  data.button = button_state == LOW ? 1 : 0;

  return data;
}

void setup() {
  Serial.begin(9600);

  pinMode(stick_1.pin_button, INPUT_PULLUP);
  pinMode(stick_2.pin_button, INPUT_PULLUP);
}

void loop() {
  JSONVar data;

  stick_data stick_1_data = get_stick_data(stick_1);
  stick_data stick_2_data = get_stick_data(stick_2);

  data["x1"] = stick_1_data.x;
  data["y1"] = stick_1_data.y;
  data["buttonState1"] = stick_1_data.button;

  data["x2"] = stick_2_data.x;
  data["y2"] = stick_2_data.y;
  data["buttonState2"] = stick_2_data.button;

  Serial.println(JSON.stringify(data));

}
