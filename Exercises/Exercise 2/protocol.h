#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#include <termios.h>
#include <unistd.h>


/**
 * @brief Receives information through the serial port
 * 
 * @param serial_port - The serial file descriptot of the serial port
 * @param msg_ptr - The array in witch to store the message 
 * @return int - 0 if OK and 1 if some error ocurred
 */
int receive_message(int serial_port,unsigned char* msg_ptr);


/**
 * @brief Sends a command through the serial port
 * 
 * @param serial_port - The serial port to send the message
 * @param sent_msg - Array where to store the message sent
 * @param aflag - The address of the sender
 * @param c_set - The command to send
 */
int send_message (int serial_port,unsigned char* sent_msg,int addr, int c_set);


/**
 * @brief Sends the SET command through the serial port and waits for an UA
 * 
 * @param serial_port - file descritor of the serial port to connect
 * @param tries - number of tries in case of a timeout
 * @param timeout - number of seconds until a timeout
 * @return int - 1 if timeout ocurred and 0 if connected normaly
 */
int serial_port_connect(int serial_port,int tries,int timeout);


/**
 * @brief Waits for a SET command and sends a UA command
 * 
 * @param serial_port - The serial port to listen for a connection
 * @return int - 1 on error 0 on succes
 */
int serial_port_wait_connection(int serial_port);


/**
 * @brief Opens a serial port and sets its configuration
 * 
 * @param serial_port_name - String containing the file of the driver Ex: "/dev/ttyS1"
 * @param baudrate - Baudrate to set the port
 * @param newtio - Pointer to a termios struct to store the new termios config
 * @param oldtio - Pointer to a termios struct to store the old termios config
 * @return int - File descriptor where the serial port is open
 */
int serial_port_start(char * serial_port_name,char baudrate, struct termios* newtio, struct termios* oldtio);

/**
 * @brief Closes the serial port setting its configuration to oldtio
 * 
 * @param serial_port_fd - File descriptor where the serial port is open
 * @param oldtio - Configuration to set the serial port (Usually the configuration it had before startig)
 * @return int - only 0 for now
 */
int serial_port_close(int serial_port_fd, struct termios* oldtio);

#endif
