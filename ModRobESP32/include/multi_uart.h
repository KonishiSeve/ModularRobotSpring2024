#ifndef MULTI_UART_H
#define MULTI_UART_H


#include <Arduino.h>
#include <HardwareSerial.h>

#define MAX_RECEIVERS       16
#define UART_TX_PIN         19
#define UART_RX_PIN         18

enum UART_STATE {
  DISCONNECTED,
  IDLE,
  RUNNING,
  DONE
};

typedef struct UartReceiver {
  //variables
  uint32_t next_sample_time;
  UART_STATE uart_state = DISCONNECTED;
  uint8_t value = 0;
  uint8_t value_index = 0;

  //configuration
  uint8_t pin;
  uint32_t baud;

} UartReceiver;


class MultiUART {
    private:
        UartReceiver receivers[MAX_RECEIVERS];
        uint8_t receivers_size = 0;

        hw_timer_t *Timer = NULL;
        uint8_t tx_pin;

    public:
        //configuration functions
        MultiUART(void);
        bool set_transmitter(uint8_t tx_pin);
        bool add_receiver(uint8_t pin);

        //communication functions
        uint8_t xfer(uint32_t baud, uint32_t timeout_ms,
                     uint16_t tx_byte, uint32_t tx_delay_us); //Rx and Tx over UART
            //This function will first start all receivers and send the tx_byte after tx_delay_us microseconds

        uint8_t get_value(uint8_t index);

};















#endif