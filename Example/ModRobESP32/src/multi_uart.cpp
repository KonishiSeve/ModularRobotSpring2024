#include "multi_uart.h"

// ===== Configuration functions =====

MultiUART::MultiUART(void) {
    //initialize the timer, 1 tick per us
    MultiUART::Timer = timerBegin(0, 80, true);
}

bool MultiUART::set_transmitter(uint8_t tx_pin) {
    //initialize TX pin
    MultiUART::tx_pin = tx_pin;
    pinMode(tx_pin, OUTPUT);
    digitalWrite(tx_pin, HIGH);
    return true;
}

bool MultiUART::add_receiver(uint8_t pin) {
    //setup a pin as software UART receiver
    if(MultiUART::receivers_size < MAX_RECEIVERS) {
        MultiUART::receivers[MultiUART::receivers_size++].pin = pin;
        pinMode(pin, INPUT_PULLDOWN);
        return true;
    }
    return false;
}

uint8_t MultiUART::xfer(uint32_t baud, uint32_t timeout_ms, uint16_t tx_byte, uint32_t tx_delay_us) {
    // === initialize all the states ===
    digitalWrite(MultiUART::tx_pin, HIGH);
    uint8_t idle_counter = 0;
    for(int i=0;i<MultiUART::receivers_size;i++) {
        //for each port, look if it has a module connected to it
        if(digitalRead(MultiUART::receivers[i].pin)) {
            MultiUART::receivers[i].uart_state = IDLE;
            MultiUART::receivers[i].value = 0;
            MultiUART::receivers[i].value_index = 0;
            idle_counter++;
        }
        else {
            MultiUART::receivers[i].uart_state = DISCONNECTED;
        }
    }

    // === Tranceiving phase ===
    timerRestart(MultiUART::Timer);
    volatile uint32_t timer_value = (uint32_t)timerReadMicros(MultiUART::Timer);
    //timer value of when to stop
    uint32_t timeout_time = timer_value + timeout_ms*1000;
    //timer value of when to start transmitting (after tx_delay_us)
    uint32_t tx_next_write_time = timer_value + tx_delay_us;
    tx_byte = (tx_byte << 1) | (1 << 9); //adding start bit and stop bit
    uint8_t tx_byte_index = 0;

    uint32_t us_per_bit = 1e6/(baud);

    //Run the loop unit all connected receivers got something and the transmitter is done (or timeout)
    while(timer_value < timeout_time && (idle_counter > 0 || tx_byte_index < 10)) {
        timer_value = (uint32_t)timerReadMicros(MultiUART::Timer);
        //receiving
        for(int i=0;i<MultiUART::receivers_size;i++) {
            //start receiving
            if(MultiUART::receivers[i].uart_state==IDLE && digitalRead(MultiUART::receivers[i].pin)==LOW) {
                MultiUART::receivers[i].uart_state = RUNNING;
                MultiUART::receivers[i].next_sample_time = timer_value + us_per_bit + us_per_bit/2; //ignore start bit and sample in the middle of the next bit
            }
            //sample a bit
            else if (MultiUART::receivers[i].uart_state==RUNNING && MultiUART::receivers[i].next_sample_time <= timer_value) {
                MultiUART::receivers[i].value += (digitalRead(MultiUART::receivers[i].pin)&(1)) << MultiUART::receivers[i].value_index++;
                MultiUART::receivers[i].next_sample_time += us_per_bit;
                if(MultiUART::receivers[i].value_index >=8) {
                    MultiUART::receivers[i].uart_state = DONE;
                    idle_counter--;
                }
            }
        }
        //transmitting
        if(tx_next_write_time <= timer_value && tx_byte_index < 10) {
            digitalWrite(MultiUART::tx_pin, (tx_byte&(1 << tx_byte_index)) >> tx_byte_index);
            tx_byte_index++;
            tx_next_write_time += us_per_bit;
        }
    }
    return 0;
}

//Get the last value read by a receiver (returns 0 if the value is not valid)
uint8_t MultiUART::get_value(uint8_t index) {
    if(MultiUART::receivers[index].uart_state == DONE) {
        return MultiUART::receivers[index].value;
    }
    return 0;
}