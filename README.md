# ddmon: Deadlock Detect Monitor

ddmon is a C shared library that detects whether a deadlock has occurred during program execution and displays information to help debugging.<br><br>
If the program does not end for a long time, it is hard to tell if a deadlock has occurred or if the program is merely taking too long time to execute.
ddmon uses runtime interpositioning to graph the relationships between mutexes and notify the programmer as soon as a deadlock occurs.
<br>

## Install
Use Makefile to build ddmon.so and ddchck.
```bash
$ make
```
You can also build a program using commands behind.
```bash
$ gcc -o ddchck ddchck.c -pthread
$ gcc -shared -fPIC -o ddmon.so ddmon.c -ldl
```
This process will make two files: ddmon.so and ddchck.<br>
ddmon.so is a shared library to interposition on pthread_mutex_lock() in target program.
## How to use

## If something goes wrong