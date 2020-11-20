# Computer Networks

Repository created to host projects developed in the Computer Networks course.

The Course focuses in computer network concepts (involving all the six different layers) using the C language in Unix based operating systems.

## Exercises 

This folder stores small experiments, developed using the C language to get some practical insights on how networks work.

- **Exercise 1:** Serial Port programming -> small program that receives an input from `stdin` and sends it through the serial port, being written in the `stdout` on the other side. 
- **Exercise 2:** Logical Connection -> Implements a basic communication protocol, using a state machine. It uses messages that use a flag and a parity bit to ensure the integrity of the message. 

## connect_serial.sh

Script that was passed to me by some colleges that uses the [socat](https://linux.die.net/man/1/socat) command to link ttyS0 to ttyS1, so that we can emulate a serial port connection using a single computer for test purposes. Unfortunately I do not know who the original author of the script is.

## Projects

Projects developed to apply Computer Networks concepts in a larger scale in more complex systems. They were both developed with my partners [Miguel Rosa](https://github.com/MSR19) and [Henrique Freitas](https://github.com/Hedwig8).

### Project 1 - File transfer software

Program that implements a custom communication protocol to transfer files between two computer using the serial port. It is robust against connection failures as well as noise. For example, if the connection stops mid transfer and is resumed some time later, the file is transferred successfully anyway.


### Project 2 - FTP file transfer software

Program that transfers files from the internet using the ftp protocol. It simply needs the address of the ftp server as well as the file name and the file is downloaded.

---

**Note**:If you find some mistake in this readme or any other part of this repository, feel free to tell me about it!
