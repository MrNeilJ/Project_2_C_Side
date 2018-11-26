#!/bin/python

##################################################################################################
#   Name: Neil Johnson
#   Due: 11/25/18
#   Assignment: Project 1
#   Details: Create a simple FTP client
##################################################################################################

from socket import *
import sys
import os
import time


#################################################
#   CLEAR TERMINAL:
#   This will clear the terminal screen if
#   requested
#################################################
def clear_terminal():
    os.system('cls' if os.name == 'nt' else 'clear')


##################################################################################################
#   Animate_Connect
#   Simple function to allow us to animated
#   Connecting to the server (used in client)
##################################################################################################
def animate_connect(port_number):
    print("Trying to connect on port {}".format(port_number))

    for i in range(0, 5):
        time.sleep(1)
        sys.stdout.write(".")

    print("Connected!")
    time.sleep(1)
    print("\nThe server is ready to receive.\n")
    time.sleep(3)


##################################################################################################
#   GET_IP
#   This gets the IP of the current client
#   Extremely useful later in the program
#
#   Reference: https://www.geeksforgeeks.org/python-program-find-ip-address/
##################################################################################################
def get_ip():
    return gethostbyname(gethostname())


##################################################################################################
#   CONNECT_SERVER
#   This is just a simple means of connecting to the server
#   Ref: https://www.bogotobogo.com/python/python_network_programming_tcp_server_client_chat_server_chat_client_select.php
##################################################################################################
def connect_server():
    if sys.argv[3] == "-g":
        print("Server: {}\nPort: {}\nFile: {}".format(sys.argv[1], sys.argv[2], sys.argv[4]))
    else:
        print("Server: {}\nPort: {}\n".format(sys.argv[1], sys.argv[2]))

    client_socket = socket(AF_INET, SOCK_STREAM)
    client_socket.connect((sys.argv[1], int(sys.argv[2])))
    
    return client_socket


##################################################################################################
#   MAKE_SOCKET
#   The use of the function is to build the socket and prep for the connection later on
#   This layout was heavily influenced by the
#
#   Ref: https://www.tutorialspoint.com/python3/python_networking.htm
##################################################################################################
def make_socket():
    # Get total number of arguments and validate that the formatting is correct
    command_loc = arg_check()

    # Mimicked this portion from the lecture files to build the connection
    server_port = sys.argv[command_loc]
    server_socket = socket(AF_INET, SOCK_STREAM)
    server_socket.bind(('', int(server_port)))
    server_socket.listen(1)
    connection_socket, addr = server_socket.accept()
    return connection_socket


##################################################################################################
#   ARG_CHECK
#   This allows me to easily validate the application information and ensure that all the
#   correct parameters are provided when starting the call.  Errors out if not
##################################################################################################
def arg_check():
    if len(sys.argv) == 5 or len(sys.argv) == 6:
        if sys.argv[3] == "-l":
            if int(sys.argv[4]) < 0 or int(sys.argv[4]) > 65535:
                exit(1)
            else:
                total_arg = 4
        elif sys.argv[3] == "-g":
            if int(sys.argv[5]) < 0 or int(sys.argv[5]) > 65535:
                print("Data Port Number out of viable range, try again.")
                exit(1)
            else:
                total_arg = 5
    else:
        print("Something does not look right here with the arguments, try again")
        exit(1)
    
    if sys.argv[1][0:-1] != "flip":
        print("Don't forget to add the server name")
        exit(1)

    elif int(sys.argv[2]) < 0 or int(sys.argv[2]) > 65535:
        print("Port number is out of viable range, try again")
        exit(1)
    
    else:
        return total_arg


##################################################################################################
#   GET_CURRENT_DIRECTORY
#   Does exactly what it sounds like, it gets the current directory and keeps printing until
#   We run out of names to print out from the current directory.
#   Ref: https://stackoverflow.com/questions/34026077/python-recv-loop
#   Ref: https://serverfault.com/questions/9546/filename-length-limits-on-linux
##################################################################################################
def get_current_directory(current_socket):
    filename = current_socket.recv(255).decode()

    while filename:
        print(filename)
        filename = current_socket.recv(255).decode()


##################################################################################################
#   GET_SPECIFIC_FILE
#   Allows us to (if the file is found on the server) load the file into our current working
#   directory.  It does so by consistently looping over and over again.
#
#   Ref: https://stackoverflow.com/questions/34026077/python-recv-loop
#   Ref: https://realpython.com/python-sockets/
##################################################################################################
def get_specific_file(current_socket):
    print("Reached file download")
    curr_file = open(sys.argv[4], "w")
    data_set = current_socket.recv(1024).decode()
    #print(data_set)

    while data_set:
        curr_file.write(data_set)
        data_set = current_socket.recv(1024).decode()
        #print(data_set)

    print("Download of <{}> completed.".format(sys.argv[4]))


##################################################################################################
#   DATA_FETCH
#   This is the bread and butter of this program.
#   It creates the essential flow to the application from beginning to end and
#   splits the options between the -g and -l parameters properly.
##################################################################################################
def data_fetch(user_socket):
    # CHECK THAT THE PORTS AND IP ADDRESS ARE VALID LOOKING
    port_location = arg_check()
    user_socket.send(sys.argv[port_location].encode())
    user_socket.recv(1024)

    # CHECK THAT THE PARAMETERS PROVIDED ARE CORRECT
    if sys.argv[3] == "-g":
        user_socket.send("g".encode())
    elif sys.argv[3] == "-l":
        user_socket.send("l".encode())
    else:
        print("Incorrect value sent somehow")
        exit(1)

    user_socket.recv(1024)

    # SEND THE IP ADDRESS TO THE SERVER
    user_socket.send(get_ip().encode())
    user_response = user_socket.recv(1024)

    # VALIDATE THAT THE IP ADDRESS MADE IT PROPERLY
    if user_response == "fail":
        print("Something went wrong, try again.")
        exit(1)

    # SEND OVER THE FILENAME THAT WE ARE LOOKING FOR AND SEE IF IT CAN BE LOCATED
    if sys.argv[3] == "-g":
        user_socket.send(sys.argv[4].encode())
        user_response = user_socket.recv(1024).decode()

        print(user_response)

        if user_response != "Found":
            print("File does not appear to be in this directory, try again with a different file name.")
            return

    # IF WE HAVE MADE IT THIS FAR, EVERYTHING MUST BE WORKING WELL, BUILD THE SOCKET AT THIS POINT IN TIME
    current_socket = make_socket()

    # RUN THE RESPECTIVE FUNCTIONS DEPENDING ON WHAT PARAMETERS WERE SUPPLIED EARLIER
    if sys.argv[3] == "-g":
        get_specific_file(current_socket)

    if sys.argv[3] == "-l":
        get_current_directory(current_socket)

    # ONCE DONE CLOSE THE SOCKET
    current_socket.close()


##################################################################################################
#   MAIN
#   Due to pulling the majority of the information out of the main function to create a more
#   modular feel, the effect of this would be that the main function is very bare and just
#   hosts some very basic functions for the application
##################################################################################################
if __name__ == "__main__":
    clientSocket = connect_server()
    data_fetch(clientSocket)




