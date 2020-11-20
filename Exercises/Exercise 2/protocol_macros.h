#ifndef _PROTOCOL_MACROS_H_
#define _PROTOCOL_MACROS_H_

#define TRUE        1 
#define FALSE       0


//Control bytes
#define SET         0x03    // set up
#define DISC        0x0b    // disconnect
#define UA          0x07    // unnumbered acknowledgement
#define RR          0x05    // receiver ready / positive acknowledgement
#define REJ         0x01    // reject / negative acknowledgement

#define FLAG        0x7e    //0x01111110

//Address bytes
#define SENDER      0x03    //Commands sent by the sender and answers sent by the receiver
#define RECEIVER    0x01    //Commands sent by the receiver and answers sent by the sender

#endif
