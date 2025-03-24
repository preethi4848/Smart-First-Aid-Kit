/dts-v1/; 
/ {
    aliases {
        uart1 = &uart1;  // Alias for UART1
        uart2 = &uart2;  // Alias for UART2 (used for XIAO ESP32S3 Sense)
        servo1 = &gpio0;  // Alias for GPIO used for Servo 1
        servo2 = &gpio0;  // Alias for GPIO used for Servo 2
        servo3 = &gpio0;  // Alias for GPIO used for Servo 3
        speaker = &gpio0; // Alias for GPIO used for Speaker
    };

    chosen {
        zephyr,console = &uart0;
    };

    gpio0: gpio@50000000 {
        compatible = "nordic,nrf-gpio";
        status = "okay";
        servo1-gpios = <&gpio0 13 GPIO_ACTIVE_HIGH>; // Pin P0.13 for Servo 1
        servo2-gpios = <&gpio0 14 GPIO_ACTIVE_HIGH>; // Pin P0.14 for Servo 2
        servo3-gpios = <&gpio0 15 GPIO_ACTIVE_HIGH>; // Pin P0.15 for Servo 3
        speaker-gpios = <&gpio0 16 GPIO_ACTIVE_HIGH>; // Pin P0.16 for Speaker
    };

    uart1: uart@40028000 {  // UART1 configuration
        compatible = "nordic,nrf-uart";
        current-speed = <115200>;
        status = "okay";
        tx-pin = <6>;  // TX on pin P0.06
        rx-pin = <8>;  // RX on pin P0.08
    };

    uart2: uart@40029000 {  // UART2 configuration for XIAO ESP32S3 Sense
        compatible = "nordic,nrf-uart";
        current-speed = <115200>;
        status = "okay";
        tx-pin = <20>;  // TX on pin P0.20
        rx-pin = <22>;  // RX on pin P0.22
    };
};
