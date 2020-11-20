#ifndef _STATE_MACHINE_H_
#define _STATE_MACHINE_H_
enum State {INIT, FLG, ADDR, CTRL, PROT, FINAL, ERROR};


/**
 * @brief Processes a state, returns the next state considering the current state and the received character
 * @param current_state - State enum indicating the current state
 * @param received - The byte of the frame we want to process
 * @param msg - array where to store the bytes of the message
 */
enum State process_state(enum State current_state, unsigned char received,unsigned char*  msg);


#endif
