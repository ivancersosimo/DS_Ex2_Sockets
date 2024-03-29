#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "keys.h"
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>

#define MAXSIZE 256


char * getIP(){
    char *var;
    var = getenv("IP_TUPLES");
    if (var == NULL){
        perror("Variable PD_TUPLES not defined\n");
        return 0;
    }
    else
        return var;
}

char * getPort(){
    char *var;
    var = getenv("PORT_TUPLES");
    if (var == NULL){
        perror("Variable PORT_TUPLES not defined\n");
        return 0;
    }
    else
        return var;
}

int sendMessage(int socket, char * buffer, int len){
    int r;
    int l = len;

    do {
        r = write(socket, buffer, l);
        l = l -r;
        buffer = buffer + r;
    } while ((l>0) && (r>=0));

    if (r < 0)
        return (-1);   /* fail */
    else
        return(0);	/* full length has been sent */
}

int recvMessage(int socket, char *buffer, int len){
    int r;
    int l = len;

    do {
        r = read(socket, buffer, l);
        l = l -r ;
        buffer = buffer + r;
    } while ((l>0) && (r>=0));

    if (r < 0)
        return (-1);   /* fallo */
    else
        return(0);	/* full length has been receive */
}
ssize_t readLine(int fd, void *buffer, size_t n){
    ssize_t numRead;  /* num of bytes fetched by last read() */
    size_t totRead;	  /* total bytes read so far */
    char *buf;
    char ch;

    if (n <= 0 || buffer == NULL) {
        errno = EINVAL;
        return -1;
    }

    buf = buffer;
    totRead = 0;

    for (;;) {
        numRead = read(fd, &ch, 1);	/* read a byte */

        if (numRead == -1) {
            if (errno == EINTR)	/* interrupted -> restart read() */
                continue;
            else
                return -1;		/* some other error */
        } else if (numRead == 0) {	/* EOF */
            if (totRead == 0)	/* no byres read; return 0 */
                return 0;
            else
                break;
        } else {			/* numRead must be 1 if we get here*/
            if (ch == '\n')
                break;
            if (ch == '\0')
                break;
            if (totRead < n - 1) {		/* discard > (n-1) bytes */
                totRead++;
                *buf++ = ch;
            }
        }
    }

    *buf = '\0';
    return totRead;
}


int init(){
    /*-------Sockets-------*/

    int sd, err; 
    struct sockaddr_in server_addr;
    struct hostent *hp;
    int op, res;
    sd = socket(AF_INET, SOCK_STREAM, 0);
    
    if (sd == 1) {
        printf("Error in socket\n");
        return -1;
    }
    bzero( (char *) &server_addr, sizeof(server_addr));
    hp = gethostbyname (getIP());
    if (hp == NULL) {
        printf("Error in gethostbyname\n");
        close(sd);
        return -1;
    }
    uint16_t myport = atoi(getPort());
    printf("my port is: %d \n",myport);

    memcpy(&(server_addr.sin_addr), hp->h_addr, hp->h_length);
    server_addr.sin_family  = AF_INET;
    server_addr.sin_port    = htons(myport);

    err = connect(sd, (struct sockaddr *) &server_addr,  sizeof(server_addr));
    if (err == -1) {
        printf("Error in connect\n");
        close(sd);
        return -1;
    }

    op = 0;
    err = sendMessage(sd, (char *) &op, sizeof(int));  // envía la operacion
    if (err == -1){
        printf("Error sending\n");
        close(sd);
        return -1;
    }

    err = recvMessage(sd, (char *) &res, sizeof(int));     // recibe la respuesta
    if (err == -1){
        printf("Error receiving\n");
        close(sd);
        return -1;
    }

  /*  if(close (sd) == -1){
        perror("Error closing socket\n");
        return -1;
    }*/
    return res;
}



int set_value(int32_t key, char *value1, int32_t value2, float value3){
    /*--------Sockets------------*/
    int sd, err; 
    struct sockaddr_in server_addr;
    struct hostent *hp;
    int op, res;
    char *buff;
    sd = socket(AF_INET, SOCK_STREAM, 0);
    char myvalue3[MAXSIZE];

    if (sd == 1) {
        printf("Error in socket\n");
        return -1;
    }
    bzero( (char *) &server_addr, sizeof(server_addr));
    hp = gethostbyname (getIP());
    if (hp == NULL) {
        printf("Error in gethostbyname\n");
        close(sd);
        return -1;
    }
    
    uint16_t myport = atoi(getPort());
    memcpy (&(server_addr.sin_addr), hp->h_addr, hp->h_length);
    server_addr.sin_family  = AF_INET;
    server_addr.sin_port    = htons(myport);

    err = connect(sd, (struct sockaddr *) &server_addr,  sizeof(server_addr));
    if (err == -1) {
        printf("Error in connect\n");
        close(sd);
        return -1;
    }

    op = 1;
    err = sendMessage(sd, (char *) &op, sizeof(int));  // envía la operacion
    if (err == -1){
        printf("Error sending\n");
        close(sd);
        return -1;
    }
    printf("set value key: %d\n", key);
    key = htonl(key);
    err = sendMessage(sd, (char *) &key, sizeof(int32_t));  // envía la operacion
    if (err == -1){
        printf("Error sending\n");
        close(sd);
        return -1;
    }
    err = sendMessage(sd, (char *) value1, strlen(value1)+1);  // envía la operacion
    if (err == -1){
        printf("Error sending value1\n");
        close(sd);
        return -1;
    }
    value2 = htonl(value2);
    err = sendMessage(sd, (char *) &value2, sizeof(int32_t));  // envía la operacion
    if (err == -1){
        printf("Error sending value2\n");
        close(sd);
        return -1;
    }
    printf("value3 f: %f\n", value3);
    sprintf(myvalue3, "%f\n", value3);
    
    printf("myvalue3: %s\n", myvalue3);
    err = sendMessage(sd, (char *) myvalue3, strlen(myvalue3)+1);  // envía la operacion
    if (err == -1){
        printf("Error sending value3\n");
        close(sd);
        return -1;
    }

    err = recvMessage(sd, (char *) &res, sizeof(int));     // recibe la respuesta
    if (err == -1){
        printf("Error receiving\n");
        close(sd);
        return -1;
    }

    if(close (sd) == -1){
        perror("Error closing socket\n");
        return -1;
    }
    return res;
}


int get_value(int32_t key, char *value1, int32_t *value2, float *value3){
    /*--------Sockets------------*/
    int sd, err; 
    struct sockaddr_in server_addr;
    struct hostent *hp;
    int op, res;
    char *buff;
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd == 1) {
        printf("Error in socket\n");
        return -1;
    }
    bzero( (char *) &server_addr, sizeof(server_addr));
    hp = gethostbyname (getIP());
    if (hp == NULL) {
        printf("Error in gethostbyname\n");
        close(sd);
        return -1;
    }
    uint16_t myport = atoi(getPort());
    memcpy (&(server_addr.sin_addr), hp->h_addr, hp->h_length);
    server_addr.sin_family  = AF_INET;
    server_addr.sin_port    = htons(myport);

    err = connect(sd, (struct sockaddr *) &server_addr,  sizeof(server_addr));
    if (err == -1) {
        printf("Error in connect\n");
        close(sd);
        return -1;
    }

    op = 2;
    err = sendMessage(sd, (char *) &op, sizeof(int));  // envía la operacion
    if (err == -1){
        printf("Error sending\n");
        close(sd);
        return -1;
    }
    key = htonl(key);
    err = sendMessage(sd, (char *) &key, sizeof(int32_t));  // envía la operacion
    if (err == -1){
        printf("Error sending\n");
        close(sd);
        return -1;
    }

    err = recvMessage(sd, (char *) &res, sizeof(int));   // recibe la respuesta
    if (err == -1){
        printf("Error receiving\n");
        close(sd);
        return -1;
    }
    err = readLine(sd, (char *) value1, MAXSIZE);  // envía la operacion
    if (err == -1){
        printf("Error reding line\n");
        close(sd);
        return -1;
    }
    int32_t myvalue2;
    err = recvMessage(sd, (char *) &myvalue2, sizeof(int32_t));  // envía la operacion
    myvalue2 = ntohl(myvalue2);
    *value2 = myvalue2;
    if (err == -1){
        printf("Error receiving\n");
        close(sd);
        return -1;
    }
    char myvalue3[50];
    err = readLine(sd, (char *) myvalue3, 50);  // envía la operacion
    *value3 = atof(myvalue3);
    if (err == -1){
        printf("Error receiving\n");
        close(sd);
        return -1;
    }

    if(close (sd) == -1){
        perror("Error closing socket\n");
        return -1;
    }
    return res;
}

int delete_key(int32_t key){
    /*--------Sockets------------*/
    int sd, err; 
    struct sockaddr_in server_addr;
    struct hostent *hp;
    int op, res;
    char *buff;
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd == 1) {
        printf("Error in socket\n");
        return -1;
    }
    bzero( (char *) &server_addr, sizeof(server_addr));
    hp = gethostbyname (getIP());
    if (hp == NULL) {
        printf("Error in gethostbyname\n");
        close(sd);
        return -1;
    }
    uint16_t myport = atoi(getPort());
    memcpy (&(server_addr.sin_addr), hp->h_addr, hp->h_length);
    server_addr.sin_family  = AF_INET;
    server_addr.sin_port    = htons(myport);

    err = connect(sd, (struct sockaddr *) &server_addr,  sizeof(server_addr));
    if (err == -1) {
        printf("Error in connect\n");
        close(sd);
        return -1;
    }

    op = 4;
    err = sendMessage(sd, (char *) &op, sizeof(int));  // envía la operacion
    if (err == -1){
        printf("Error sending\n");
        close(sd);
        return -1;
    }
    key = htonl(key);
    err = sendMessage(sd, (char *) &key, sizeof(int32_t));  // envía la operacion
    if (err == -1){
        printf("Error sending\n");
        close(sd);
        return -1;
    }

    err = recvMessage(sd, (char *) &res, sizeof(int));     // recibe la respuesta
    if (err == -1){
        printf("Error receiving\n");
        close(sd);
        return -1;
    }

    if(close (sd) == -1){
        perror("Error closing socket\n");
        return -1;
    }
    return res;
}

int modify_value(int32_t key, char *value1, int32_t value2, float value3){
   /*--------Sockets------------*/
    int sd, err; 
    struct sockaddr_in server_addr;
    struct hostent *hp;
    int op, res;
    char *buff;
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd == 1) {
        printf("Error in socket\n");
        return -1;
    }
    bzero( (char *) &server_addr, sizeof(server_addr));
    hp = gethostbyname (getIP());
    if (hp == NULL) {
        printf("Error in gethostbyname\n");
        close(sd);
        return -1;
    }

    uint16_t myport = atoi(getPort());
    memcpy (&(server_addr.sin_addr), hp->h_addr, hp->h_length);
    server_addr.sin_family  = AF_INET;
    server_addr.sin_port    = htons(myport);

    err = connect(sd, (struct sockaddr *) &server_addr,  sizeof(server_addr));
    if (err == -1) {
        printf("Error in connect\n");
        close(sd);
        return -1;
    }

    op = 3;
    err = sendMessage(sd, (char *) &op, sizeof(int));  // envía la operacion
    if (err == -1){
        printf("Error sending\n");
        close(sd);
        return -1;
    }
    key = htonl(key);
    err = sendMessage(sd, (char *) &key, sizeof(int32_t));  // envía la operacion
    if (err == -1){
        printf("Error sending\n");
        close(sd);
        return -1;
    }
    err = sendMessage(sd, (char *) value1, strlen(value1)+1);  // envía la operacion
    if (err == -1){
        printf("Error sending\n");
        close(sd);
        return -1;
    }
    printf("keys value2 before htonl: %d\n", value2);
    value2 = htonl(value2);
    printf("keys value2 after htonl: %d\n", value2);
    err = sendMessage(sd, (char *) &value2, sizeof(int32_t));  // envía la operacion
    if (err == -1){
        printf("Error sending\n");
        close(sd);
        return -1;
    }
    char myvalue3[MAXSIZE];
    sprintf(myvalue3, "%f", value3);
    printf("value3 as a string %s:\n",myvalue3);
    err = sendMessage(sd, (char *) myvalue3, strlen(myvalue3)+1);  // envía la operacion
    if (err == -1){
        printf("Error sending\n");
        close(sd);
        return -1;
    }

    err = recvMessage(sd, (char *) &res, sizeof(int));     // recibe la respuesta
    if (err == -1){
        printf("Error receiving\n");
        close(sd);
        return -1;
    }

    if(close (sd) == -1){
        perror("Error closing socket\n");
        close(sd);
        return -1;
    }
    return res;
}

int exist(int32_t key){
    /*--------Sockets------------*/
    int sd, err; 
    struct sockaddr_in server_addr;
    struct hostent *hp;
    int op, res;
    char *buff;
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd == 1) {
        printf("Error in socket\n");
        return -1;
    }
    bzero( (char *) &server_addr, sizeof(server_addr));
    hp = gethostbyname (getIP());
    if (hp == NULL) {
        printf("Error in gethostbyname\n");
        close(sd);
        return -1;
    }

    uint16_t myport = atoi(getPort());
    memcpy (&(server_addr.sin_addr), hp->h_addr, hp->h_length);
    server_addr.sin_family  = AF_INET;
    server_addr.sin_port    = htons(myport);

    err = connect(sd, (struct sockaddr *) &server_addr,  sizeof(server_addr));
    if (err == -1) {
        printf("Error in connect\n");
        close(sd);
        return -1;
    }

    op = 5;
    err = sendMessage(sd, (char *) &op, sizeof(int));  // envía la operacion
    if (err == -1){
        printf("Error sending\n");
        close(sd);
        return -1;
    }
    key = htonl(key);
    err = sendMessage(sd, (char *) &key, sizeof(int32_t));  // envía la operacion
    if (err == -1){
        printf("Error sending\n");
        close(sd);
        return -1;
    }

    err = recvMessage(sd, (char *) &res, sizeof(int));     // recibe la respuesta
    if (err == -1){
        printf("Error receiving\n");
        close(sd);
        return -1;
    }

    if(close (sd) == -1){
        perror("Error closing socket\n");
        return -1;
    }
    return res;
}

int num_items(){
    /*--------Sockets------------*/
    int sd, err; 
    struct sockaddr_in server_addr;
    struct hostent *hp;
    int op, res;
    char *buff;
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd == 1) {
        printf("Error in socket\n");
        return -1;
    }
    bzero( (char *) &server_addr, sizeof(server_addr));
    hp = gethostbyname (getIP());
    if (hp == NULL) {
        printf("Error in gethostbyname\n");
        close(sd);
        return -1;
    }
    uint16_t myport = atoi(getPort());
    memcpy (&(server_addr.sin_addr), hp->h_addr, hp->h_length);
    server_addr.sin_family  = AF_INET;
    server_addr.sin_port    = htons(myport);

    err = connect(sd, (struct sockaddr *) &server_addr,  sizeof(server_addr));
    if (err == -1) {
        printf("Error in connect\n");
        close(sd);
        return -1;
    }

    op = 6;
    err = sendMessage(sd, (char *) &op, sizeof(int));  // envía la operacion
    if (err == -1){
        printf("Error sending\n");
        close(sd);
        return -1;
    }

    err = recvMessage(sd, (char *) &res, sizeof(int));     // recibe la respuesta
    if (err == -1){
        printf("Error receiving\n");
        close(sd);
        return -1;
    }

    if(close (sd) == -1){
        perror("Error closing socket\n");
        return -1;
    }
    return res;
}

int exit_key(){
    /*--------Sockets------------*/
    int sd, err; 
    struct sockaddr_in server_addr;
    struct hostent *hp;
    int op, res;
    char *buff;
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd == 1) {
        printf("Error in socket\n");
        return -1;
    }
    bzero( (char *) &server_addr, sizeof(server_addr));
    hp = gethostbyname (getIP());
    if (hp == NULL) {
        printf("Error in gethostbyname\n");
        close(sd);
        return -1;
    }
    uint16_t myport = atoi(getPort());
    memcpy (&(server_addr.sin_addr), hp->h_addr, hp->h_length);
    server_addr.sin_family  = AF_INET;
    server_addr.sin_port    = htons(myport);

    err = connect(sd, (struct sockaddr *) &server_addr,  sizeof(server_addr));
    if (err == -1) {
        printf("Error in connect\n");
        close(sd);
        return -1;
    }

    op = 7;
    err = sendMessage(sd, (char *) &op, sizeof(int));  // envía la operacion
    if (err == -1){
        printf("Error sending\n");
        close(sd);
        return -1;
    }

    if(close (sd) == -1){
        perror("Error closing socket\n");
        return -1;
    }
    return 0;
}