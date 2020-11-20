#include <stdio.h>

#include "state_machine.h"
#include "protocol_macros.h"

enum State process_state(enum State current_state, unsigned char received,unsigned char* msg) {
    switch (current_state) {
    case INIT:
        if (received == FLAG) {
            msg[0] = received;
            return FLG;
        }
        else {
            return INIT;
        }

    case FLG: 
        if (received == FLAG) {
            msg[0] = received;
            return FLG;
        }
        else if (received == SENDER || received == RECEIVER) {
            msg[1] = received;
            return ADDR;
        }
        else {
            return INIT;
        }

    case ADDR:
        if (received == FLAG) {
            msg[0] = received;
            return FLG;
        }
        else if (received == SET || received == DISC || received == UA || received == RR || received == REJ){
            msg[2] = received;
            return CTRL;
        }
        else {
            return INIT;
        }
    
    case CTRL:
        if (received == FLAG) {
            return FLG;
        }
        else if ((msg[1] ^ msg[2]) == received) { 
            msg[3] = received;
            return PROT;
        }
        else {
            return INIT;
        }

    case PROT:
        if (received == FLAG) {
            msg[4] = received;
            return FINAL;
        }
        else {
            return INIT;
        }

    default:
        printf("Unknown state");
        return ERROR;
    }
}
