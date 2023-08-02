#include <iostream>
#include <cstring>
#include "sim_mem.h"
#include "unistd.h"

using namespace std;
char main_memory[MEMORY_SIZE];

int main() {
    char val;
    char*temp = (char *) malloc(sizeof(char) * 65);
    strcpy(temp,"/home/student/CLionProjects/sim_mem/exec_file.txt");
    temp[64]='\0';
    sim_mem mem_sm(temp, NULL, 4, 4, 8, 8, 2);
    val = mem_sm.load(1025);
//    cout << val;
    val = mem_sm.load(2);
//    cout << val;

    val = mem_sm.load(2);
//    cout << val;
    mem_sm.store(1025, 'G');
    mem_sm.store(2053, 'H');

    val = mem_sm.load(2);
    val = mem_sm.load(0);
    val = mem_sm.load(1025);
    mem_sm.store(2052, 'Y');
//
    mem_sm.store(3079, 'M');
    mem_sm.store(3078, 'G');
    mem_sm.store(3073, 'L');
    mem_sm.store(3075, 'T');
    val = mem_sm.load(2);
    val = mem_sm.load(2);
    val = mem_sm.load(2);
    val = mem_sm.load(2);
    val = mem_sm.load(1025);
    mem_sm.store(2053, 'G');
    val = mem_sm.load(2053);
    mem_sm.store(1025, 'B');
    val = mem_sm.load(1025);
    val = mem_sm.load(1025);
    val = mem_sm.load(2);
    val = mem_sm.load(2);
    val = mem_sm.load(2);
    val = mem_sm.load(2);
    val = mem_sm.load(2050);
    val = mem_sm.load(2050);
    val = mem_sm.load(3074);
    val = mem_sm.load(1025);

    mem_sm.print_memory();
    mem_sm.print_swap();
    mem_sm.print_page_table();
    free(temp);
}



// "/home/student/CLionProjects/sim_mem/exec_file.txt"