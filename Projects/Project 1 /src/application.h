#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#define MAX_PKG_SIZE    2048    //Max size of a package
#define MAX_FILE_N      64      //Max size of a file name
#define DATA_SIZE       2000    //Number of bytes put in a package at once

#define PKG_DATA        0x1     //Control value that indicates a data package
#define PKG_START       0x2     //Control value that indicates package start
#define PKG_END         0x3     //Control value that indicates package end

#define FILE_SIZE       0x0     //Type value that indicates a file size
#define FILE_NAME       0x1     //Type value that indicates a file name

/**
 * @defgroup application application
 * @{
 *
 * Application layer funtions
 */

struct application_layer
{
    int port_file_des;          //File descriptor where the serial port is open
    int file_des;               //Descriptor of the file being transfered
    char file_name[MAX_FILE_N]; //Name of the file being trasfered
    int file_size;              //Size of the file being transfered
    int device;                 //TRANSMITER OR RECEIVER
    int port;                   //The port number currently using
};

/**
 * @brief Creates a control package with the specified fields
 *
 * @param control - The control byte of the package
 * @param info - Information about the file being trasfered
 * @param package - Array where the package will be put
 * @return int - The size of the array
 */
int build_control_package(unsigned char control, struct application_layer *info, unsigned char package[]);

/**
 * @brief Creates a data package with the specified fields
 *
 * @param sequence_num - Sequence number of the package
 * @param data_length - Number of the bytes of the data in the package
 * @param data - Data to include in the package
 * @param package - Array where to put the package
 * @return int - Size of the array
 */
int build_data_package(int sequence_num, int data_length, unsigned char data[], unsigned char package[]);

/**
 * @brief Processes a received data package
 *
 * @param data_package - The array where the package is stored
 * @param extracted_data - The array where the data will be stored
 * @param sequence_num - Pointer to an integer where the sequence number of the package will be stored
 * @return int - The length of the data processed
 */
int process_data_package(unsigned char data_package[], unsigned char extracted_data[], int *sequence_num);

/**
 * @brief Processes a received control package
 *
 * @param package - The array where the package is stored
 * @param info - Information about the file being trasfered
 * @return int - The command or -1 on error
 */
int process_control_package(unsigned char package[], struct application_layer *info);

/**
 * @}
 */

#endif
