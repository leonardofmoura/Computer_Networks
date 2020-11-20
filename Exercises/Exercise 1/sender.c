/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE;

int main(int argc, char** argv)
{
  int serialPort,res;
  struct termios oldtio,newtio;
  char buf[255];

  if ( (argc < 2) || ((strcmp("/dev/ttyS0", argv[1])!=0) && (strcmp("/dev/ttyS1", argv[1])!=0) )) {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
    exit(1);
  }


  /*
  Open serial port device for reading and writing and not as controlling tty
  because we don't want to get killed if linenoise sends CTRL-C.
  */


  serialPort = open(argv[1], O_RDWR | O_NOCTTY );
  if (serialPort <0) {perror(argv[1]); exit(-1); }

  if ( tcgetattr(serialPort,&oldtio) == -1) { /* save current port settings */
    perror("tcgetattr");
    exit(-1);
  }

  bzero(&newtio, sizeof(newtio));
  newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
  newtio.c_iflag = IGNPAR;
  newtio.c_oflag = 0;

  /* set input mode (non-canonical, no echo,...) */
  newtio.c_lflag = 0;

  newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
  newtio.c_cc[VMIN]     = 5;   /* blocking read until 5 chars received */



  /*
  VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
  leitura do(s) pr�ximo(s) caracter(es) i < 255; i++) {
  buf[i] = 'a';
  }
  */

  tcflush(serialPort, TCIOFLUSH);

  if ( tcsetattr(serialPort,TCSANOW,&newtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  printf("New termios structure set\n");


  /*
  O ciclo FOR e as instru��es seguintes devem ser alterados de modo a respeitar
  o indicado no gui�o
  */

  if (fgets(buf,255,stdin) == NULL) {
    printf("Error reading stdin\n");
  }

  //remove '\n' from the string
  buf[strcspn(buf,"\n")] = '\0';

  write(serialPort,&buf,strlen(buf)+1);
  printf("Message sent\n");

  sleep(2);

  int chars_received = 0;
  char receiving_buffer[255];
  char temp_receiver;

  while (chars_received < strlen(buf)+1) {            /* loop for input */
    res = read(serialPort,&temp_receiver,1);   /* returns after 5 chars have been input */

    if(res > 0) {
      receiving_buffer[chars_received] = temp_receiver;
      chars_received++;
    }              

    printf("Received character [%c]\n", temp_receiver);
  }

  printf("Message received: [%s]\n",receiving_buffer);

  sleep(2);

  if ( tcsetattr(serialPort,TCSANOW,&oldtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  close(serialPort);
  return 0;
}
