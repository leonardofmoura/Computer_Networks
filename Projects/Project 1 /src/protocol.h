#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#include <termios.h>
#include <unistd.h>

/**
 * @defgroup protocol protocol
 * @{
 *
 * Functions that interact directly with the serial port
 */

/**
 * @brief defines the current iteration sequence number,
 * setting it to be the last one sent and returning the new sequence number
 *
 * @return int - the current sequence number
 */
int change_sequence_number();

/**
 * @brief Returns the sequence number of the message
 *
 * @param msg - message to be analised
 * @return int - the respective sequence number [0 OR 1] of msg
 */
int get_sequence_number( unsigned char* msg);

/**
 * @brief Returns the sequence number of the answer
 *
 * @param msg - answer to be analised
 * @return int - the respective sequence number [0 OR 1] of msg
 */
int get_sequence_answer(unsigned char* msg);

/**
 * @brief TRANSMITTER: sets last sequence number dependent on the received answer
 *
 * @param msg - message to be analised
 * @return int - new last sequence number
 */
int set_sequence_number_rejected(unsigned char* msg);

/**
 * @brief Receives information through the serial port
 *
 * @param serial_port - The serial file descriptor of the serial port
 * @param msg_ptr - The array in which to store the message
 * @return int - number of bytes received or -1 if some error ocurred
 */
int receive_message(int serial_port, unsigned char* msg_ptr);

/**
 * @brief Sends a command through the serial port
 *
 * @param serial_port - The serial port to send the message
 * @param sent_msg - Array where to store the message sent
 * @param aflag - The address of the sender
 * @param c_set - The command to send
 * @return int - size of sent message or negative on error
 */
int send_message (int serial_port,unsigned char* sent_msg, unsigned char addr, unsigned char c_set);

/**
 * @brief Waits to receive a command
 *
 * @param serial_port - The file descriptor of the serial port
 * @param rec_addr - The addr expecting to receive
 * @param rec_c_set - The command expecting to receive
 * @return int - positive if receives correct message or negative on error
 */
int wait_for_message(int serial_port, unsigned char rec_addr, unsigned char rec_c_set);

/**
 * @brief Sends a message and waits for an answer, if the no answer received or the answer is wrong sends the message again
 *
 * @param serial_port - The file descriptor of the serial port
 * @param addr - The address of the command to send
 * @param c_set - The command to send
 * @param ans_addr - The address expected to receive
 * @param ans_c_set - The command expected to receive
 * @param tries - Number of tries until a timeout
 * @param timeout_time - The time between each try
 * @return int - positive if message was sent and received expected answer, -1 on timeout
 */
int send_timed_message_get_answer(int serial_port, unsigned char addr, unsigned char c_set, unsigned char ans_addr, unsigned char ans_c_set, int tries, int timeout_time);

/**
 * @brief Sends the SET command through the serial port and waits for an UA
 *
 * @param serial_port - file descritor of the serial port to connect
 * @param tries - number of tries in case of a timeout
 * @param timeout - number of seconds until a timeout
 * @return int - positive if OK or negative if timeout ocurred
 */
int serial_port_connect(int serial_port,int tries,int timeout);

/**
 * @brief Waits for a SET command and sends a UA command
 *
 * @param serial_port - The serial port to listen for a connection
 * @return int - positive on success or negative on error
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
int serial_port_start(char * serial_port_name, speed_t baudrate, struct termios* newtio, struct termios* oldtio);

/**
 * @brief Closes the serial port setting its configuration to oldtio
 *
 * @param serial_port_fd - File descriptor where the serial port is open
 * @param oldtio - Configuration to set the serial port (Usually the configuration it had before startig)
 * @return int - only 0 for now
 */
int serial_port_close(int serial_port_fd, struct termios* oldtio);

/**
 * @brief Ends a serial port connection
 *
 * @param serial_port - File escriptor of the serial port
 * @param tries - Number of tries until timeout
 * @param timeout - Time between tries
 * @return int - positive if OK or negative on error
 */
int serial_port_end_connection(int serial_port, int tries, int timeout);

/**
 * @brief Waits the command to end a connection
 *
 * @param serial_port - File descriptor of the serial port
 * @param tries - Number of tries until a timeout
 * @param timeout - Time between each try
 * @return int - positive if OK or negative on error
 */
int serial_port_wait_end_connection(int serial_port, int tries, int timeout);



/**
 *
 * TRANSMITTER END
 *
 */

/**
 * @brief receives the empty message and startes building the frame
 *
 * @param msg - empty message to be constructed
 * @return int - 0 if OK or negative if errors occured
 */
int build_header(unsigned char* msg);

/**
 * @brief Adds input to the output msg
 *
 * @param msg - msg to be field with data
 * @param data - data to be put in msg
 * @param data_length - size of data argument
 * @return size of msg or negative if error
 */
int add_data_to_frame(unsigned char* msg, unsigned char* data, int data_length);

/**
 * @brief stuffes the data bytes that are equal to special bytes,
 * except the initial and final FLAG itself
 *
 * @param output - message with the stuffing applied
 * @param input - message to be applied the stuffing
 * @param input_length - size of input
 * @return int - size of output
 */
int stuffing_message (unsigned char* output, unsigned char* input, int input_length);

/**
 * @brief Sends data message through the serial port
 *
 * @param serial_port - The serial port to send the message
 * @param data - The data to be sent
 * @param data_length - size of data argument
 * @return int - number of data bytes sent, negative if error
 */
int send_message_data (int serial_port, unsigned char* data, int data_length);

/**
 * @brief Verifies if answer is positive and not duplicated
 *
 * @param msg - answer to be analised
 * @return int - 1 if OK or 0 if NOT OK
 */
int answer_confirmation(unsigned char* msg);

/**
 * @brief Receives answer to sent message
 *        and verifies if answer is accordingly to sent
 *
 * @return positive if OK or negative on error (needs to repeat sending)
 */
int receive_answer(int serial_port);

/**
 * @brief Sends a data message (I) and waits for an answer, if nop answer received or the answer is wrong sends the message again
 *
 * @param serial_port - the file descriptor of the serial port
 * @param data - data to be sent
 * @param data_length - size of data
 * @return int - positive if message was send and received expected answer, -1 on timeout
 */
int send_timed_data_get_answer(int serial_port, unsigned char* data, int data_length);



/**
 *
 * RECEIVER END
 *
 */

/**
 * @brief applies destuffing operation to the message received
 *
 * @param output - destuffed message (MUST BE INITIALIZED)
 * @param input - stuffed message
 * @param input_length - size of input
 * @return int - size of output and -1 if error
 */
int destuffing_message (unsigned char* output, unsigned char* input, int input_length);

/**
 * @brief Verifies the header info validity
 *
 * @param msg - message to be analised
 * @return int - 0 if OK, -1 if not valid header
 */
int validate_header( unsigned char* msg);

/**
 * @brief Verifies the data info validity
 *
 * @param msg - message to be analised
 * @param msg_length - size of msg
 * @return int - 1 if OK, 0 if not valid
 */
int validate_data( unsigned char *msg, int msg_length);

/**
 * @brief Answers sender, accordingly to the message received
 *
 * @param serial_port - port that the answer will be sent
 * @param msg - message to be analised
 * @ return int - 0 if OK or -1 if REJ or repeated (data should be ignored)
 */
int send_answer(int serial_port, unsigned char* msg, int msg_length);

/**
 * @brief Receives one data frame through serial port
 *
 * @param serial_port - file descriptor from where the message arrives
 * @param data_msg - output, returns the data received
 * @return int - size of data_msg (all correct),
 *               0 if disconnect command,
 *               -1 if message should be ignored (some processing or header error)
 */
int receive_message_data(int serial_port, unsigned char* data_msg);

/**
 * @brief Introduces bit flips in the frame received, so the code can be tested, 
 *        the approval functions must return errors when bit flips happen
 *        has as internal argument that when changed, provide a ratio of error in the frame received
 * 
 * @param msg - message to be altered 
 * @param length - size of the msg
 * @return void
 */
void addErrors(unsigned char *msg, int length);

/**
 * @}
 */

#endif
