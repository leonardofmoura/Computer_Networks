// #include "application.h"
// #include "api.h"
// #include "protocol.h"

// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>

// int main(int argc, char * argv[]) {
    // //API testing
    //  char * error_message = "usage: app <1/2> <receiver/transmiter>\n";
    //  int port_num;
    //  int port_fd;
    //  int device;

    //  if (argc != 3) {
    //      printf("%s",error_message);
    //      exit(1);
    //  }

    //  if (strcmp(argv[1],"1") == 0 || strcmp(argv[1],"2") == 0) {
    //      sscanf(argv[1],"%d",&port_num);
    //  }
    //  else {
    //      printf("%s",error_message);
    //      exit(2);
    //  }

    //  if (strcmp(argv[2],"receiver") == 0) {
    //      device = _RECEIVER;
    //  }
    //  else if (strcmp(argv[2],"transmiter") == 0) {
    //      device = _TRANSMITER;
    //  }
    //  else {
    //      printf("%s",error_message);
    //      exit(3);
    //  }

    //  port_fd = llopen(port_num,device);

    //  if (port_fd < 0) {
    //      printf("ERROR: could not establish connection\n");
    //      exit(4);
    //  }

    //  printf("Connection established succesfully\n");

    //  char data [100] = "ola OLA oLa\0";

    //  printf("\n");


    //  llwrite(port_fd, data);

    //  int cl_res = llclose(port_fd);

    //  if (cl_res < 0 ) {
    //      printf("Error: wrong device\n");
    //  }
    //  else if (cl_res > 0) {
    //      printf("Error ending connection\n");
    //  }
    //  else {
    //      printf("Connection ended succesfully\n");
    //  }

    // unsigned char stuffed[] = {0x7e, 0x03, 0x00, 0x03, 0x61, 0x61, 0x61, 0x7d, 0x41, 0x02, 0x7e};
    // unsigned char destuffed[11];

    // printf("stuffed:   %x %x %x %x %x %x %x %x %x %x %x\n", stuffed[0], stuffed[1], stuffed[2], stuffed[3], stuffed[4], stuffed[5], stuffed[6], stuffed[7], stuffed[8], stuffed[9], stuffed[10]);
    // int res = receive_message_data(0, destuffed, stuffed);
    // printf("res: %d\n", res);
    // printf("destuffed: %x %x %x %x %x %x %x %x %x %x\n", destuffed[0], destuffed[1], destuffed[2], destuffed[3], destuffed[4], destuffed[5], destuffed[6], destuffed[7], destuffed[8], destuffed[9]);
    // printf("%s\n", destuffed);


    // Application testing -> coment to test another thing
    // unsigned char package[MAX_PKG_SIZE];
    // int s = 1999999;
    // int package_size = build_control_package(PKG_START,s,"file",package);

    // printf("size: %x\n",s);

    // //print the package 
    // for (int loles = 0; loles < package_size; loles++) {
    //     printf("%x\n",package[loles]);
    // }

    // char test_data[MAX_PKG_SIZE] = "Generico grande semis xnaxos, obvio";
    // int len = strlen(test_data);

    // unsigned char pac[MAX_PKG_SIZE];
    // int pa = build_data_package(0,len,test_data,pac);

    // printf("Package:");
    // for (int i = 0; i < len+4; i++) {
    //     printf(" %x",pac[i]);
    // }

    // printf("\ndata:");
    // for (int i = 4; i < len+4; i++) {
    //     printf("%c",pac[i]);
    // }
    // printf("\n");

    // unsigned char extracted[MAX_PKG_SIZE];
    // int seq_num;
    // process_data_package(pac,extracted,&seq_num);

    // printf("processed package:");
    // for (int i = 0; i < len+4; i++) {
    //     printf(" %x",extracted[i]);
    // }

    // int size = 2;
    // char name[32];
    // int command = process_control_package(package,&size,name);
    // printf("procesed control:%d,%s,%d\n",size,name,command);

//     return 0;
// }
