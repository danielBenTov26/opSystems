Memory simulator
Authored by Daniel ben tov
ID: 208680413

==Description==

The program is a simulation of processor approaches to memory (RAM), We use the mechanism "paging" that allow running a program when only a portion of it is in the memory.
The virtual memory is divided into pages ,this pages are brought to the main memory (RAM) by need.
In this program we use one proccess as the virtual memory, the simulation will be implemented by  two main approaches,load an address to the main memory and store address in the main memory by using the Hard disk.

==Program DATABASE:
struct database that contain sub databases, the sub databases are:
1.page_table=array of structs, the page table serves as a table of contents ,we can get information about the RAM,address,and swap file.
2.swap_fd= file descriptor,hold the access to the swap file , the swap file simulate the Hard disk .
3.program_fd= file descriptor,hold the access to the executable file, this file simulate a process! 
4.main_memory=array of char size 200, this array simulate the RAM (random access memory).

==functions:
two main functions:
1.load- this method receive an address and a Database , and ensure that the requested address will be in the main memory.
2.store- this method is very similar to "load" function,receive an address, Database and a value,
to store the address in the RAM the method need to allocate memory (same as "malloc"), and insert the value in the RAM.

Other relevant functions:
3. load_to_maimMemory - get the data on the page that  he needs to load and put him in free place in the main memory and return the index of the new page that he put in the main memory.
4. clear_main_memory - if the previos func didnt find a free place in main memory she call to clear func that delete the oldest frame on memory by LRU algoritem and retuen the index of the frame she clear, if the oldest frame is dirty the func move him to swap file.

==Program Files==
mem_sim.c- the file contain only functions
mem_sim.h- an header file ,contain structs,declerations of functions.
main.c- contain the main only.
Makefile-to compile the program.

==How to compile?==
compile: gcc ex4.c -o ex4
run: ./ex4

==Input:==
no input

==Output:==
main memory (RAM)
swap file
char (that the load function return)


