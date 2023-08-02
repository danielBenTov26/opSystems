//
// Created by student on 6/5/23.
//
#include <cstdio>
#include <cstdlib>
#include <csignal>
#include "sim_mem.h"
#include <fstream>
#include <cstring>
#include <fcntl.h>
#include <stdexcept>
#include <cmath>
#include <iostream>


int num_of_txt_pages, num_of_data_pages, num_of_stack_heap_pages, num_of_bss_pages, num_of_pages_in_mainMemory;
int globalTime = 0;

using namespace std;

sim_mem::sim_mem(char exe_file_name[], char swap_file_name[], int text_size, int data_size, int bss_size,
                 int heap_stack_size, int page_size) {

    this->text_size = text_size;
    this->data_size = data_size;
    this->bss_size = bss_size;
    this->heap_stack_size = heap_stack_size;
    this->page_size = page_size;

    num_of_txt_pages = text_size / page_size;
    num_of_data_pages = data_size / page_size;
    num_of_bss_pages = bss_size / page_size;
    num_of_stack_heap_pages = heap_stack_size / page_size;
    num_of_pages_in_mainMemory = MEMORY_SIZE / page_size;

    memoryFrame = new int[num_of_pages_in_mainMemory];
    for (int i = 0; i < num_of_pages_in_mainMemory; ++i) {
        memoryFrame[i] = 0;
    }


zeroFrame = new char[page_size+1];
    for (int i = 0; i < page_size; ++i) {
        zeroFrame[i] = '0';
    }
zeroFrame[page_size] = '\0';

    this->program_fd = open(exe_file_name, O_RDONLY);
    if (program_fd == -1) {
        throw std::runtime_error("ERR");
    }

    if (swap_file_name == nullptr) {
        swap_file_name = (char*)"my_new_swap.txt";
    }
    this->swapfile_fd = open(swap_file_name, O_RDWR | O_CREAT | O_TRUNC , 0666);
    if (swapfile_fd == -1) {
        // Error occurred while opening the file, handle the error
        // For example, you can throw an exception
        throw std::runtime_error("ERR");
    }
    swap_file_size = data_size + bss_size + heap_stack_size;
    char zero = '0';
    int bytes_written;
    for (int i = 0; i < swap_file_size; ++i) {
        bytes_written = write(swapfile_fd, &zero, sizeof(zero));
        if (bytes_written == -1)
            throw std::runtime_error("ERR");
    }

    swapFrame = new int[swap_file_size / page_size];
    for (int i = 0; i < swap_file_size / page_size; ++i) {
        swapFrame[i] = 0;

    }
    page_table = new page_descriptor *[4];
    page_table[0] = new page_descriptor[num_of_txt_pages];
    page_table[1] = new page_descriptor[num_of_data_pages];
    page_table[2] = new page_descriptor[num_of_bss_pages];
    page_table[3] = new page_descriptor[num_of_stack_heap_pages];

    for (int i = 0; i < num_of_txt_pages; ++i) {
        page_table[0][i].dirty = false;
        page_table[0][i].valid = false;
        page_table[0][i].frame = -1;
        page_table[0][i].swap_index = -1;
        page_table[0][i].time = -1;
    }
    for (int i = 0; i < num_of_data_pages; ++i) {
        page_table[1][i].dirty = false;
        page_table[1][i].valid = false;
        page_table[1][i].frame = -1;
        page_table[1][i].swap_index = -1;
        page_table[1][i].time = -1;
    }
    for (int i = 0; i < num_of_bss_pages; ++i) {
        page_table[2][i].dirty = false;
        page_table[2][i].valid = false;
        page_table[2][i].frame = -1;
        page_table[2][i].swap_index = -1;
        page_table[2][i].time = -1;
    }
    for (int i = 0; i < num_of_stack_heap_pages; ++i) {
        page_table[3][i].dirty = false;
        page_table[3][i].valid = false;
        page_table[3][i].frame = -1;
        page_table[3][i].swap_index = -1;
        page_table[3][i].time = -1;
    }

    std::memset(main_memory, '0', MEMORY_SIZE);


}


char sim_mem::load(int address) {
    globalTime++;
    string binNumStr;
    binNumStr = decimalToBinary(address);
    string substring = binNumStr.substr(0, 2);
    int ind_num_of_First_table_in_bin = stoi(substring);
    int index_Of_p1 = binaryToDecimal(ind_num_of_First_table_in_bin);
    int ofsetSize = log2(page_size);
    substring = binNumStr.substr(2, (10 - ofsetSize));
    int num_of_p2 = stoi(substring);
    num_of_p2 = binaryToDecimal(num_of_p2);
    substring = binNumStr.substr((2 + (10 - ofsetSize)), ofsetSize);
    int num_of_offset = stoi(substring);
    num_of_offset = binaryToDecimal(num_of_offset);
    char* bufferPage = new char[page_size + 1];
    strcpy(bufferPage,zeroFrame);
    bufferPage[page_size] = '\0';
    char result;
    if(!validAddress(index_Of_p1,num_of_p2,num_of_offset))
    { cout << "ERR\n";
        return '\0';}
    if (page_table[index_Of_p1][num_of_p2].valid) {
        result = main_memory[page_table[index_Of_p1][num_of_p2].frame * page_size + num_of_offset];

    } else {
        if (index_Of_p1 == 0) { // txt case
           ssize_t  x = pread(program_fd, bufferPage, page_size, num_of_p2 * page_size);
           if( x == -1)
           {
               cout << " ERR\n";
           }
            page_table[index_Of_p1][num_of_p2].frame = load_to_maimMemory(
                    bufferPage); // retuen the index frame in main memory
            result = main_memory[page_table[index_Of_p1][num_of_p2].frame * page_size + num_of_offset];


        } else { // other cases if is dirty bring from swep else we check if its valid
            if (page_table[index_Of_p1][num_of_p2].dirty) {
                int swapInd = page_table[index_Of_p1][num_of_p2].swap_index;
                ssize_t  x = pread(swapfile_fd, bufferPage, page_size, swapInd * page_size);
                if( x == -1)
                {
                    cout << " ERR\n";
                }
                ssize_t  y = pwrite(swapfile_fd, zeroFrame, page_size, swapInd * page_size);
                if( y == -1)
                {
                    cout << " ERR\n";
                }

                swapFrame[swapInd] = 0;
                page_table[index_Of_p1][num_of_p2].frame = load_to_maimMemory(
                        bufferPage);
                page_table[index_Of_p1][num_of_p2].swap_index = -1;
                result = main_memory[page_table[index_Of_p1][num_of_p2].frame * page_size + num_of_offset];

            } else {
                if (index_Of_p1 == 3) { // heap_stack case
                    cout << ("ERR\n");//this ver not exists in memory - you try to exes to var from 'heap_stack'
                    result = '\0';
                } else {

                    if (index_Of_p1 == 1) {
                        ssize_t  x = pread(program_fd, bufferPage, page_size, text_size + num_of_p2 * page_size);
                        if( x == -1)
                        {
                            cout << " ERR\n";
                        }
                    } else {
                        ssize_t  x =  pread(program_fd, bufferPage, page_size,
                                            data_size + text_size + num_of_p2 * page_size);
                        if( x == -1)
                        {
                            cout << " ERR\n";
                        }


                    }
                    page_table[index_Of_p1][num_of_p2].frame = load_to_maimMemory(
                            bufferPage);
                    result = main_memory[page_table[index_Of_p1][num_of_p2].frame * page_size + num_of_offset];
                }
            }
        }
    }
    delete[] bufferPage;
    page_table[index_Of_p1][num_of_p2].valid = true;
    page_table[index_Of_p1][num_of_p2].time = globalTime;
    return result;
}

void sim_mem::store(int address, char value) {
    globalTime++;
    string binNumStr;
    binNumStr = decimalToBinary(address);
    string substring = binNumStr.substr(0, 2);
    int ind_num_of_First_table_in_bin = stoi(substring);
    int index_Of_p1 = binaryToDecimal(ind_num_of_First_table_in_bin);
    int ofsetSize = log2(page_size);
    substring = binNumStr.substr(2, (10 - ofsetSize));
    int num_of_p2 = stoi(substring);
    num_of_p2 = binaryToDecimal(num_of_p2);
    substring = binNumStr.substr((2 + (10 - ofsetSize)), ofsetSize);
    int num_of_offset = stoi(substring);
    num_of_offset = binaryToDecimal(num_of_offset);
    char* bufferPage= new char[page_size+1];
    strcpy(bufferPage,zeroFrame);
    bufferPage[page_size] = '\0';
    if(!validAddress(index_Of_p1,num_of_p2,num_of_offset))
    { cout << "ERR\n";
        return;}

    if (index_Of_p1 == 0) {
        cout << "STORE_TEXT_ERR\n";
    } else {

        if (!page_table[index_Of_p1][num_of_p2].valid) {
            if (page_table[index_Of_p1][num_of_p2].dirty) {
                int swapInd = page_table[index_Of_p1][num_of_p2].swap_index;
                ssize_t  x =  pread(swapfile_fd, bufferPage, page_size, swapInd * page_size);
                if( x == -1)
                {
                    cout << " ERR\n";
                }
                ssize_t y =  pwrite(swapfile_fd, zeroFrame, page_size, swapInd * page_size);
                if( y == -1)
                {
                    cout << " ERR\n";
                }
                swapFrame[swapInd] = 0;
                page_table[index_Of_p1][num_of_p2].frame = load_to_maimMemory(
                        bufferPage);
                page_table[index_Of_p1][num_of_p2].swap_index = -1;
            } else {
                if (index_Of_p1 == 3) // load the heap_stack from program to buffer - puts zeros in main memory
                {
                    for (int i = 0; i < page_size; ++i) {
                        bufferPage[i] = '0';
                    }
                } else if (index_Of_p1 == 2) { // load the bss from program to buffer
                    ssize_t  x = pread(program_fd, bufferPage, page_size,
                          (num_of_txt_pages + num_of_data_pages + num_of_p2) * page_size);
                    if( x == -1)
                    {
                        cout << " ERR\n";
                    }
                } else if (index_Of_p1 == 1) // load the data from program to buffer
                {
                    ssize_t  x = pread(program_fd, bufferPage, page_size, (num_of_txt_pages + num_of_p2) * page_size);
                    if( x == -1)
                    {
                        cout << " ERR\n";
                    }
                }
                page_table[index_Of_p1][num_of_p2].frame = load_to_maimMemory(bufferPage);

            }

        }
        page_table[index_Of_p1][num_of_p2].dirty = true;
        page_table[index_Of_p1][num_of_p2].valid = true;
        main_memory[page_table[index_Of_p1][num_of_p2].frame * page_size + num_of_offset] = value;
        page_table[index_Of_p1][num_of_p2].time = globalTime;
        delete[] bufferPage;

    }
}


int sim_mem::load_to_maimMemory(char* page) { // put the page to free frame in main memory & return page.frame
    for (int i = 0; i < num_of_pages_in_mainMemory; ++i) {
        if (memoryFrame[i] == 0) {
            for (int j = 0; j < page_size; ++j) {
                main_memory[i*page_size + j] = page[j];
            }
            memoryFrame[i] = -1;
            return i;
        }
    }
//     memory full , clear same please and put him the page
    int index = clear_main_memory();
    for (int i = 0; i < page_size; ++i) {
        main_memory[index*page_size +i] = page[i];
    }
    memoryFrame[index] = -1;

    return index;
}

int sim_mem::clear_main_memory() {
    int indP1Tmp, indP2Tmp, timeTmp;
    timeTmp = globalTime;
    for (int i = 0; i < num_of_txt_pages; i++) {
        if (page_table[0][i].valid && page_table[0][i].time <= timeTmp) {
            timeTmp = page_table[0][i].time;
            indP1Tmp = 0;
            indP2Tmp = i;

        }
    }
    for (int i = 0; i < num_of_data_pages; i++) {
        if (page_table[1][i].valid && page_table[1][i].time <= timeTmp) {
            timeTmp = page_table[1][i].time;
            indP1Tmp = 1;
            indP2Tmp = i;
        }
    }

    for (int i = 0; i < num_of_bss_pages; i++) {
        if (page_table[2][i].valid && page_table[2][i].time <= timeTmp) {
            timeTmp = page_table[2][i].time;
            indP1Tmp = 2;
            indP2Tmp = i;
        }
    }

    for (int i = 0; i < num_of_stack_heap_pages; i++) {
        if (page_table[3][i].valid && page_table[3][i].time <= timeTmp) {
            timeTmp = page_table[3][i].time;
            indP1Tmp = 3;
            indP2Tmp = i;
        }
    }

    if (indP1Tmp == 0 || !page_table[indP1Tmp][indP2Tmp].dirty) {                        // txt - delete the page from mainMemory
        page_table[indP1Tmp][indP2Tmp].swap_index = -1;
    } else {                                   //data/bss/heap_stack

        char *frameTemp = new char[page_size+1];
        strncpy(frameTemp, main_memory + page_table[indP1Tmp][indP2Tmp].frame * page_size, page_size);
        for (int i = 0; i < swap_file_size / page_size; ++i) {
            if (swapFrame[i] == 0) {
                swapFrame[i] = -1;
                ssize_t y = pwrite(swapfile_fd, frameTemp, page_size, i * page_size);
                if(y == -1)
                {
                    cout << "ERR\n";
                }
                page_table[indP1Tmp][indP2Tmp].swap_index = i;
                delete[] frameTemp;
                break;
            }
        }
    }

    int frameIndx = page_table[indP1Tmp][indP2Tmp].frame;
    for (int i = 0; i < page_size; ++i) {
        main_memory[frameIndx * page_size + i] = '0';
    }

    strncpy(main_memory + frameIndx * page_size , zeroFrame, page_size);
    memoryFrame[frameIndx] = 0;
    page_table[indP1Tmp][indP2Tmp].frame = -1;
    page_table[indP1Tmp][indP2Tmp].valid = false;
    page_table[indP1Tmp][indP2Tmp].time = -1;
    return frameIndx;

}


/**************************************************************************************/
void sim_mem::print_memory() {
    int i;
    printf("\n Physical memory\n");
    for(i = 0; i < MEMORY_SIZE; i++) {
        printf("[%c]\n", main_memory[i]);
    }
}
/************************************************************************************/
void sim_mem::print_swap() {
    char* str = (char*)malloc(this->page_size *sizeof(char));
    int i;
    printf("\n Swap memory\n");
    lseek(swapfile_fd, 0, SEEK_SET); // go to the start of the file
    while(read(swapfile_fd, str, this->page_size) == this->page_size) {
        for(i = 0; i < page_size; i++) {
            printf("%d - [%c]\t", i, str[i]);
        }
        printf("\n");
    }
    free(str);
}
/***************************************************************************************/
void sim_mem::print_page_table() {
    int i;
    num_of_txt_pages = text_size / page_size;
    num_of_data_pages = data_size / page_size;
    num_of_bss_pages = bss_size / page_size;
    num_of_stack_heap_pages = heap_stack_size / page_size;
    printf("Valid\t Dirty\t Frame\t Swap index\n");
    for(i = 0; i < num_of_txt_pages; i++) {
        printf("[%d]\t[%d]\t[%d]\t[%d]\n",
               page_table[0][i].valid,
               page_table[0][i].dirty,
               page_table[0][i].frame ,
               page_table[0][i].swap_index);

    }
    printf("Valid\t Dirty\t Frame\t Swap index\n");
    for(i = 0; i < num_of_data_pages; i++) {
        printf("[%d]\t[%d]\t[%d]\t[%d]\n",
               page_table[1][i].valid,
               page_table[1][i].dirty,
               page_table[1][i].frame ,
               page_table[1][i].swap_index);

    }
    printf("Valid\t Dirty\t Frame\t Swap index\n");
    for(i = 0; i < num_of_bss_pages; i++) {
        printf("[%d]\t[%d]\t[%d]\t[%d]\n",
               page_table[2][i].valid,
               page_table[2][i].dirty,
               page_table[2][i].frame ,
               page_table[2][i].swap_index);

    }
    printf("Valid\t Dirty\t Frame\t Swap index\n");
    for(i = 0; i < num_of_stack_heap_pages; i++) {
        printf("[%d]\t[%d]\t[%d]\t[%d]\n",
               page_table[3][i].valid,page_table[3][i].dirty,page_table[3][i].frame ,page_table[3][i].swap_index);


    }
}



string sim_mem::decimalToBinary(int num) {
    std::string binary = "";
    // Calculate the binary representation
    while (num > 0 || binary.length() < 12) {
        binary = ((num % 2) == 0 ? "0" : "1") + binary;
        num /= 2;
    }
    // Add leading zeros if necessary
    while (binary.length() < 12) {
        binary = "0" + binary;
    }
    return binary;
}

int sim_mem::binaryToDecimal(int binary) {
    int decimal = 0;
    int placeValue = 0;
    while (binary > 0) {
        int digit = binary % 10;
        decimal += digit * pow(2, placeValue);
        binary /= 10;
        placeValue++;
    }
    return decimal;
}

sim_mem::~sim_mem() {
    for (int i = 0; i < 4; ++i) {
        delete[] page_table[i];
    }
    delete[] page_table;
    delete[] zeroFrame;
    delete[] memoryFrame;
    delete[] swapFrame;

    close(swapfile_fd);
    close(program_fd);
}

bool sim_mem::validAddress(int p1, int p2, int offset) {
    if(p1 < 0 || p1 >= 4)
        return false;
    if(p1 == 0 && (p2 < 0 || p2 >= num_of_txt_pages))
        return false;
    if(p1 == 1 && (p2 < 0 || p2 >= num_of_data_pages))
        return false;
    if(p1 == 2 && (p2 < 0 || p2 >= num_of_bss_pages))
        return false;
    if(p1 == 3 && (p2 < 0 || p2 >= num_of_stack_heap_pages))
        return false;
    if(offset < 0 || offset >= page_size)
        return false;
    return true;
}







