#ifndef _PROTOCOL_MACROS_H_
#define _PROTOCOL_MACROS_H_

/**
 * @defgroup protocol_macros protocol_macros
 * @{
 *
 * Macros used by the protocol
 */

#define TRUE            1
#define FALSE           0

#define MAX_FRAME_SIZE  4096

//Error protocol
#define TIMEOUT_TRIES   25
#define TIMEOUT_TIME    2

//Control bytes
#define SET             0x03    // set up
#define DISC            0x0b    // disconnect
#define UA              0x07    // unnumbered acknowledgement
#define RR              0x05    // receiver ready / positive acknowledgement
#define REJ             0x01    // reject / negative acknowledgement

#define FLAG            0x7e    //beginning and end of each frame

//Address bytes
#define SENDER          0x03    //Commands sent by the sender and answers sent by the receiver
#define RECEIVER        0x01    //Commands sent by the receiver and answers sent by the sender

#define BAUDRATE        B38400  //Default baudrate

//Stuffing bytes
#define ESCAPE          0x7d    //byte that indicates that a byte was changed
#define ESC_BYTE        0x20    //byte that xor with the byte exhausted, giving the second byte

//sequence bytes
#define I0              0x00    // information message even
#define I1              0x40    // information message odd
#define R0              0x00    // answer information message even
#define R1              0x80    // answer information message odd

/**
 * @}
 */

#endif
