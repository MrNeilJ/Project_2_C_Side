#!/bin/python

#################################################
#   Name: Neil Johnson                          #
#   Due: 10/28/18                               #
#   Assignment: Project 1                       #
#   Details: Create a simple chat client        #
#################################################

from socket import *
import sys
import os
import time


def clear_terminal():
    os.system('cls' if os.name == 'nt' else 'clear')


def animate_connect(port_number):
    print("Trying to connect on port {}".format(port_number))

    for i in range(0, 5):
        time.sleep(1)
        sys.stdout.write(".")

    print("Connected!")
    time.sleep(1)
    print("\nThe server is ready to receive.\n")
    time.sleep(3)


# Reference: https://www.geeksforgeeks.org/python-program-find-ip-address/
def get_ip():
    return gethostbyname(gethostname())


def connect_server():
    
    if len(sys.argv[1]) > 5:
        serverName = sys.argv[1]
    else:
        serverName = sys.argv[1]+".engr.oregonstate.edu"
        print("The server name is: {}".format(serverName))

    print("Server: {}\nPort: {}".format(serverName, sys.argv[2]))
   
    clientSocket = socket(AF_INET, SOCK_STREAM)
    
    clientSocket.connect(("flip2", int(sys.argv[2])))
    
    return clientSocket


def make_socket():
    # Get total number of arguments and validate that the formatting is correct
    commandLoc = arg_check()
    
    # Mimicked this portion from the lecture files to build the connection
    serverPort = sys.argv[commandLoc]
    serverSocket = socket(AF_INET, SOCK_STREAM)
    serverSocket.bind(('', int(serverPort)))
    serverSocket.listen(1)
    connectionSocket, addr = serverSocket.accept()
    return connectionSocket


def arg_check():
    if len(sys.argv) == 4 or len(sys.argv) == 5:
        print("Argument Supplied: {}".format(sys.argv[3]))
        if sys.argv[3] == "-l":
            if int(sys.argv[4]) < 0 or int(sys.argv[4]) > 65535:
                print("Data Port Number out of viable range, try again.")
                exit(1)
            else:
                print("TotalArg = 4")
                totalArg = 4
        elif sys.argv[3] == "-g":
            if int(sys.argv[5]) < 0 or int(sys.argv[5]) > 65535:
                print("Data Port Number out of viable range, try again.")
                exit(1)
            else:
                totalArg = 5
        else:
            print("Incorrect formatting.  Try again.")
            exit(1)
    
    if sys.argv[1][0:-1] != "flip":
        print("Don't forget to add the server name")
        exit(1)

    elif int(sys.argv[2]) < 0 or int(sys.argv[2]) > 65535:
        print("Port number is out of viable range, try again")
        exit(1)
    
    else:
        return totalArg


def get_current_directory(currentSocket):
    filename = currentSocket.recv(100)

    while filename:
        print(filename)
        filename = currentSocket.recv(100)


def get_specific_file(currentSocket):
    currFile = open(sys.argv[4], "w")
    dataSet = currentSocket.recv(100)

    # https://stackoverflow.com/questions/34026077/python-recv-loop
    while dataSet:
        currFile.write(dataSet.decode())
        dataSet = currentSocket.recv(100)


def data_fetch(clientSocket):
    portLocation = arg_check()
    clientSocket.send(sys.argv[portLocation].encode())

    clientSocket.recv(1024)

    if sys.argv[3] == "-g":
        clientSocket.send("g".encode())
    elif sys.argv[3] == "-l":
        clientSocket.send("l".encode())
    else:
        print("Incorrect value sent somehow")
        exit(1)

    clientSocket.recv(1024)

    clientSocket.send(get_ip().encode())

    response = clientSocket.recv(1024)

    if response == "fail":
        print("Something went wrong, try again.")
        exit(1)

    if sys.argv[3] == "-g":
        clientSocket.send(sys.argv[4])
        response = clientSocket.recv(1024)

        if response != "Found":
            print("File does not appear to be in this directory, try again with a different file name.")
            return

    currentSocket = make_socket()

    if sys.argv[3] == "-g":
        get_specific_file(currentSocket)

    if sys.argv[3] == "-l":
        get_current_directory(currentSocket)

    currentSocket.close()





    


if __name__ == "__main__":
    clientSocket = connect_server()
    data_fetch(clientSocket)




