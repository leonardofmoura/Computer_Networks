#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <string.h>

#include "url_parser.h"
#include "ftp.h"

int main(int argc, char** argv) {

    ftp_t info;

    info.url = init_url();
    int res;

    if (argc == 2) {
        int res = parse_url(argv[1],info.url);

        if (res != 0) {
            delete_url(info.url);
            return 1;
        }
    }
    else if (argc == 1) {
        printf("Wrong number of arguments: using default values\n");
    }

    info.main_socket = connect_socket(info.url);

    if (info.main_socket < 0) {
        delete_url(info.url);
        return 1;
    }

    //login in the server
    res = ftp_login(info.main_socket,info.url->user,info.url->password);

    if (res != 0) {
        delete_url(info.url);
        close(info.main_socket);
        return 1;
    }

    //Enter passive mode
    res = ftp_enter_pasv(info.main_socket,&info.sec_port,info.sec_addr);

    if (res != 0) {
        delete_url(info.url);
        close(info.main_socket);
        return 1;
    }


    //Connect second socket
    info.sec_socket = connect_socket_ip(info.sec_addr,info.sec_port);

    retreive_file(info.main_socket,info.sec_socket,info.url->file_path);

    ftp_quit(info.main_socket);

    delete_url(info.url);
    close(info.main_socket);
    close(info.sec_socket);

    return 0;
}
