Shell linux in C
Authored by hagy pinkasovich
208065284

==Description==

The program is a simulation of mini shell that run on linux.

The program as close as possible to the original shell.
The program supports multiple commands separated by ; .
The program implements a prompt.
The program supports the definition of environment variables and their implementation.
The program supports of pipes commands, The program supports creating an unlimited pipeline. 
The program supports running a process in the background by using the '&' sign.
The program supports stopping a process by using C^Z, and reviving a process by using the 'bg' command.
The program supports routing values and information to a file by the command ">"

==Program DATABASE:==
To define an environment variable I used a linked list.
Each element in the list is a structure with name and value attributes, and points to the next element.
The list grows dynamically when defining new variables.
functions:
Standard linked list function.

==Program Files==
ex2.c- the file contain the program and the main
==How to compile?==
compile: gcc ex2.c -o ex2
run: ./ex2
Shell command in linux.

==Output:==
Shell output in linux.


