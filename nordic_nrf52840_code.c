#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>
#include <drivers/uart.h>
#include <string.h>
#include <net/http_client.h>

#define UART_DEV_LABEL "UART_1"  
#define UART_XIAO_DEV_LABEL "UART_2"  // New UART for XIAO ESP32S3 Sense
#define SERVO_PIN_1 DT_ALIAS_SERVO1_GPIOS_PIN
#define SERVO_PIN_2 DT_ALIAS_SERVO2_GPIOS_PIN
#define SERVO_PIN_3 DT_ALIAS_SERVO3_GPIOS_PIN
#define SPEAKER_PIN DT_ALIAS_SPEAKER_GPIOS_PIN

#define MAX_BOX_DATA_LENGTH 50
#define UART_BUFFER_SIZE 256
#define GPT_API_URL "http://your_gpt_api_url_here"
#define API_KEY "your_api_key_here"

char box1[MAX_BOX_DATA_LENGTH] = {0};
char box2[MAX_BOX_DATA_LENGTH] = {0};
char box3[MAX_BOX_DATA_LENGTH] = {0};

char uart_buffer[UART_BUFFER_SIZE];
char uart_xiao_buffer[UART_BUFFER_SIZE];  // Buffer for XIAO ESP32S3 Sense UART
const struct device *uart_dev;
const struct device *uart_xiao_dev;  // UART device for XIAO ESP32S3 Sense
const struct device *gpio_dev;

void init_peripherals() {
    uart_dev = device_get_binding(UART_DEV_LABEL);
    uart_xiao_dev = device_get_binding(UART_XIAO_DEV_LABEL);  // Initialize second UART
    gpio_dev = device_get_binding(DT_LABEL(DT_NODELABEL(gpio0)));

    if (!device_is_ready(gpio_dev)) {
        printk("GPIO device not found!\n");
        return;
    }

    gpio_pin_configure(gpio_dev, SERVO_PIN_1, GPIO_OUTPUT);
    gpio_pin_configure(gpio_dev, SERVO_PIN_2, GPIO_OUTPUT);
    gpio_pin_configure(gpio_dev, SERVO_PIN_3, GPIO_OUTPUT);
    gpio_pin_configure(gpio_dev, SPEAKER_PIN, GPIO_OUTPUT);
}

void parse_box_data(const char *data) {
    sscanf(data, "box1=%49[^;];box2=%49[^;];box3=%49[^;];", box1, box2, box3);
    printk("Parsed box data: box1=%s, box2=%s, box3=%s\n", box1, box2, box3);
}

void generate_and_play_audio(const char *box_data) {
    char request_buffer[256];
    sprintf(request_buffer, "{\"prompt\": \"Details for %s\", \"max_tokens\": 100}", box_data);

    struct http_request req = {
        .method = HTTP_POST,
        .url = GPT_API_URL,
        .header = "Content-Type: application/json\r\nAuthorization: Bearer " API_KEY "\r\n",
        .body = request_buffer,
    };

    struct http_response res;
    int ret = http_client_post(&req, &res);

    if (ret == 0 && res.status_code == 200) {
        strncpy(uart_buffer, res.body, sizeof(uart_buffer) - 1);
        uart_buffer[sizeof(uart_buffer) - 1] = '\0';
        play_audio(uart_buffer);
    } else {
        printk("Error: %d\n", res.status_code);
    }
}

void play_audio(const char *text) {
    printk("Playing audio: %s\n", text);
    gpio_pin_set(gpio_dev, SPEAKER_PIN, 1);
    k_sleep(K_MSEC(2000));
    gpio_pin_set(gpio_dev, SPEAKER_PIN, 0);
}

void control_servo(const char *text) {
    if (strstr(text, box1) != NULL) {
        gpio_pin_set(gpio_dev, SERVO_PIN_1, 1);
        generate_and_play_audio(box1);
    } else if (strstr(text, box2) != NULL) {
        gpio_pin_set(gpio_dev, SERVO_PIN_2, 1);
        generate_and_play_audio(box2);
    } else if (strstr(text, box3) != NULL) {
        gpio_pin_set(gpio_dev, SERVO_PIN_3, 1);
        generate_and_play_audio(box3);
    } else {
        printk("No matching box data found.\n");
    }
}

void uart_thread_fn(void) {
    while (1) {
        int len = uart_poll_in(uart_dev, uart_buffer, sizeof(uart_buffer) - 1);
        if (len > 0) {
            uart_buffer[len] = '\0';
            if (strstr(uart_buffer, "box1=") != NULL) {
                parse_box_data(uart_buffer);
            } else {
                control_servo(uart_buffer);
            }
        }
        k_sleep(K_MSEC(500));
    }
}

void uart_xiao_thread_fn(void) {
    while (1) {
        int len = uart_poll_in(uart_xiao_dev, uart_xiao_buffer, sizeof(uart_xiao_buffer) - 1);
        if (len > 0) {
            uart_xiao_buffer[len] = '\0';
            printk("Received from XIAO ESP32S3 Sense: %s\n", uart_xiao_buffer);
            control_servo(uart_xiao_buffer);
        }
        k_sleep(K_MSEC(500));
    }
}

K_THREAD_DEFINE(uart_thread, 1024, uart_thread_fn, NULL, NULL, NULL, 7, 0, 0);
K_THREAD_DEFINE(uart_xiao_thread, 1024, uart_xiao_thread_fn, NULL, NULL, NULL, 7, 0, 0);

void main() {
    init_peripherals();
    while (1) {
        k_sleep(K_MSEC(1000));
    }
}
