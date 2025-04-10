#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <linux/uinput.h>
#include <unistd.h>
#include <sys/time.h>

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

    // Release
    gettimeofday(&ev.time, NULL);
    ev.type = EV_KEY;
    ev.code = keycode;
    ev.value = 0;
    write(fd, &ev, sizeof(ev));

    // Sync again
    ev.type = EV_SYN;
    ev.code = SYN_REPORT;
    ev.value = 0;
    write(fd, &ev, sizeof(ev));
}

void press_arrows(int fd, int left, int right, int up, int down) {
    for (int i = 0; i < left; i++) press_key(fd, KEY_LEFT);
    for (int i = 0; i < right; i++) press_key(fd, KEY_RIGHT);
    for (int i = 0; i < up; i++) press_key(fd, KEY_UP);
    for (int i = 0; i < down; i++) press_key(fd, KEY_DOWN);
}

int main() {
    int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    // Enable events and keys
    ioctl(fd, UI_SET_EVBIT, EV_KEY);
    ioctl(fd, UI_SET_KEYBIT, KEY_LEFT);
    ioctl(fd, UI_SET_KEYBIT, KEY_RIGHT);
    ioctl(fd, UI_SET_KEYBIT, KEY_UP);
    ioctl(fd, UI_SET_KEYBIT, KEY_DOWN);

    // Create virtual device
    struct uinput_user_dev uidev = {0};
    snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "my-virtual-keyboard");
    uidev.id.bustype = BUS_USB;
    uidev.id.vendor = 0x1234;
    uidev.id.product = 0xfedc;
    uidev.id.version = 1;

    write(fd, &uidev, sizeof(uidev));
    ioctl(fd, UI_DEV_CREATE);

    sleep(1); // wait for setup

    // Example: press Left 1 time, Right 2 times, Up 0 times, Down 1 time
    press_arrows(fd, 0, 0, 0, 5);

    sleep(1);
    ioctl(fd, UI_DEV_DESTROY);
    close(fd);
    return 0;
}
