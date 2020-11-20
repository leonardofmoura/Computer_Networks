/*Non-Canonical Input Processing*/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#include "protocol_macros.h"
#include "state_machine.h"
#include "protocol.h"

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */

volatile int STOP = FALSE;

int main(int argc, char** argv) {
    struct termios oldterm, newterm;

    if ((argc < 2) || ((strcmp("/dev/ttyS0", argv[1]) != 0) &&
                       (strcmp("/dev/ttyS1", argv[1]) != 0))) {
        printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
        exit(1);
    }

    int serial_port = serial_port_start(argv[1],BAUDRATE,&newterm,&oldterm);
    
    int res = serial_port_wait_connection(serial_port);

    if (res == 0) {
        printf("Connection established successfully\n");
    }
    else {
        printf("ERROR: could not establish connection\n");
    }

    serial_port_close(serial_port,&oldterm);

    return 0;
}
