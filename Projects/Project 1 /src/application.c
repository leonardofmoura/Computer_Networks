#include "api.h"
#include "application.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int build_control_package(unsigned char control, struct application_layer* info, unsigned char package[]) {
    //Set the control
    if (control != PKG_START && control != PKG_END) {
        return -1;
    }
    package[0] = control;

    //Set the file size
    unsigned char temp_size[4];
    unsigned char curr_byte = (unsigned char) info->file_size;
    int i = 1;
    int shift;

    //this will put the bytes in the array in reverse order
    while (curr_byte != 0) {
        temp_size[i] = curr_byte;
        shift = i*8;
        curr_byte = (unsigned char) (info->file_size >> shift);
        i++;
    }

    //we now know all we need to know to set the file size in the package
    package[1] = FILE_SIZE;
    int file_size_len = i-1;
    package[2] = (unsigned char) file_size_len;

    //copy size bytes to the package
    for (int j = file_size_len; j > 0; j--) {
        package[2+j] = temp_size[file_size_len+1-j];
    }

    //Set the file name
    int name_index = 0;
    unsigned char curr_char = info->file_name[name_index];
    package[file_size_len+5+name_index] = info->file_name[name_index];

    while(curr_char != '\0') {
        name_index++;
        curr_char = info->file_name[name_index];
        package[file_size_len+5+name_index] = info->file_name[name_index];
    }
    package[i+6+name_index] = curr_char;

    //now we know all we need to know to set the file name in the package
    package[file_size_len+3] = FILE_NAME;
    package[file_size_len+4] = name_index+1;

    //create the package
    int package_size = file_size_len+6+name_index;

    return package_size;
}


int build_data_package(int sequence_num,int data_length,unsigned char data[],unsigned char package[]) {
    //set control
    package[0] = PKG_DATA;

    //set sequence number
    if (sequence_num > 256) {
        printf("Error building data package: Sequence number must be greater than one\n");
        return -1;
    }
    package[1] = (unsigned char) sequence_num;

    //Set data length
    if (data_length > (MAX_PKG_SIZE - 4)) {
        printf("Error building data package: data size is greater than maximum package size");
        return -1;
    }

    package[2] = (unsigned char) (data_length >> 8);
    package[3] = (unsigned char) data_length;

    //set the data
    for (int i = 0; i < data_length; i++) {
        package[i+4] = data[i];
    }

    int package_size = data_length + 4;

    return package_size;
}

int process_data_package(unsigned char data_package[], unsigned char extracted_data[],int* sequence_num) {
    *sequence_num = (int) data_package[1];

    //calculate length of the data
    int len1 = (int) data_package[3];
    int len2 = (int) (data_package[2] << 8);

    int data_length = len1 + len2;

    //place the extracted data in the array
    for (int i = 0; i < data_length; i++) {
        extracted_data[i] = data_package[4+i];
    }

    return data_length;
}

int process_control_package(unsigned char package[], struct application_layer* info) {
    if (package[0] != PKG_START && package[0] != PKG_END) {
        return -1;
    }

    //process file size
    if (package[1] != FILE_SIZE) {
        printf("Error processing command package: file size is not defined");
        return -1;
    }

    int size_len = (int) package[2];
    info->file_size = 0; //makes sure the size is 0, so it can be used

    for (int i = 0; i < size_len; i++) {
        info->file_size += (int) (package[3+i] << (8*(size_len-i-1)));
    }

    //process file name
    if (package[3+size_len] != FILE_NAME) {
        printf("Error processing command package: file name is not defined");
        return -1;
    }

    int name_len = (int) package[4+size_len];

    for (int i = 0; i < name_len; i++) {
        info->file_name[i] = package[5+size_len+i];
    }

    return package[0];
}
