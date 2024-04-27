#include "multi_uart.h"

// ===== Configuration functions =====

MultiUART::MultiUART(void) {
    //initialize the timer
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
    if(MultiUART::receivers_size < MAX_RECEIVERS) {
        MultiUART::receivers[MultiUART::receivers_size++].pin = pin;
        pinMode(pin, INPUT);
        return true;
    }
    return false;
}

uint8_t MultiUART::xfer(uint32_t baud, uint32_t timeout_ms, uint16_t tx_byte, uint32_t tx_delay_us) {
    //DEBUG
    #define DEBUG_PIN   25
    pinMode(DEBUG_PIN, OUTPUT);
    digitalWrite(DEBUG_PIN, LOW);

    //initialize all the states
    Serial.println("In Xfer");
    digitalWrite(MultiUART::tx_pin, HIGH);
    uint8_t idle_counter = 0;
    for(int i=0;i<MultiUART::receivers_size;i++) {
        //detect if each port has a neighbour connected or not
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

    Serial.printf("[Transceiving] IDLE counter : %d \n\r", idle_counter);
    //Tranceiving phase
    timerRestart(MultiUART::Timer);
    volatile uint32_t timer_value = (uint32_t)timerReadMicros(MultiUART::Timer);

    uint32_t timeout_time = timer_value + timeout_ms*1000;
    //transmitter variables
    uint32_t tx_next_write_time = timer_value + tx_delay_us;
    tx_byte = (tx_byte << 1) | (1 << 9); //adding start bit and stop bit
    uint8_t tx_byte_index = 0;

    uint32_t us_per_bit = 1e6/(baud);

    digitalWrite(DEBUG_PIN, HIGH);

    while(timer_value < timeout_time && (idle_counter > 0 || tx_byte_index < 10)) { //(timer_value < timeout_time && idle_counter > 0)
        timer_value = (uint32_t)timerReadMicros(MultiUART::Timer);
        //receiving
        for(int i=0;i<MultiUART::receivers_size;i++) {
            //start the receiving
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
    Serial.println(MultiUART::receivers[0].value);
    Serial.println(MultiUART::receivers[1].value);
    Serial.println("[Transceiving] done");
    return 0;
}