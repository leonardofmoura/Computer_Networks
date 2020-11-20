#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>

#include "url_parser.h"
#include "ftp.h"

int get_answer_code(char* answer) {
    char answer_code[4];

    memcpy(answer_code,answer,3);
    answer_code[4] = '\0';

    return atoi(answer_code);
}

void get_file_name(char* file_path,char* file_name) {
    char mod_file_path[MAX_FILENAME_SIZE];     //Copy of the file path so we can tokenize it
    strcpy(mod_file_path,file_path);

    char* name = strtok(mod_file_path,"/");
    char* temp;

    while(1) {
        temp = strtok(NULL,"/");
        if (temp == NULL) {
            break;
        }
        else {
            name = temp;
        }
    }

    strcpy(file_name,name);
}

int ftp_send_receive(int sock,char* msg, char* answer, int print_ans) {
    //Send the message
    send(sock,msg,strlen(msg),0);

    //Receive the answer
    return ftp_receive(sock,answer,print_ans);
}

int ftp_receive(int sock, char* answer, int print_ans) {
    char ans[MAX_ANS_SIZE];                   
    char* res = NULL;   
    memset(ans,0,MAX_ANS_SIZE); // clear the answer

    FILE* sock_read = fdopen(sock,"r");

    while (ans[3] != ' ') {
        res = fgets(ans,MAX_ANS_SIZE,sock_read);
        if (res != NULL && print_ans == PRINT_ANSWER) {
            printf("%s\n",ans);
        }
    }

    if (answer != NULL) {
        strcpy(answer,ans);
    }

    //Check the answer               
    return get_answer_code(ans);
}
                

int connect_socket(url_t* url) {

    //find the host using getaddrinfo()
    int status;
    struct addrinfo hints;
    struct addrinfo* servers;           //will point to the results

    memset(&hints,0,sizeof(hints));     //empty the struct
    hints.ai_family = AF_INET;          //Use IPv4 only
    hints.ai_socktype = SOCK_STREAM;    //Use a TCP socket

    //find hosts
    status = getaddrinfo(url->address,url->protocol,&hints,&servers);

    // check for errors
    if (status != 0) {
        fprintf(stderr,"Error finding hosts: %s\n", gai_strerror(status));
        return -1;
    }

    // open the TCP socket for IPv4
    int sock_fd;

    if ((sock_fd = socket(servers->ai_family,servers->ai_socktype,servers->ai_protocol)) < 0 ) {
        perror("Error creating socket");
        return -1;
    }

    //Connect to the address
    if (connect(sock_fd,servers->ai_addr,servers->ai_addrlen) < 0) {
        perror("Error connecting to socket");
        return -1;
    } 

    //Get the answer from the server
    int ans_code = ftp_receive(sock_fd,NULL,NO_PRINT_ANS);

    if (ans_code != 220) {
        printf("Error connecting to the server");
        close(sock_fd);
        freeaddrinfo(servers);
        return -1;
    }


    freeaddrinfo(servers);
    return sock_fd;
}

int connect_socket_old(url_t* url) {
    char ans[MAX_ANS_SIZE];

    //Find the host
    struct hostent* host; 
    
    printf("%s\n",url->address);

    host = gethostbyname(url->address);
    if (host == NULL ) {
        herror("gethostbyname");
        return 1;
    }

    // Get the ip
    char* ip = inet_ntoa(*((struct in_addr *) host->h_addr_list[0]));

    //Connect to the socket using the default port (21)
    int sock_fd = connect_socket_ip(ip,21);

    if (sock_fd < 0) {
        return -1;
    }

    //Get the answer from the server
    int ans_code = ftp_receive(sock_fd,NULL,NO_PRINT_ANS);

    if (ans_code != 220) {
        printf("Error connecting to the server:\n");
        printf("%s\n",ans);
        close(sock_fd);
        return -1;
    }

    return sock_fd;
}

int connect_socket_ip(char* ip, int port) {
    struct sockaddr_in server;
    int sock_fd;
    
    memset(&server,0,sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_port = htons(port);

    //open the socket
    if ((sock_fd = socket(AF_INET,SOCK_STREAM,0)) < 0 ) {
        perror("Error creating socket");
        return -1;
    }


    //Connect to the address
    if (connect(sock_fd,(struct sockaddr*) &server,sizeof(server)) < 0) {
        perror("Error connecting to socket");
        return -1;
    } 

    return sock_fd;
}


int ftp_login(int sock_fd, char* user, char* password) {
    char ans[MAX_ANS_SIZE];

    char user_str[MAX_FILENAME_SIZE];
    strcpy(user_str,"user ");

    char pass_str[MAX_FILENAME_SIZE];
    strcpy(pass_str,"pass ");

    if (user != NULL && password != NULL) {
        strcat(user_str,user);
        strcat(user_str,"\n");

        strcat(pass_str,password);
        strcat(pass_str,"\n");
    }
    else {
        strcat(user_str,"anonymous\n");
        strcat(pass_str,"123\n");
    }


    //Send the user
    int answer_code = ftp_send_receive(sock_fd,user_str,ans,NO_PRINT_ANS);

    if (answer_code != 331) {
        printf("Error specifying the user:\n");
        printf("%s\n",ans);
        return 1;
    }

    // Send the password
    answer_code = ftp_send_receive(sock_fd,pass_str,ans,NO_PRINT_ANS);

    if (answer_code != 230) {
        printf("Error setting the password:\n");
        printf("%s\n",ans);
        return 1;
    }

    printf("Login successful\n");

    return 0;
}

int ftp_enter_pasv(int sock_fd, int* port, char* addr) {
    char* pasv = "pasv\n";
    char ans[MAX_ANS_SIZE];

    //Send the user
    int answer_code = ftp_send_receive(sock_fd,pasv,ans,NO_PRINT_ANS);

    //Check the answer
    if (answer_code != 227) {
        printf("Error setting the server to passive mode:\n");
        printf("%s\n",ans);
        return 1;
    }

    //Parse the answer to get the new server and port 
    //Get only the "6" bytes we need
    strtok(ans,"(");    //splits the string
    char* bytes = strtok(NULL,")");
    char new_addr[25];  // will store the new address
    int new_port;           // will store the new port

    char* addr1 = strtok(bytes,",");
    strcpy(new_addr,addr1); 
    strcat(new_addr,".");

    for (int i = 0; i < 2; i++) {
        char* addrn = strtok(NULL,",");
        strcat(new_addr,addrn);
        strcat(new_addr,".");
    }

    char* addr4 = strtok(NULL,",");
    strcat(new_addr,addr4);

    char* port1 = strtok(NULL,",");
    char* port2 = strtok(NULL,",");

    int p1 = atoi(port1);
    int p2 = atoi(port2);

    new_port = p1*256 + p2;

    //Copy the calculated values to the parameter pointers
    *port = new_port;
    strcpy(addr,new_addr);

    printf("Server entered passive mode\n");

    return 0;
}

void ftp_quit(int sock_fd) {
    char* quit = "quit\n";

    send(sock_fd,quit,strlen(quit),0);
}


int retreive_file(int main_sock,int sec_sock, char* file_path) {
    char ans[MAX_ANS_SIZE];

    char retr[256] = "retr ";
    char filename[MAX_FILENAME_SIZE];
    char buf[MAX_ANS_SIZE];
    int rec;

    get_file_name(file_path,filename);

    strcat(retr,file_path);
    strcat(retr,"\n");

    //send the command
    int retr_code = ftp_send_receive(main_sock,retr,ans,NO_PRINT_ANS);

    if (retr_code != 150) {
        printf("Error opening the file:\n");
        printf("%s\n",ans);
        return 1;
    }

    //create the file 
    int file = open(filename, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

    printf("Transfering file...\n");

    while(1) {
        rec = recv(sec_sock,&buf,MAX_ANS_SIZE,0);

        if (rec == 0) {
            break;
        }
        else {
            write(file,&buf,strlen(buf));
        }
    }

    printf("DONE\n");

    return 0;
}


