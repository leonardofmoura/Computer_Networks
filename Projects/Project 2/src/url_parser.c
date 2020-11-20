#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "url_parser.h"

//Creates a new url struct with default values
url_t* init_url() {
    url_t* url = (url_t*) malloc(sizeof(url_t));

    url->address = DEFAULT_SERVER_ADDR;
    url->protocol = DEFAULT_SERVER_PORT;
    url->file_path = DEFAULT_FILE_PATH;
    url->user = NULL;
    url->password = NULL;

    return url;
}

void delete_url(url_t* url) {
    free(url);
}

//prints a url
void print_url(url_t* url) {
    printf("%s://%s/%s",url->protocol,url->address,url->file_path);
}

//parses a url string to a url_t
int parse_url(char* url, url_t* parsed) {
    
    parsed->protocol = strtok(url,"://");
    char* user_password = strtok(NULL,"@"); //Separates the user and password part, which are not used
    parsed->address = strtok(NULL,"/");
    parsed->file_path = strtok(NULL,"");

    //Set user and password to null by default
    parsed->password = NULL;
    parsed->user = NULL;

    //Check if no user/password was found -> The rest of the url is in user_password
    if(parsed->address == NULL) {
        parsed->address = strtok(user_password,"/");
        parsed->file_path = strtok(NULL,"");
    }
    else { //Means user and password were found
        parsed->user = strtok(user_password,":");
        parsed->user = parsed->user + 2; // trick to remove the // strok leaves in user_password
        parsed->password = strtok (NULL,"@");
    }

    if (strcmp(parsed->protocol,"ftp")) {
        fprintf(stderr,"This application only suports the ftp protocol\n");
        return 1;
    }

    return 0;
}