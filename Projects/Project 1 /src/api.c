#include "api.h"
#include "protocol.h"
#include "protocol_macros.h"

#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

static struct link_layer link_config;

int llopen(int port, int device) {
    if (!(device == _RECEIVER || device == _TRANSMITTER)) { //test invalid device
        return -1;
    }
    else if (!(port == 1 || port == 2)) { //test valid port
        return -2;
    }

    printf("\nTrying to start the connection\n");

    int res;

    //set default settings
    //get port name
    if (port == 1) {
        strcpy(link_config.port,"/dev/ttyS0");
    }
    else if (port == 2) {
        strcpy(link_config.port,"/dev/ttyS1");
    }

    link_config.baud_rate = BAUDRATE;
    link_config.timeout = 3;
    link_config.num_transmit = 3;
    link_config.device = device;

    link_config.file_des =  serial_port_start(link_config.port,link_config.baud_rate,&link_config.newterm,&link_config.oldterm);

    if (device == _TRANSMITTER) {
        res = serial_port_connect(link_config.file_des,link_config.num_transmit,link_config.timeout);
    }
    else if (device == _RECEIVER) {
        res = serial_port_wait_connection(link_config.file_des);
    }

    if (res > 0) {
        printf("Connection established successfully\n\n");
        return link_config.file_des;
    }
    else {
        printf("Could not establish connection\n\n");
        close(link_config.file_des);
        return -1; //error ocurred
    }
}

int llwrite(int fd, unsigned char* buffer, int length) {
    //test if port is still open
    if (fcntl(fd, F_GETFD) == -1 ) {
        return -1;
    }

    return send_timed_data_get_answer(fd, buffer, length);
}

int llread(int fd, unsigned char* buffer) {
    //test if port is still open
    if (fcntl(fd, F_GETFD) == -1 ) {
        return -1;
    }

    int res;
    do {
        res = receive_message_data(fd,buffer);
    } while(res < 0);

    return res;
}

int llclose(int port_fd) {
    int res;

    //test if port is still open
    if (fcntl(port_fd, F_GETFD) == -1 ) {
        printf("Port is already closed\n");
        return -1;
    }

    printf("\n\nTrying to end the connection\n");

    int device = link_config.device;

    if (device == _TRANSMITTER) {
        res = serial_port_end_connection(port_fd,link_config.num_transmit,link_config.timeout);
        sleep(1);
        serial_port_close(port_fd,&link_config.oldterm);
    }
    else if (device == _RECEIVER) {
        res = serial_port_wait_end_connection(port_fd,link_config.num_transmit,link_config.timeout);
        serial_port_close(port_fd,&link_config.oldterm);
    }
    else {
	printf("Could not end connection\n\n");
        return -1;
    }

    printf("Connection ended successfully\n\n");
    return res;
}
