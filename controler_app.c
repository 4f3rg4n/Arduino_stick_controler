#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <jansson.h>
#include <X11/Xlib.h>
#include <fcntl.h>
#include <linux/uinput.h>
#include <sys/time.h>

#define DEVICE_PATH "/dev/stick-data"
#define BUFFER_SIZE 256
#define MODULE_NAME "stick-controler"


/* General data structs */

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


// X11 variables
static Display *display;
static Window root;

// Uinput variables for keyboard emulation
int fd;

void print_log(const char* action, const char* message);
char* read_from_device(const char* device_path);
controler parse_data(const char* data, json_error_t* j_err);
mouse_pose get_mouse_pose();

/*
This function reads data from stick and returns it in a struct.
Input:
    stick - struct containing the pin numbers for the x, y, and button inputs of the stick.
Output: struct containing the x and y values of the stick, and the state of the button.
*/
void print_log(const char* action, const char* message) {
    printf("[%s] %s: %s\n", MODULE_NAME, action, message);
}

/*
This function reads data from the device file and returns it as a string.
Input:
    device_path - path to the device file
Output: string containing the data read from the device file.
*/
char* read_from_device(const char* device_path) {
    FILE *file;
    char *buffer;
    size_t bytes_read = 0;
    int ch;

    // allocate memory for the buffer to read data
    buffer = (char*)malloc(BUFFER_SIZE);
    if (!buffer) {
        print_log("malloc", "allocation failed!");
        return NULL;
    }

    // open the device file 
    file = fopen(device_path, "r");
    if (!file) {
        print_log("open", "device file not found!");
        free(buffer);
        return NULL;
    }

    // read data from the device file
    while ((ch = fgetc(file)) != EOF && ch != '\n' && bytes_read < BUFFER_SIZE - 1) {
        buffer[bytes_read++] = (char)ch;
    }

    buffer[bytes_read] = '\0';

    // log
    print_log("read", buffer);

    fclose(file);

    return buffer;
}

/*
This function parses the JSON data and returns a controler struct.
Input:
    data - JSON string containing the data to be parsed
    j_err - pointer to a json_error_t struct to store any parsing errors
Output: controler struct containing the parsed data.
*/
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


    /*************\
    |* Stick two *|
    \*************/

    // extract x2
    json_t* x2_json = json_object_get(json_data, "x2");
    parsed_data.stick_2.x = (int)json_integer_value(x2_json);

    // extract y2
    json_t* y2_json = json_object_get(json_data, "y2");
    parsed_data.stick_2.y = (int)json_integer_value(y2_json);

    // extract buttonState2
    json_t* button_state2_json = json_object_get(json_data, "buttonState2");
    parsed_data.stick_2.button = (int)json_integer_value(button_state2_json);


    // clean up
    json_decref(json_data);

    return parsed_data;
}

/*
This function gets the current mouse position and returns it in a struct.
Input: none
Output: struct containing the x and y coordinates of the mouse.
*/
mouse_pose get_mouse_pose(){
    mouse_pose pose;
    Window child;
    unsigned int mask;
    int x, y;

    // get the current mouse position
    XQueryPointer(display, root, &child, &child, &pose.x, &pose.y, &x, &y, &mask);

    return pose;
}

/*
This function moves the mouse to the specified x and y coordinates.
Input:
    x - x coordinate to move the mouse to
    y - y coordinate to move the mouse to
Output: none
*/
void move_mouse(int x, int y) {
    XWarpPointer(display, None, root, 0, 0, 0, 0, x, y);
    XFlush(display);  // Flush the display to apply the move
}

/*
This function handles mouse movement based on the stick data.
Input:
    stick - struct containing the x and y values of the stick
Output: none
*/
void handle_mouse(stick_data stick) {
    // get the current mouse position
    mouse_pose pose = get_mouse_pose();

    // adjust the stick values to control the mouse movement
    if (stick.x * stick.x < 2500) {
        stick.x = 0;
    }
    if (stick.y * stick.y < 2500) {
        stick.y = 0;
    }
    stick.x /= -20;
    stick.y /= 20;

    // move
    move_mouse(pose.x + stick.x, pose.y + stick.y);
}

/*
This function presses a key on the virtual keyboard.
Input:
    fd - file descriptor for the uinput device
    keycode - keycode of the key to be pressed
Output: none
*/
void press_key(int fd, int keycode) {
    struct input_event ev = {0};
    gettimeofday(&ev.time, NULL);

    // Press
    ev.type = EV_KEY;
    ev.code = keycode;
    ev.value = 1;
    write(fd, &ev, sizeof(ev));

    // Sync
    ev.type = EV_SYN;
    ev.code = SYN_REPORT;
    ev.value = 0;
    write(fd, &ev, sizeof(ev));
}

/*
This function releases a key on the virtual keyboard.
Input:
    fd - file descriptor for the uinput device
    keycode - keycode of the key to be released
Output: none
*/
void release_key(int fd, int keycode){
    struct input_event ev = {0};
    gettimeofday(&ev.time, NULL);

    // Release
    ev.type = EV_KEY;
    ev.code = keycode;
    ev.value = 0;
    write(fd, &ev, sizeof(ev));

    // Sync
    ev.type = EV_SYN;
    ev.code = SYN_REPORT;
    ev.value = 0;
    write(fd, &ev, sizeof(ev));
}

/*
This function handles key presses based on the stick data.
Input:
    stick - struct containing the x and y values of the stick
Output: none
*/
void handle_keys(stick_data stick) {
    // adjust the stick values to control the key presses
    if (stick.x * stick.x < 2500) {
        stick.x = 0;
    }
    if (stick.y * stick.y < 2500) {
        stick.y = 0;
    }

    // handle button presses (Right & Left)
    if (stick.x < 0) {
        release_key(fd, KEY_LEFT);
        press_key(fd, KEY_RIGHT);
    } else if (stick.x > 0) {
        release_key(fd, KEY_RIGHT);
        press_key(fd, KEY_LEFT);
    }  else if (stick.x == 0) {
        release_key(fd, KEY_LEFT);
        release_key(fd, KEY_RIGHT);
    }

    // handle button presses (Up & Down)
    if (stick.y > 0) {
        release_key(fd, KEY_UP);
        press_key(fd, KEY_DOWN);
    } else if (stick.y < 0) {
        release_key(fd, KEY_DOWN);
        press_key(fd, KEY_UP);
    }
    else if (stick.y == 0) {
        release_key(fd, KEY_UP);
        release_key(fd, KEY_DOWN);
    }
}

int main() {
    json_error_t j_err;
    controler data;
    char *device_data;

    print_log("init", "controler-app started!");

    display = XOpenDisplay(NULL);

    // get the root window
    root = DefaultRootWindow(display);


    /**********************\
    |* init keyboard data *|
    \**********************/

    fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    // enable events and keys
    ioctl(fd, UI_SET_EVBIT, EV_KEY);
    ioctl(fd, UI_SET_KEYBIT, KEY_LEFT);
    ioctl(fd, UI_SET_KEYBIT, KEY_RIGHT);
    ioctl(fd, UI_SET_KEYBIT, KEY_UP);
    ioctl(fd, UI_SET_KEYBIT, KEY_DOWN);

    // create virtual device
    struct uinput_user_dev uidev = {0};
    snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "virt-keyboard");
    uidev.id.bustype = BUS_USB;
    uidev.id.vendor = 0x1234;
    uidev.id.product = 0xfedc;
    uidev.id.version = 1;

    write(fd, &uidev, sizeof(uidev));
    ioctl(fd, UI_DEV_CREATE);

    /* 
    *   controler loop: 
    *   1. read data from the device
    *   2. parse the data
    *   3. handle the keys and mouse
    *   4. free the data
    *   5. repeat
    */
    while(1==1) {
        device_data = read_from_device(DEVICE_PATH);
        data = parse_data(device_data, &j_err);
        printf("Data read from %s: %s\n", DEVICE_PATH, device_data);
        handle_keys(data.stick_2);
        handle_mouse(data.stick_1);
        free(device_data);
    }

    print_log("exit", "controler-app finished");

    return 0;
}
