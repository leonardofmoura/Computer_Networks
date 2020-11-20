#include <stdio.h>

#include "state_machine.h"
#include "protocol_macros.h"

enum State process_state(enum State current_state, unsigned char received,unsigned char* msg, int *current_byte) {
    switch (current_state) {
    case INIT:
        if (received == FLAG) {
            msg[*current_byte] = received;
            (*current_byte)++;
            return FLG;
        }
        else {
            return INIT;
        }

    case FLG: 
        if (received == FLAG) {
            *current_byte = 1;
            return FLG;
        }
        else if (received == SENDER || received == RECEIVER) {
            msg[*current_byte] = received;
            (*current_byte)++;
            return ADDR;
        }
        else {
            return INIT;
        }

    case ADDR:
        if (received == FLAG) {
            *current_byte = 1;
            return FLG;
        }
        else if (received == SET     || received == DISC    || received == UA || // control
                 received == (RR|R0) || received == (RR|R1) || received == (REJ|R1) || received == (REJ|R0) || // received answer
                 received == I0      || received == I1) { // received data
            msg[*current_byte] = received;
            (*current_byte)++;
            return CTRL;
        }
        else {
            return INIT;
        }
    
    case CTRL:
        if (received == FLAG) {
            *current_byte = 1;
            return FLG;
        }
        else if ((msg[1] ^ msg[2]) == received) { 
            msg[*current_byte] = received;
            (*current_byte)++;
            return PROT;
        }
        else {
            return INIT;
        }

    case PROT:
        if (received == FLAG) {
            msg[*current_byte] = received;
            (*current_byte)++;
            return FINAL;
        }
        else {
            msg[*current_byte] = received;
            (*current_byte)++;
            return DATA;
        }

    case DATA:
        if (received == FLAG) {
            msg[*current_byte] = received;
            (*current_byte)++;
            return FINAL;
        }
        else {
            msg[*current_byte] = received;
            (*current_byte)++;
            return DATA;
        }
    default:
        printf("Unknown state");
        return ERROR;
    }
}
