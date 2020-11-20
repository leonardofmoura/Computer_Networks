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
#include <time.h>

//global variables to use with the alarm
static int timeout = FALSE;

void sigalrm_handler_set(int signo) {
    timeout = TRUE;
}


// global variables to use in the receiver end
// first one is 1
int last_sequence_no = 1;

int change_sequence_number() {
    last_sequence_no = last_sequence_no ? 0 : 1;
    return last_sequence_no;
}

int get_sequence_number(unsigned char* msg) {
    // verifies if bit 6 of byte 2 is 1
    return msg[2] & I1 ? 1 : 0;
}

int get_sequence_answer(unsigned char* msg) {
    // verifies if answer ctrl is R1 (1) or R0 (0)
    return msg[2] & R1 ? 1 : 0;
}

int set_sequence_number_rejected(unsigned char* msg) {
    last_sequence_no = get_sequence_answer(msg) ? 1 : 0;
    return last_sequence_no;
}



int receive_message(int serial_port, unsigned char* msg_ptr) {
    enum State current_state = INIT;
    int res, bytes_counter=0;
    int stop = FALSE;
    unsigned char buf;

    while (stop == FALSE) {
        if (timeout == TRUE) {
            //return timeout
            return -1;
        }

        // reads 1 byte
        res = read(serial_port, &buf, 1);
        if (res == 0 || res == -1) {
            continue; //Continue if nothing was read
        }

        // update state
        current_state = process_state(current_state,buf,msg_ptr, &bytes_counter);

        if (current_state == FINAL) {
            stop = TRUE;
        }
    }

    return bytes_counter;
}

int send_message(int serial_port, unsigned char* sent_msg, unsigned char addr, unsigned char c_set) {
    //calculate protection byte
    unsigned char bcc = addr ^ c_set;

    if (addr != SENDER && addr != RECEIVER) {
        return -1;
    }

    //craft the frame
    sent_msg[0] = FLAG;
    sent_msg[1] = addr;
    sent_msg[2] = c_set;
    sent_msg[3] = bcc;
    sent_msg[4] = FLAG;

    //send the message
    return write(serial_port,sent_msg,5);
}

int wait_for_message(int serial_port, unsigned char rec_addr, unsigned char rec_c_set) {
    int rd_res;
    unsigned char rec_msg[5];

    int counter = 3;
    do {
        rd_res = receive_message(serial_port,rec_msg);

        if (rd_res > 0 && rec_msg[2] == rec_c_set && rec_msg[1] == rec_addr) { //received valid message
            return 1;
        }
    } while(--counter);

    return -1;
}

int send_timed_message_get_answer(int serial_port, unsigned char addr, unsigned char c_set, unsigned char ans_addr, unsigned char ans_c_set,int tries, int timeout_time) {
    unsigned char sent_msg[5];
    unsigned char msg_rec[5];
    int try_no = 0;
    int rd_res;

    //set up the handler
    struct sigaction new_sa,old_sa;
    new_sa.sa_handler = sigalrm_handler_set;
    sigemptyset(&new_sa.sa_mask);
    new_sa.sa_flags = 0;
    sigaction(SIGALRM,&new_sa,&old_sa);

    while (try_no < tries) {
        timeout = FALSE;
        send_message(serial_port,sent_msg,addr,c_set);
        alarm(timeout_time);
        rd_res = receive_message(serial_port,msg_rec);

        if (rd_res > 0 && msg_rec[2] == ans_c_set && msg_rec[1] == ans_addr) { //received message normally
            alarm(0); //disable the alarm

            //restore old handler
            sigaction(SIGALRM,&old_sa,NULL);
            return 1;
        }
        //invalid answer or timeout try again? TODO check
        try_no++;
	#ifdef DEBUG
        printf("Failed attempt to send %d\n", try_no);
	#endif
    }

    //restore old handler
    sigaction(SIGALRM,&old_sa,NULL);

    //number of tries exceeded return error
    return -1;
}

int serial_port_connect(int serial_port,int tries,int timeout_time) {
    return send_timed_message_get_answer(serial_port,SENDER,SET,SENDER,UA,tries,timeout_time);
}

//TODO maybe include a timer to set how much time waiting for a connection
int serial_port_wait_connection(int serial_port) {
    unsigned char sent_msg[5];

    wait_for_message(serial_port,SENDER,SET);
    return send_message(serial_port,sent_msg,SENDER,UA) > 0;
}

int serial_port_start(char * serial_port_name, speed_t baudrate, struct termios* newtio, struct termios* oldtio) {

    srand((unsigned) time(NULL));

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

    #ifdef DEBUG
        printf("New termios structure set\n");
    #endif

    return fd;
}

//TODO maybe check for errors in the tcsetattr funciton
int serial_port_close(int serial_port_fd, struct termios* oldtio) {
    tcsetattr(serial_port_fd, TCSANOW, oldtio);
    close(serial_port_fd);

    return 0;
}

int serial_port_end_connection(int serial_port, int tries, int timeout) {
    unsigned char sent_msg[5];

    int res1 = send_timed_message_get_answer(serial_port,SENDER,DISC,SENDER,DISC,tries,timeout);
    int res2 = send_message(serial_port,sent_msg,SENDER,UA);

    if (res1 < 0 || res2 < 0) {
        return -1;
    }
    else {
        return 1;
    }
}

int serial_port_wait_end_connection(int serial_port, int tries, int timeout) {

    int res1 = wait_for_message(serial_port,SENDER,DISC);
    int res2 = send_timed_message_get_answer(serial_port,SENDER,DISC,SENDER,UA,tries,timeout);

    if (res1 < 0) {
        return -1;
    }
    else if (res2 < 0) {
        return -1;
    }
    else {
        return 1;
    }
}



int build_header(unsigned char* msg) {
    unsigned char addr, ctrl, bcc;

    addr = SENDER;
    ctrl = change_sequence_number() ?  I1 : I0;
    bcc = addr ^ ctrl;

    msg[0] = FLAG;
    msg[1] = addr;
    msg[2] = ctrl;
    msg[3] = bcc;

    return 0;
}

int add_data_to_frame(unsigned char* msg, unsigned char* data, int data_length)
{
    unsigned char bcc = msg[3];

    int msg_length = 4;
    for (int i = 0; i < data_length; i++, msg_length++) {
        bcc = bcc ^ data[i];
        msg[msg_length] = data[i];
    }

    msg[msg_length] = bcc;
    msg[msg_length+1] = FLAG;
    return msg_length+2;
}

int stuffing_message (unsigned char* output, unsigned char* input, int input_length) {

    int output_length = 1;
    output[0] = input[0]; //first FLAG

    // first and last char are not changed, those are the real FLAG
    for (int i = 1; i < input_length -1; i++, output_length++) {
        if(input[i] != ESCAPE && input[i] != FLAG) {
            output[output_length] = input[i];
        }
        else {
            output[output_length] = ESCAPE;
            output_length++;
            output[output_length] = input[i]^ESC_BYTE;
        }
    }

    output[output_length] = input[input_length-1];
    output_length++;
    return output_length;
}

int send_message_data(int serial_port, unsigned char* data, int data_length) {

    unsigned char destuffed_msg[MAX_FRAME_SIZE], stuffed_msg[MAX_FRAME_SIZE << 1];
    int stuffed_length, destuffed_length;

    // start the frame
    build_header(destuffed_msg);

    // put data received in the frame
    destuffed_length = add_data_to_frame(destuffed_msg, data, data_length);

    // eliminating FLAG-like bytes in the middle of the frame by stuffing it
    stuffed_length = stuffing_message(stuffed_msg, destuffed_msg, destuffed_length);

    int sent_length = write(serial_port,stuffed_msg,stuffed_length);

	#ifdef DEBUG
    printf("sent: ");
    for(int i = 0; i < sent_length; i++)
        printf("%x ", stuffed_msg[i]);
    printf("\n");
	#endif

    return sent_length == stuffed_length ? data_length : -1;
}

int answer_confirmation(unsigned char* msg) {
    if(!( (msg[2] == (RR | R1)) || (msg[2] == (RR | R0)) )) {
        return 0;
    }

    return get_sequence_answer(msg) != last_sequence_no;
}

int receive_answer(int serial_port) {
    unsigned char answer[6];

    int answer_size = receive_message(serial_port, answer);

    if (answer_size < 0) {
        change_sequence_number();
	#ifdef DEBUG
        printf("received: N/A\n");
	#endif
        return -1;
    }

    #ifdef DEBUG
        printf("received: ");
        for(int i = 0; i < answer_size; i++)
            printf("%x ", answer[i]);
        printf("\n");
    #endif

    if (answer_confirmation(answer)) { //RR0 or RR1
        return 1;
    }

    // REJ0 or REJ1
    set_sequence_number_rejected(answer);
    return -1;
}

int send_timed_data_get_answer(int serial_port, unsigned char* data, int data_length) {
    int try_no = 0;
    int res, rd_res;
    int tries = TIMEOUT_TRIES, timeout_time = TIMEOUT_TIME;

    //set up the handler
    struct sigaction new_sa,old_sa;
    new_sa.sa_handler = sigalrm_handler_set;
    sigemptyset(&new_sa.sa_mask);
    new_sa.sa_flags = 0;
    sigaction(SIGALRM,&new_sa,&old_sa);

    while (try_no < tries) {
        timeout = FALSE;
        res = send_message_data(serial_port, data, data_length);
        alarm(timeout_time);
        rd_res = receive_answer(serial_port);

        if (res == data_length && rd_res > 0) { //sent message and received answer normally
            alarm(0); //disable the alarm

            //restore old handler
            sigaction(SIGALRM,&old_sa,NULL);
            return res;
        }

        //invalid answer or timeout try again? TODO check
        try_no++;
	#ifdef DEBUG
        printf("Failed attempt to send %d\n", try_no);
	#endif
    }
    return -1;
}



int destuffing_message(unsigned char* output, unsigned char* input, int input_length) {
    // first byte of frame (FLAG)
    output[0] = input[0];

    int output_length = 1;
    for(int i = 1; i < input_length; i++) {
        if(input[i] == ESCAPE) {
            i++; // go to the byte that will be destuffed
            unsigned char byte = input[i];

            // byte XOR 0x20
            byte = byte^ESC_BYTE;

            output[output_length] = byte;
            output_length++;
        } else {
            // procedes as normal
            output[output_length] = input[i];
            output_length++;
        }
    }

    return output_length;
}

int validate_header(unsigned char* msg) {
    unsigned char addr = msg[1], ctrl = msg[2], bcc = msg[3];

    // if address XOR control doesn't check out
    if((addr^ctrl) != bcc) return -1;

    return 0;
}

int validate_data(unsigned char* msg, int msg_length) {
    unsigned char bcc2 = msg[3]; // starts equal to bbc1

    for (int i = 4; i < msg_length - 2; i++) // goes through data until 2 positions till end
        bcc2 = bcc2^msg[i];

    // compare calculated bcc with received data_bcc
    return bcc2 == msg[msg_length - 2];
}

int send_answer(int serial_port, unsigned char* msg, int msg_length) {
    unsigned char sent[6] = {0};
    // get the sequence number to the answer
    unsigned char sequence = get_sequence_number(msg) ?  R0 : R1;

    // if repeated sequence number, send respective RR and discard data
    if(get_sequence_number(msg) == last_sequence_no) {
	#ifdef DEBUG
        printf("repeated\n");
	#endif
        send_message(serial_port, sent, SENDER, RR | sequence);
        return -1;
    }

    // if bcc2 not valid
    if(!validate_data(msg, msg_length)) {
	#ifdef DEBUG
        printf("rejected\n");
	#endif
        send_message(serial_port, sent, SENDER, REJ | sequence);
        return -1;
    }

    // everything OK
    change_sequence_number();
    send_message(serial_port, sent, SENDER, RR | sequence);

    #ifdef DEBUG
        printf("sent: ");
        for(int i = 0; i < 5; i++)
            printf("%x ", sent[i]);
        printf("\n");
    #endif
    return 1;
}

void addErrors(unsigned char* msg, int length) {
    int random = rand() % 100;
    int ratio = 0;

    if(random < ratio) {
        int pos = rand() % (length -6) + 4;
        msg[pos] ^= (1 << (rand() % 8));
    }
}

int receive_message_data(int serial_port, unsigned char* data_msg) {
    unsigned char received_msg[MAX_FRAME_SIZE] = {0}; // raw message
    unsigned char destuffed_msg[MAX_FRAME_SIZE] = {0};// processed message

    int received_bytes = receive_message(serial_port, received_msg);

    //addErrors(received_msg, received_bytes); // used for efficiency testing

    if (received_bytes == -1)
        return -1;

    int destuffed_length = destuffing_message(destuffed_msg, received_msg, received_bytes);
    if (destuffed_length < 0)
        return -1;


	#ifdef DEBUG
        printf("received: ");
        for(int i = 0; i < destuffed_length; i++)
            printf("%x ", destuffed_msg[i]);
        printf("\n");
	#endif

    // at this point, we know that it is a data frame, so we will answer accordingly
    // send_answer() analises the data and send response to sender
    if(send_answer(serial_port, destuffed_msg, destuffed_length) < 0) {
        return -1;
    }

    // get only the data inside the message frame
    int data_length = destuffed_length - 6;

    // copies data from inside the frame array
    memcpy(data_msg, destuffed_msg+4, data_length);

    return data_length;
}
