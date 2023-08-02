
Authored by 
Daniel Bentov

(ID)208680413

==Description==

This program is a basic calculator for polynoms implemented in C that can run commands entered by the user


==Program DATABASE:==
struct database that contain the shared mem and the threads
This program performs various operations on polynomials, including addition, multiplication, and printing the result. It also includes functionality to store polynomials in shared memory and perform operations using threads
The program defines several functions


functions:
add,Sub & MUL function.

==Program Files==
Ex3q1.c Ex3q2a.c, Ex3q2b.c, Ex3q3A.c, Ex3q3B.c

==How to compile?==
compile: gcc Ex3q1.c -o Ex3q1
run: ./Ex3q1.c

compile: gcc Ex3q2a.c -o writer
run: .writer.c  

compile: gcc Ex3q2b.c -o reader
run: .reader.c

compile: gcc Ex3q3A.c -o writer -lpthread
run: .writer.c  

compile: gcc Ex3q2b.c -o reader -lpthread
run: .reader.c

Shell command in linux.

==Output:==
Shell output in linux.


