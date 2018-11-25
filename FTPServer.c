#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <dirent.h>

#define MAXDATASIZE 500
#define MAXFILENAMELENGTH 255
#define BACKLOG 10 // how many pending connections queue will hold


void getUser(char *input) {
    printf("Type in your username > ");
    scanf("%s", input);
}

/* Got the addrinfo struct required information from the Linux  Man Pages
 Not sure if they are the same, but the seem to match up for what I needed.

 Migrated the first  IF statement from Beej.us layout into a more modular layout
 */
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

/*
 * Simplified the socket process found in Beej.us since I do not need to loop
 * through and find the compatible socket as I am hard coding that in essentially
 * using the command line when starting the application.
 *
 * Pulled that loop out and made it into a function for more modularity
 */
int makeSocket(struct addrinfo *p) {
    int sockfd;

    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
        fprintf(stderr, "Socket could not be made at this time, try again.");
        exit(1);
    }
    return sockfd;
}

/*
 * Pulled out  third if statement from the Beej.us program example and migrated that into a function
 * to help validate that the application properly connected to the socket we were trying to connect to.
 */
void connectSocket(int sockfd, struct addrinfo *p) {
    int status;
    if ((status = connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)) {
        fprintf(stderr, "Socket could not be connected at this time, try again.");
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

// Pulled frome lines 63-65 of Beej's Networking Example
void bindSocket(int sockfd, struct addrinfo * p) {
    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
        fprintf(stderr, "Socket could not be properly binded, closing application.");
        exit(1);
    }
}

// Pulled from lines 75-77 of Beej's Networking Example
void listenSocket(int sockfd) {
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        close(sockfd);
        exit(1);
    }
}

/******************************************************
 * PRINTING OUT THE FILE NAMES
 * Found two variations of this process and will implement
 * whichever seems to best fit the need of this prorgram
 *
 * Come back to this later to find the best version
 *******************************************************/
int getDirectory(char** files){         //We need to get all of the files in the directory
    DIR* d;                                 //Structure for this function was found here: https://goo.gl/oqbjTv
    struct dirent * dir;
    d = opendir(".");
    int i = 0;
    if (d){
        while ((dir = readdir(d)) != NULL){     //While there are still things to read, read in the file names
            if (dir->d_type == DT_REG){
                strcpy(files[i], dir->d_name);
                i++;
            }
        }
        closedir(d);
    }
    return i;
}

// Personally I feel that this one is a little bit cleaner and will most likely use this one instead.
int getDirectoryFiles(char** dirFiles) {
    struct dirent *de;  // Pointer for directory entry

    // opendir() returns a pointer of DIR type.
    DIR *dr = opendir(".");

    if (dr == NULL)  // opendir returns NULL if couldn't open directory
    {
        printf("Could not open current directory" );
        return 0;
    }

    // Refer http://pubs.opengroup.org/onlinepubs/7990989775/xsh/readdir.html
    // for readdir()
    int i = 0;
    while ((de = readdir(dr)) != NULL)
        if (de->d_type == DT_REG){
            strcpy(dirFiles[i], de->d_name);
            i++;
        }

    closedir(dr);
    return i;
}

char ** tempStringDirectory(int n) {
    char ** array = malloc(n * sizeof(char *));
    int i;
    for(i = 0; i < n; i++) {
        array[i] = malloc(100*sizeof(char));
        memset(array[i], 0, sizeof(array[i]));
    }
    return array;
}

void freeStringDirectory(char ** array, int n) {
    int i;
    for(i = 0; i < n; i++) {
        free(array[i]);
    }
    free(array);
}

int fileSearch(char ** fileDir, int numFile, char* fileName) {
    int found = -1;
    int i;

    for (i = 0; i < numFile; i++) {
        if (strcmp(fileDir[i], fileName) == 0) {
            found = 1;
            break;
        }
    }
    return found;
}

// https://goo.gl/Q99WQM
void sendingFile(char * address, char * port, char * filename){
    struct addrinfo *res = setAddressInfo(argv[1], argv[2]);

    // Get the address information for the server.
    struct addrinfo *res = setAddressInfo(address, port);

    int sockfd = makeSocket(res);

    connectSocket(sockfd, res);

    char * buffer[1000];
    memset(buffer, 0, sizeof(buffer));

    int fileDirectory = open(filename, O_RDONLY);

    while(1) {
        // Read data into buffer.  We may not have enough to fill up buffer, so we
        // store how many bytes were actually read in bytes_read.
        int bytes_read = read(input_file, buffer, sizeof(buffer));
        if (bytes_read == 0) // We're done reading from the file
            break;

        if (bytes_read < 0) {
            // handle errors
            fprintf(stderr, "Cannot locate file, try again.");
            return;
        }

        // You need a loop for the write, because not all of the data may be written
        // in one call; write will return how many bytes were written. p keeps
        // track of where in the buffer we are, while we decrement bytes_read
        // to keep track of how many bytes are left to write.
        void *p = buffer;
        while (bytes_read > 0) {
            int bytes_written = write(output_socket, p, bytes_read);
            if (bytes_written <= 0) {
                // handle errors
                fprintf(stderr, "Error reading information into buffer.");
                return;
            }
            bytes_read -= bytes_written;
            p += bytes_written;
        }
        memset(buffer, 0, sizeof(buffer));
    }

    memset(buffer, 0, sizeof(buffer));
    strcpy(buffer, "__done__");
    send(dataSocket, buffer, sizeof(buffer),0);

    close(dataSocket);
    freeaddrinfo(res);
}

// REDO THIS SECTION LATER
void sendFullDirectory(char * address, char * port, char ** files, int numFiles){
    //struct addrinfo *res = setAddressInfo(argv[1], argv[2]);

    // Get the address information for the server.
    struct addrinfo *res = setAddressInfo(address, port);

    int sockfd = makeSocket(res);

    connectSocket(sockfd, res);

    int i ;
    for (i = 0; i < numFiles; i++){
        send(dataSocket, files[i], 100 ,0);                 //Send for the total number of files
    }

    //char* completed = "done";
    //send(dataSocket, completed, strlen(completed), 0);

    close(dataSocket);
    freeaddrinfo(res);
}


void buildConnection(int new_fd){
    char* pass = "pass";
    char* fail = "fail";
    char port[100];
    char command[500];
    char ipAddress[100];

    // Get confirmation that we are able to successfully connect to the client
    memset(port, 0 sizeof(port));
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

    // Now to check all the information that we just received so we can send back all the correct information.
    if (strcmp(command, "g") == 0) {
        send(new_fd, good, strlen(good), 0);

        // Get the name of the file that we are looking for
        char filename[100];
        memset(filename, 0, sizeof(filename));
        recv(new_fd, filename, sizeof(filename)-1, 0);

        // Get all the current files in our directory
        char** fileSet = tempStringDirectory(500);
        int numFile = getDirectory(files);
        int locateFile = fileSearch(files, numFile, filename);

        if (locateFile) {
            char* found = "Found";
            send(new_fd, found, strlen(found), 0);

            char req_filename[100];
            memset(req_filename, 0, sizeof(req_filename));
            strcpy(req_filename, "./");
            char * end = req_filename + strlen(req_filename);
            end += sprintf(end, "%s", req_filename);
        }

        else {
            printf("File Not Found");
            char * notFound = "File not found";
            send(new_fd, notFound, 100, 0);
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
        print("Did not understand request, make sure you use a g or l for this program");
    }

}

void sendDirectory(char* address, char * port, char ** files, int numFiles) {
    struct addrinfo * res = setAddressInfo(address, port);
    int sockfd = makeSocket(res);
    connectSocket(sockfd, res);

    int i;
    for (i = 0; i < numFiles; i++) {
        send(sockfd, files[i], 100, 0);
    }

    char* completed = "done";
    send(sockfd, completed, strlen(completed), 0);
}

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

int main(int argc, char *argv[]) {
    // Validate that we have the correct number of arguments before proceeding with the rest of the setup
    if (argc != 2) {
        fprintf(stderr, "Not the correct amount of arguments supplied.  Re-run the application");
    }

    // Get the address information for the server.
    struct addrinfo *res = setAddressInfo(argv[1], argv[2]);

    int sockfd = makeSocket(res);

    //connectSocket(sockfd, res);
    bindSocket(sockfd, res);
    listenSocket(sockfd);

    clientConnect(sockfd);

    //chatWithServer(sockfd, username, servername);

    freeaddrinfo(res);


}