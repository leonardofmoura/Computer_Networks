#include "application.h"
#include "api.h"

#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <time.h>
#include <sys/time.h>

struct application_layer info;

int print_progress_bar(int *counter, int last_sent, int file_size)
{
#ifndef DEBUG
    *counter += last_sent;
    int bar_size = 50;
    int progress = *counter * bar_size / file_size;
    printf("\r[");
    for (int i = 0; i < bar_size; i++)
    {
        if (i < progress)
            printf("#");
        else
            printf(".");
    }
    printf("] %d%% - %d/%d bytes", progress * 100 / bar_size, *counter, file_size);
    fflush(stdout);
#endif
    return 0;
}

int transmitter()
{
    int write_res;

    info.file_des = open(info.file_name, O_RDONLY);

    //open the file
    if (info.file_des == -1)
    {
        perror("Error opening file");
        exit(-1);
    }

    //determine file size
    struct stat st;
    if (stat(info.file_name, &st) != 0)
    {
        perror("Error determining file name\n");
        exit(-1);
    }
    info.file_size = st.st_size;

    //open the port
    info.port_file_des = llopen(info.port, info.device);

    if (info.port_file_des < 0)
    {
        return -1;
    }

    //Send control package
    unsigned char control_package[MAX_PKG_SIZE];
    int control_len = build_control_package(PKG_START, &info, control_package);
    write_res = llwrite(info.port_file_des, control_package, control_len);

    if (write_res < 0)
    {
        printf("Timeout ocurred\n");
        llclose(info.port_file_des);
        close(info.file_des);
        return -1;
    }

    //File transfer section
    int bytes_read;
    unsigned char data[DATA_SIZE];
    unsigned char data_package[MAX_PKG_SIZE];
    int seq_num = 0, bytes_counter = 0;
    int data_package_len = 0;

    do
    {
        bytes_read = read(info.file_des, data, DATA_SIZE);
        data_package_len = build_data_package(seq_num, bytes_read, data, data_package);
        write_res = llwrite(info.port_file_des, data_package, data_package_len);

        if (write_res < 0)
        {
            printf("Timeout ocurred\n");
            llclose(info.port_file_des);
            close(info.file_des);
            return -1;
        }
#ifdef DEBUG
        printf("package number sent: %d\n", seq_num);
#endif
        print_progress_bar(&bytes_counter, data_package_len - 4, info.file_size);
        seq_num = (seq_num + 1) % 256;
    } while (bytes_read != 0);

    //send end package
    control_len = build_control_package(PKG_END, &info, control_package);
    write_res = llwrite(info.port_file_des, control_package, control_len);

    if (write_res < 0)
    {
        printf("Timeout ocurred\n");
        llclose(info.port_file_des);
        close(info.file_des);
        return -1;
    }

    close(info.file_des);
    llclose(info.port_file_des);

    return 0;
}

int receiver()
{
    //open the port
    info.port_file_des = llopen(info.port, info.device);

    if (info.port_file_des < 0)
    {
        return -1;
    }

    unsigned char received_package[MAX_PKG_SIZE];
    unsigned char processed_data[MAX_PKG_SIZE];
    int seq_num = -1, bytes_counter = 0;
    int data_len = 0;

    struct timespec time0, time1;
    clock_gettime(CLOCK_REALTIME, &time0);
    // forcing a start to be the first
    do
    {
        llread(info.port_file_des, received_package);
        if (received_package[0] == PKG_START)
        {
            process_control_package(received_package, &info);
            info.file_des = open(info.file_name, O_WRONLY | O_CREAT);
        }
    } while (received_package[0] != PKG_START);

    // receiving data until the end package is received
    do
    {
        llread(info.port_file_des, received_package);
        if (received_package[0] == PKG_DATA && ((seq_num + 1) % 256) == received_package[1])
        {
            data_len = process_data_package(received_package, processed_data, &seq_num);
#ifdef DEBUG
            printf("package number received: %d\n", seq_num);
#endif
            print_progress_bar(&bytes_counter, data_len, info.file_size);
            write(info.file_des, processed_data, data_len);
        }
    } while (received_package[0] != PKG_END);

    clock_gettime(CLOCK_REALTIME, &time1);

    long msec = (time1.tv_nsec - time0.tv_nsec) / 1000000;
    long sec = time1.tv_sec - time0.tv_sec;

    if(msec < 0) {
        sec--;
        msec += 1000;
        msec %= 1000;
    }

    printf("Executed in %ld.%ld s\n", sec, msec);

    process_control_package(received_package, &info);

    close(info.file_des);
    llclose(info.port_file_des);

    return 0;
}

int main(int argc, char *argv[])
{
    char *error_message = "usage: file_transmitter <1/2> <receiver/transmitter> [name of the file]\n";

    if (argc != 4 && argc != 3)
    {
        printf("%s\n", error_message);
        exit(1);
    }

    if (strcmp(argv[1], "1") == 0 || strcmp(argv[1], "2") == 0)
    {
        sscanf(argv[1], "%d", &info.port);
    }
    else
    {
        printf("%s\n", error_message);
        exit(2);
    }

    if (strcmp(argv[2], "receiver") == 0)
    {
        info.device = _RECEIVER;
    }
    else if (strcmp(argv[2], "transmitter") == 0 && argc == 4)
    {
        info.device = _TRANSMITTER;
        strcpy(info.file_name, argv[3]);
    }
    else
    {
        printf("%s\n", error_message);
        exit(3);
    }

    if (info.device == _TRANSMITTER)
    {
        transmitter();
    }
    else if (info.device == _RECEIVER)
    {
        receiver();
    }

    return 0;
}
