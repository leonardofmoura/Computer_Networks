#ifndef _URL_PARSER_H_
#define _URL_PARSER_H_

#define DEFAULT_SERVER_PORT "ftp"
#define DEFAULT_SERVER_ADDR "ftp.up.pt"
#define DEFAULT_FILE_PATH   "mirrors/ftp.gnome.org/README" 

#define MAX_FILENAME_SIZE   128

/**
 * @brief Struct that defines a URL
 * 
 */
typedef struct {
    char* user;             //The user to access the server
    char* password;         // The password to access the server
    char* protocol;         //The protocol in use Ex "ftp"
    char* address;          //The address to connect to Ex "ftp.up.pt"
    char* file_path;        //The path of the file to download Ex "mirrors/ftp.gnome.org/README"
} url_t;

//Creates a new url struct with default values
url_t* init_url();

//deletes a url struct
void delete_url(url_t* url);

//prints a url
void print_url(url_t* url);

//parses a url string to a url_t
int parse_url(char* url, url_t* parsed);

#endif