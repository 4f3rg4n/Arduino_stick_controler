#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <jansson.h>
#include <X11/Xlib.h>
#include <unistd.h>

#define DEVICE_PATH "/dev/stick-data"
#define BUFFER_SIZE 256
#define MODULE_NAME "stick-controler"

typedef struct {
    int x;
    int y;
} mouse_pose;

typedef struct {
    int x;
    int y;
    int button;
} stick_data;

typedef struct {
    stick_data stick_1;
    stick_data stick_2;
} controler;

static Display *display;
static Window root;

void print_log(const char* action, const char* message);
char* read_from_device(const char* device_path);
controler parse_data(const char* data, json_error_t* j_err);
mouse_pose get_mouse_pose();

void print_log(const char* action, const char* message) {
    printf("[%s] %s: %s\n", MODULE_NAME, action, message);
}

char* read_from_device(const char* device_path) {
    FILE *file;
    char *buffer;
    size_t bytes_read = 0;
    int ch;

    // Allocate memory for the buffer to read data
    buffer = (char*)malloc(BUFFER_SIZE);
    if (!buffer) {
        print_log("malloc", "allocation failed!");
        return NULL;
    }

    // Open the device file (assumes it exists and is accessible)
    file = fopen(device_path, "r");
    if (!file) {
        print_log("open", "device file not found!");
        free(buffer);
        return NULL;
    }

    // Read data from the device file until we encounter a newline or EOF
    while ((ch = fgetc(file)) != EOF && ch != '\n' && bytes_read < BUFFER_SIZE - 1) {
        buffer[bytes_read++] = (char)ch;
    }

    // Null-terminate the buffer
    buffer[bytes_read] = '\0';

    // print_log device file data
    print_log("read", buffer);

    // Clean up and close the file
    fclose(file);

    return buffer;
}

controler parse_data(const char* data, json_error_t* j_err) {
    controler parsed_data = {0};  

    // parse the JSON data
    json_t* json_data = json_loads(data, 0, j_err);

    /*************\
    |* Stick one *|
    \*************/

    // extract x1
    json_t* x1_json = json_object_get(json_data, "x1");
    parsed_data.stick_1.x = (int)json_integer_value(x1_json);

    // extract y1
    json_t* y1_json = json_object_get(json_data, "y1");
    parsed_data.stick_1.y = (int)json_integer_value(y1_json);

    // extract buttonState1
    json_t* button_state1_json = json_object_get(json_data, "buttonState1");
    parsed_data.stick_1.button = (int)json_integer_value(button_state1_json);

    // clean up
    json_decref(json_data);

    return parsed_data;
}

mouse_pose get_mouse_pose(){
    mouse_pose pose;

    int x, y;


    // Get the current mouse position
    Window child;
    unsigned int mask;
    XQueryPointer(display, root, &child, &child, &pose.x, &pose.y, &x, &y, &mask);

    return pose;
}

void move_mouse(int x, int y) {
    XWarpPointer(display, None, root, 0, 0, 0, 0, x, y);
    XFlush(display);  // Flush the display to apply the move
}

void handle_mouse(stick_data stick) {
    mouse_pose pose = get_mouse_pose();
    if (stick.x * stick.x < 2500) {
        stick.x = 0;
    }
    if (stick.y * stick.y < 2500) {
        stick.y = 0;
    }
    stick.x /= -20;
    stick.y /= 20;

    move_mouse(pose.x + stick.x, pose.y + stick.y);
    //sleep(0.1);
}


int main() {
    print_log("init", "Program started");

    display = XOpenDisplay(NULL);
    // Get the root window
    root = DefaultRootWindow(display);


    // Read data from the device
    json_error_t j_err;
    controler data;
    char *device_data;
    while(1==1) {
        device_data = read_from_device(DEVICE_PATH);
        data = parse_data(device_data, &j_err);
        printf("Data read from %s: %s\n", DEVICE_PATH, device_data);
        handle_mouse(data.stick_1);
        free(device_data);
    }

    // Clean up

    print_log("exit", "Program finished");

    return 0;
}
