#define _POSIX_C_SOURCE 200809L

#include "protocol_macros.h"
#include "state_machine.h"
#include "protocol.h"

#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <termios.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

//global variables to use with the alarm
static int timeout = FALSE;


void sigalrm_handler_set(int signo) {
    timeout = TRUE;
}


int receive_message(int serial_port,unsigned char* msg_ptr) {
    enum State current_state = INIT; 
    int res;
    int stop = FALSE;
    unsigned char buf;

    while (stop == FALSE) { 
        if (timeout == TRUE) {
            //return timeout
            return 1;
        } 

        res = read(serial_port, &buf, 1);  
        
        if (res == 0 || res == -1) {
            continue; //Continue if nothing was read
        } 

        current_state = process_state(current_state,buf,msg_ptr);

        if (current_state == FINAL) {
            stop = TRUE;
        }
    }

    //return normaly
    return 0;
}


int send_message(int serial_port,unsigned char* sent_msg,int addr, int c_set) {
    //calculate protection byte
    unsigned char bcc = addr ^ c_set;

    if (addr != SENDER && addr != RECEIVER) {
        return 1;
    }
    else if (c_set != SET && c_set != DISC && c_set != UA && c_set != RR && c_set != REJ) {
        return 1;
    }

    //craft the packet
    sent_msg[0] = FLAG;
    sent_msg[1] = addr;
    sent_msg[2] = c_set;
    sent_msg[3] = bcc;
    sent_msg[4] = FLAG;
        
    //send the message
    write(serial_port,sent_msg,5);
    return 0;
}

int serial_port_connect(int serial_port,int tries,int timeout_time) {
    unsigned char sent_msg[5];
    unsigned char msg_rec[5];
    int try_no = 0;
    int rd_res;

    unsigned char addr = SENDER;
    unsigned char c_set = SET;

    //set up the handler
    struct sigaction new_sa,old_sa;
    new_sa.sa_handler = sigalrm_handler_set;
    sigemptyset(&new_sa.sa_mask);
    new_sa.sa_flags = 0;
    sigaction(SIGALRM,&new_sa,&old_sa);


    while (try_no < tries) {
        timeout = FALSE;
        send_message(serial_port,sent_msg,addr,c_set);
        printf("Sent message:\t%x\t%x\t%x\t%x\t%x\n",sent_msg[0],sent_msg[1],sent_msg[2],sent_msg[3],sent_msg[4]); //TODO delete debug
        alarm(timeout_time);
        rd_res = receive_message(serial_port,msg_rec);

        if (rd_res == 0 && msg_rec[2] == UA && msg_rec[1] == SENDER) { //received message normally
            alarm(0); //disable the alarm
            printf("Received message:\t%x\t%x\t%x\t%x\t%x\n",msg_rec[0],msg_rec[1],msg_rec[2],msg_rec[3],msg_rec[4]); //TODO delete debug

            //restore old handler
            sigaction(SIGALRM,&old_sa,NULL);
            return 0;
        }
        else { //invalid answer or timeout try again? TODO check
            try_no++;
            continue;
        }
    }

    //restore old handler
    sigaction(SIGALRM,&old_sa,NULL);

    //number of tries exceeded return error
    return 1;
}

//TODO maybe include a timer to set how much time waiting for a connection
int serial_port_wait_connection(int serial_port) {
    int rd_res;
    unsigned char rec_msg[5];
    unsigned char sent_msg[5];

    rd_res = receive_message(serial_port,rec_msg);
    printf("Received message:\t%x\t%x\t%x\t%x\t%x\n", rec_msg[0], rec_msg[1], rec_msg[2], rec_msg[3], rec_msg[4]); //TODO delete debug

    if (rd_res == 0 && rec_msg[2] == SET && rec_msg[1] == SENDER) { //received valid message
        send_message(serial_port,sent_msg,SENDER,UA); //sends the answer
        printf("Sent message:\t%x\t%x\t%x\t%x\t%x\n", sent_msg[0], sent_msg[1], sent_msg[2], sent_msg[3], sent_msg[4]);
    }
    else {
        //received wrong message 
        return 1;
    }

    return 0;
}

int serial_port_start(char * serial_port_name,char baudrate, struct termios* newtio, struct termios* oldtio) {
    int fd;

    fd = open(serial_port_name, O_RDWR | O_NOCTTY);
    if (fd < 0) {
        perror(serial_port_name);
        exit(-1);
    }

    if (tcgetattr(fd, oldtio) == -1) { /* save current port settings */
        perror("tcgetattr");
        exit(-1);
    }

    memset(newtio,0,sizeof(*newtio));
    newtio->c_cflag = baudrate | CS8 | CLOCAL | CREAD;
    newtio->c_iflag = IGNPAR;
    newtio->c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio->c_lflag = 0;

    newtio->c_cc[VTIME] = 0; /* inter-character timer unused */
    newtio->c_cc[VMIN] = 1;  /* blocking read until 4 chars received */


    tcflush(fd, TCIOFLUSH);

    if (tcsetattr(fd, TCSANOW, newtio) == -1) {
        perror("tcsetattr");
        exit(-1);
    }

    printf("New termios structure set\n");

    return fd;
} 

//TODO maybe check for errors in the tcsetattr funciton
int serial_port_close(int serial_port_fd, struct termios* oldtio) {
    tcsetattr(serial_port_fd, TCSANOW, oldtio);
    close(serial_port_fd);

    return 0;
}



