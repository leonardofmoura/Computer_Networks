/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include "protocol.h"
#include "protocol_macros.h"
#include "state_machine.h"

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */

volatile int STOP=FALSE;

int serialPort;

int main(int argc, char** argv)
{
  struct termios oldtio,newtio;

    if ((argc < 2) || ((strcmp("/dev/ttyS0", argv[1]) != 0) && (strcmp("/dev/ttyS1", argv[1]) != 0))) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }
  
  serialPort = serial_port_start(argv[1],BAUDRATE,&newtio,&oldtio);

  int connection = serial_port_connect(serialPort,3,3);

  if (connection == 0) {
    printf("Serial port connected succesfully\n");
  }
  else {
    printf("ERROR clould not connect to the serial port: TIMEOUT\n");
  }

  serial_port_close(serialPort,&oldtio);

  return 0;
}
