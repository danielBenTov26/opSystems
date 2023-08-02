

#ifndef SIM_MEM
#define SIM_MEM

#include <string>

#define MEMORY_SIZE 200 // TODO back to 200!!!
     extern char main_memory[MEMORY_SIZE];
using namespace std;

 typedef struct page_descriptor
 {
     bool valid;
     int frame;
     bool dirty;
     int swap_index;
     int time;
 }page_descriptor;



class sim_mem {
 int swapfile_fd; //swap file fd
 int program_fd; //executable file fd
 int text_size;
 int data_size;
 int bss_size;
 int heap_stack_size;
// int num_of_pages;
 int page_size;
 int *memoryFrame;
    int *swapFrame;
    int swap_file_size;
 char *zeroFrame;

 page_descriptor **page_table; //pointer to page table
 public:

         sim_mem(char exe_file_name[],char  swap_file_name[], int text_size,
                          int data_size, int bss_size,
                          int heap_stack_size ,int page_size);

 ~sim_mem();

 char load(int address);
 void store(int address, char value);
 void print_memory();
 void print_swap ();
 void print_page_table();
 static int binaryToDecimal(int binary);
 string decimalToBinary(int num);
 int load_to_maimMemory(char * page);
bool validAddress( int p1, int p2,int offset);
int clear_main_memory();
};
#endif