#ifndef _FTP_H_
#define _FTP_H_

#include "url_parser.h"

#define MAX_ANS_SIZE        512

//flags used in ftp_send
#define PRINT_ANSWER        1       //The function will print the answer from the server in stdout
#define NO_PRINT_ANS        0       //The function will not print the answer from the server

typedef struct {
    url_t* url;
    int main_socket;
    int sec_socket;
    char sec_addr[25];
    int sec_port;
} ftp_t;

//Sends a message to the ftp server and gets the answer from the server
int ftp_send_receive(int sock,char* msg, char* answer, int print_ans);

//Receives an answer from the server
int ftp_receive(int sock, char* answer, int print_ans);

//Returns the answer code of a server answer
int get_answer_code(char* answer);

// Opens a tcp socket to the specified url
int connect_socket(url_t* url);

// Opens a tcp socket to the specified url using deprecated functions
int connect_socket_old(url_t* url);

//Connects to a tcp socket if the ip and port are specified
int connect_socket_ip(char* addr, int port);

//Returns the answer code of a server answer
int get_answer_code(char* answer);

//Logs in in the server
int ftp_login(int sock_fd, char* user, char* password);

//Sets the server to passive mode
int ftp_enter_pasv(int sock_fd, int* port, char* addr);

//Transfers a file
int retreive_file(int main_sock,int sec_sock, char* file_path);

//Closes the conection to the server
void ftp_quit(int sock_fd);

#endif
