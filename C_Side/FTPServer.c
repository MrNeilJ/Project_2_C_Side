#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <fcntl.h>

#define MAXDATASIZE 500
#define MAXFILENAMELENGTH 255
#define BACKLOG 10 // how many pending connections queue will hold

/*************************************************************************************************
 *  GETUSER
 *  Simple use for the chat client.  Allows us to get the name of the user easily
 *************************************************************************************************/
void getUser(char *input) {
    printf("Type in your username > ");
    scanf("%s", input);
}

/* Got the addrinfo struct required information from the Linux  Man Pages
 Not sure if they are the same, but the seem to match up for what I needed.

 Migrated the first  IF statement from Beej.us layout into a more modular layout
 */
/************************************************************************************************
 *  SETADDRESSINFO
 *  This was pulled from Beej's Guide, just made more modular and separate from the main function
 *************************************************************************************************/
struct addrinfo *setAddressInfo(char *address, char *port) {
    struct addrinfo hints, *servinfo;
    int rv;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((rv = getaddrinfo(address, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }
    return servinfo;
}

/*************************************************************************************************
 * SETADDRESINFONOIP
 * Pulled from Beej's guide as well, validated that this could be used when no IP address was
 * supplied
 *
 * Ref: http://man7.org/linux/man-pages/man3/getaddrinfo.3.html
 *************************************************************************************************/
struct addrinfo *setAddressInfoNoIP(char *port) {
    struct addrinfo hints, *servinfo;
    int rv;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }
    return servinfo;
}

/*************************************************************************************************
 * MAKESOCKET
 * Simplified the socket process found in Beej. Pulled from main function to make the application
 * appear more modular.
 *************************************************************************************************/
int makeSocket(struct addrinfo *p) {
    int sockfd;

    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
        fprintf(stderr, "Socket could not be made at this time, try again.");
        exit(1);
    }
    return sockfd;
}

/*************************************************************************************************
 * CONNECTSOCKET
 * Pulled out  third if statement from the Beej.us program example and migrated that into a function
 * to help validate that the application properly connected to the socket we were trying to connect to.
 *************************************************************************************************/
void connectSocket(int sockfd, struct addrinfo *p) {
    int status;
    if ((status = connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)) {
        fprintf(stderr, "Socket could not be connected at this time, try again. Fails Here...");
        exit(1);
    }
}

/*
 * Removed the sending an receiving section and made it more modular and easier to read.
 * Pulled concept partially from both the example from Beej.us from the client and the server
 * side to understand how to send and receive the information
 */
void nameExchange(int sockfd, char *userName, char *serveName) {
    int sentStatus = send(sockfd, userName, strlen(userName), 0);
    int recvStatus = recv(sockfd, serveName, 10, 0);
}

/*
 * Bread and butter on how this works.  Found some examples online from Geeks for Geeks
 * about client / server examples in C.
 *
 * Pulled the concepts half from the client example of Beej.us and the other half from
 * the server side.  To create the sense of communication back and forth
 */
void chatWithServer(int sockfd, char *username, char *servername) {
    char input[MAXDATASIZE];
    char output[MAXDATASIZE];

    int total_bytes = 0;
    int status;

    memset(input, 0, sizeof(input));
    memset(output, 0, sizeof(input));

    fgets(input, MAXDATASIZE, stdin);

    while (1) {
        printf("%s> ", username);
        fgets(input, MAXDATASIZE, stdin);

        if (strcmp(input, "\\quit\n") == 0) {
            break;
        }

        if (strlen(input) > MAXDATASIZE - 1) {
            // Re did so it shouldn't ever proc this.  But still good to have..?
            printf("Too big, try again");
        } else {
            // Send input to server
            total_bytes = send(sockfd, input, strlen(input), 0);

            // Check status of sending to server
            if (total_bytes == -1) {
                fprintf(stderr, "Couldn't send information properly, try again");
                exit(1);
            }

            // Receive information from user
            status = recv(sockfd, output, MAXDATASIZE - 1, 0);

            // Check status of receiving from server
            if (status == -1) {
                fprintf(stderr, "Couldn't receive the information from the server, try again");
                exit(1);
            } else if (status == 0) {
                printf("Connection closed safely by server.");
                break;
            }
                // Print out response
            else {
                printf("%s> %s\n", servername, output);
            }
            // Clear out buffer for standard input so we don't get erroneous values
            setbuf(stdin, NULL);
        }

        memset(input, 0, sizeof(input));
        memset(output, 0, sizeof(output));

    }
    // Close socket once done communicating
    close(sockfd);
    printf("Closing Connection");
}

/*************************************************************************************************
 * BINDSOCKET
 * Binds the socket
 * Pulled from lines 63-65 of Beej's Networking Example
 *************************************************************************************************/
void bindSocket(int sockfd, struct addrinfo * p) {
    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
        fprintf(stderr, "Socket could not be properly binded, closing application.");
        exit(1);
    }
}

/*************************************************************************************************
 * LISTENSOCKET
 * Pulled from lines 75-77 of Beej's Networking Example
 *************************************************************************************************/
void listenSocket(int sockfd) {
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        close(sockfd);
        exit(1);
    }
}

/*************************************************************************************************
 * GETDIRECTORYFILES
 * Preps and sends the directory information to the client
 *
 * Ref: https://www.geeksforgeeks.org/c-program-list-files-sub-directories-directory/
 * Ref: http://pubs.opengroup.org/onlinepubs/7990989775/xsh/readdir.html
 *************************************************************************************************/
int getDirectoryFiles(char** dirFiles) {
    struct dirent *de;  // Pointer for directory entry
    DIR *dr = opendir(".");
    int i = 0;

    if (dr == NULL)  // opendir returns NULL if couldn't open directory
    {
        printf("Could not open current directory" );
        return 0;
    }

    while ((de = readdir(dr)) != NULL)
        if (de->d_type == DT_REG){
            strcpy(dirFiles[i], de->d_name);
            i++;
        }
    closedir(dr);

    return i;
}

/*************************************************************************************************
 *  TEMPSTRINDIRECTORY
 *  This is used in order to make it easy to build an array of "C-Like Strings" for our directory
 *  function to utilize later when trying to print these out
 *
 *  ref: https://stackoverflow.com/questions/8824057/pointer-to-string-array-in-c
 *************************************************************************************************/
char ** tempStringDirectory(int n) {
    char ** str_ptr = malloc(n * sizeof(char *));
    int i;
    for(i = 0; i < n; i++) {
        str_ptr[i] = malloc(100*sizeof(char));
        memset(str_ptr[i], 0, sizeof(str_ptr[i]));
    }
    return str_ptr;
}

/*************************************************************************************************
 *  FREESTRINGDIRECTORY
 *  Does the opposite of the Tempstringdirectory.  It frees all the memory allocated for the
 *  pointer array of strings that it intially built.
 *************************************************************************************************/
void freeStringDirectory(char ** array, int n) {
    int i;
    for(i = 0; i < n; i++) {
        free(array[i]);
    }
    free(array);
}

/*************************************************************************************************
 *  FILESEARCH
 *  Does exactly what it sounds like.  It goes through the previously mentioned string set and
 *  validates that the file in question exists in the current set of strings.
 *
 *  Ref: https://stackoverflow.com/questions/8004237/how-do-i-properly-compare-strings
 *************************************************************************************************/
int fileSearch(char ** fileDir, int numFile, char* fileName) {
    int found = 0;
    int i;

    for (i = 0; i < numFile; i++) {
        if (strcmp(fileDir[i], fileName) == 0) {
            found = 1;
        }
    }
    return found;
}


/*************************************************************************************************
 *  SENDINGFILE
 *  Bridges the connection between the client and server fully, once done it sends the file
 *  that has been requested by the client.
 *
 *  Ref: https://stackoverflow.com/questions/2014033/send-and-receive-a-file-in-socket-programming-in-linux-with-c-c-gcc-g
 *************************************************************************************************/
void sendingFile(char * address, char * port, char * filename){
   // struct addrinfo *res = setAddressInfo(argv[1], argv[2]);
    sleep(2);
    printf("\nSending file %s, to %s\n", filename, address);

    // Get the address information for the server.
    struct addrinfo *res = setAddressInfo(address, port);

    int sockfd = makeSocket(res);

    connectSocket(sockfd, res);

    char buffer[1000];
    memset(buffer, 0, sizeof(buffer));

    int fileDirectory = open(filename, O_RDONLY);

    while(1) {

        int bytes_read = read(fileDirectory, buffer, sizeof(buffer) - 1);
        if (bytes_read == 0) // We're done reading from the file
            break;

        if (bytes_read < 0) {
            // handle errors
            fprintf(stderr, "Cannot locate file, try again.");
            return;
        }

        void *p = buffer;
        int bytes_written;
        while (bytes_read > 0) {
            if (sizeof(buffer) > bytes_read) {
                bytes_written = send(sockfd, p, bytes_read, 0);
            }
            else{
                bytes_written = send(sockfd, p, sizeof(buffer), 0);
            }
            if (bytes_written < 0) {
                // handle errors
                fprintf(stderr, "Error reading information into buffer.");
                return;
            }
            bytes_read -= bytes_written;
            p += bytes_written;
        }
        printf("File <%s> was successfully sent.\n\n", filename);
        memset(buffer, 0, sizeof(buffer));
    }

    memset(buffer, 0, sizeof(buffer));



    close(sockfd);
    freeaddrinfo(res);
}

// REDO THIS SECTION LATER
/*************************************************************************************************
 *  SENDFULLDIRECTORY
 *  Bridges the connection between the client and server fully, once done it sends the file
 *  that has been requested by the client. In this case the file is just a list of the full directory
 *  Referenced the Beej's guide, and then just looped through a sending flurry
 *
 *  Ref: https://stackoverflow.com/questions/2014033/send-and-receive-a-file-in-socket-programming-in-linux-with-c-c-gcc-g
 *************************************************************************************************/
void sendFullDirectory(char * address, char * port, char ** files, int totalFiles){
    struct addrinfo *res = setAddressInfo(address, port);
    int sockfd = makeSocket(res);
    connectSocket(sockfd, res);
    int i;

    for (i = 0; i < totalFiles; i++){
        send(sockfd, files[i], 100 ,0);
    }

    close(sockfd);
    freeaddrinfo(res);
}

/*************************************************************************************************
 *  BUILDCONNECTION
 *  The bread and butter for this setup.  It allows us to easily validate all information that is
 *  being sent and received between the client and server.  It then deciphers the information
 *  provided by the client and the calls the correct functions allowing us to either send
 *  a file to the user or a list of the items in the directory.
 *
 *  Largely referenced that page as it assisted in my understanding that you need to pair each
 *  send with a proper receive and clean the memory out just in case the memory was used previously
 *  Ref: https://stackoverflow.com/questions/19794764/linux-socket-how-to-make-send-wait-for-recv
 *************************************************************************************************/
void buildConnection(int new_fd){
    char* pass = "pass";
    char* fail = "fail";
    char port[100];
    char command[500];
    char ipAddress[100];

    // Get confirmation that we are able to successfully connect to the client
    memset(port, 0, sizeof(port));
    recv(new_fd, port, sizeof(port)-1, 0 );
    send(new_fd, pass, strlen(pass), 0);

    // Get the command type of what we are going to send the client (G or L)
    memset(command, 0, sizeof(command));
    recv(new_fd, command, sizeof(command)-1, 0);
    send(new_fd, pass, strlen(pass), 0);

    // Get the IP address from the client
    memset(ipAddress, 0, sizeof(ipAddress));
    recv(new_fd, ipAddress, sizeof(ipAddress)-1, 0);
    send(new_fd, pass, strlen(pass), 0);

    printf("Client information:\nIP:%s \nPORT:%s \n", ipAddress, port);

    // Now to check all the information that we just received so we can send back all the correct information.
    if (strcmp(command, "g") == 0) {
        //send(new_fd, pass, strlen(pass), 0);

        // Get the name of the file that we are looking for
        char filename[100];
        memset(filename, 0, sizeof(filename));
        recv(new_fd, filename, sizeof(filename)-1, 0);

        // Get all the current files in our directory
        char** fileSet = tempStringDirectory(500);
        int numFile = getDirectoryFiles(fileSet);
        int locateFile = fileSearch(fileSet, numFile, filename);

        if (locateFile) {
            char* found = "Found";
            send(new_fd, found, strlen(found), 0);

            char req_filename[100];
            memset(req_filename, 0, sizeof(req_filename));
            strcpy(req_filename, "./");
            char * end = req_filename + strlen(req_filename);
            end += sprintf(end, "%s", req_filename);

            sendingFile(ipAddress, port, filename);
        }

        else {
            printf("File Not Found");
            char * notFound = "File not found";
            send(new_fd, notFound, 100, 0);
            exit(1);
        }

        freeStringDirectory(fileSet, 500);
    }

    else if (strcmp(command, "l") == 0) {
        send(new_fd, pass, strlen(pass), 0);
        char** files = tempStringDirectory(500);

        int numFile = getDirectoryFiles(files);

        sendFullDirectory(ipAddress, port, files, numFile);

        freeStringDirectory(files, 500);
    }
    else {
        send(new_fd, fail, strlen(fail), 0);
        fprintf(stderr, "Failure, try again.");
    }
}

/*************************************************************************************************
 *  CLIENT CONNECT
 *  Connects the client to the server.
 *  Referenced Beej's Guide for this section.
 *************************************************************************************************/
void clientConnect(int sockfd) {
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    int new_fd;

    while(1) {
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }
        buildConnection(new_fd);
        close(new_fd);
    }

}
/*************************************************************************************************
 *  MAIN
 *  Pulled from the Beej's Guide, just sets the application up for the best possible service
 *************************************************************************************************/
int main(int argc, char *argv[]) {
    // Validate that we have the correct number of arguments before proceeding with the rest of the setup
    if (argc != 2) {
        fprintf(stderr, "Not the correct amount of arguments supplied.  Re-run the application");
    }

    // Get the address information for the server.
    struct addrinfo *res = setAddressInfoNoIP(argv[1]);

    int sockfd = makeSocket(res);

    //connectSocket(sockfd, res);
    bindSocket(sockfd, res);
    listenSocket(sockfd);
    printf("Starting Server on port #: %s\n", argv[1]);
    clientConnect(sockfd);

    //chatWithServer(sockfd, username, servername);

    freeaddrinfo(res);


}