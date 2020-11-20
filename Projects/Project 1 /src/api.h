#ifndef _API_H_
#define _API_H_

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

/** 
 * @defgroup API API 
 * @{
 * 
 * Funcitons that interact with the protocol 
 */

#define _TRANSMITTER  0       //Code of the transmiter
#define _RECEIVER    1       //Code of the receiver   

struct link_layer {
    char port[20];              //name of the port
    int file_des;               //file descriptor where the port is open
    speed_t baud_rate;              //rate of transmition
    int device;                 //type of device
    // unsigned int sequence_no;   //sequence number of the frame
    unsigned int timeout;       //time until timeout
    unsigned int num_transmit;  //number of transmitions until error
    struct termios oldterm;     //Old configs of the port
    struct termios newterm;     //New configs of the port
}; 


/**
 * @brief Opens the serial port and tries to establish a connection 
 * 
 * @param port - Number of the port to open (COM1 OR COM2)
 * @param buffer - output: the data received through
 * @return int - file descriptor of connection, negative value on error
 */
int llopen(int port, int device);

/**
 * @brief Writes the message to fd
 * 
 * @param fd - file descriptor from where the data will be sent
 * @param buffer - data to be written
 * @param length - size of buffer
 * @return int - number of bytes written (size of buffer)
 * 
 */
int llwrite(int fd, unsigned char* buffer, int length);

/**
 * @brief Reads the message from fd
 * 
 * @param port - file descriptor from where the data will be read
 * @param buffer - data that was received
 * @return int - number of chars read (size of buffer)
 */
int llread(int fd, unsigned char* buffer);

/**
 * @brief Ends a connection and closes the serial port
 * 
 * @param port_fd - File descriptor of the port
 * @return int - positive if OK, negative if error
 */
int llclose(int port_fd);

/**
 * @}
 */

#endif
