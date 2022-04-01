#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "keys.h"
//#include <mqueue.h>
#include <sys/socket.h>

#define MAXSIZE 256


int init(){
    /*-------Sockets-------*/

    int sd;
    struct sockaddr_in server_addr;
    struct hostent *hp;

    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd == 1) {
        printf("Error in socket\n");
        return -1;
    }
    bzero((char *)&server_addr, sizeof(server_addr));
    hp = gethostbyname (argv[1]);
    if (hp == NULL) {
        printf("Error in gethostbyname\n");
        return -1;
    }

    memcpy (&(server_addr.sin_addr), hp->h_addr, hp->h_length);
    server_addr.sin_family  = AF_INET;
    server_addr.sin_port    = htons(4200);

    err = connect(sd, (struct sockaddr *) &server_addr,  sizeof(server_addr));
    if (err == -1) {
        printf("Error in connect\n");
        return -1;
    }
    a = htonl(5);
    b = htonl(2);
    op = 0; // add

    err = sendMessage(sd, (char *) &op, sizeof(char));  // envía la operacion
    if (err == -1){
        printf("Error seding\n");
        return -1;
    }
    err = sendMessage(sd, (char *) &a, sizeof(int32_t)); // envía a
    if (err == -1){
        printf("Error sending\n");
        return -1;
    }
    err = sendMessage(sd, (char *) &b, sizeof(int32_t)); // envíab
    if (err == -1){
        printf("Error sending\n");
        return -1;
    }

    err = recvMessage(sd, (char *) &res, sizeof(int32_t));     // recibe la respuesta
    if (err == -1){
        printf("Error receiving\n");
        return -1;
    }

    printf("Result is %d \n", ntohl(res));

    close (sd);


    /*-----------------Message Queue-----------------*/
     /* server message queue */
    mqd_t q_server;

    /* client message queue */
    mqd_t q_client;
   
    struct message_request msg;
    int res;
    struct mq_attr attr;
    attr.mq_maxmsg = 1;
    attr.mq_msgsize = sizeof(int);
    printf("test0");
    q_client = mq_open("/CLIENT_ONE", O_CREAT|O_RDONLY, 0700, &attr);
    if(q_client == -1){
        perror("Can't create client queue\n");
        return -1;
    }    
    
    q_server = mq_open("/ADD_SERVER", O_WRONLY);
    if(q_server == -1){
        perror("Can't create server queue\n");
        return -1;
    }    
    printf("test1");

    /* fill in request */
    strcpy(msg.q_name, "/CLIENT_ONE");
    msg.op = 0;

    if(mq_send(q_server, (char *)  &msg, sizeof(struct message_request), 0) == -1){
        perror("Error sending message queue\n");
        return -1;
    }
    if (mq_receive(q_client, (char *) &res, sizeof(int), 0) == -1){
        perror("Error receiving message queue keys.c\n");
        return -1;
    }
    if(mq_close(q_server) == -1){
        perror("Error closing q_server\n");
        return -1;
    }
    if(mq_close(q_client) == -1){
        perror("Error closing q_client\n");
        return -1;
    }
    if(mq_unlink("/CLIENT_ONE") == -1){
        perror("Error unlinking queue\n");
        return -1;
    }
    return 0;
}



int set_value(int key, char *value1, int value2, float value3){
   /* don't forget error handeling" */
    printf("set value keys\n");
    if(strlen(value1) == 0 )
        return -1;

    struct message_request msg;
 
    mqd_t q_server;
    mqd_t q_client;
    /* server message queue */
    /* client message queue */
    int res;
    struct mq_attr attr;
    attr.mq_maxmsg = 1;
    attr.mq_msgsize = sizeof(int);
    q_client = mq_open("/CLIENT_ONE", O_CREAT|O_RDONLY, 0700, &attr);
    if( q_client ==-1){
        perror("mq_open");
        return -1;
    }
    q_server = mq_open("/ADD_SERVER", O_WRONLY);
    if(q_server == -1){
        perror("mq_open");
        return-1;
    }
    /* fill in request */
    strcpy(msg.q_name, "/CLIENT_ONE");
    msg.op = 1;
    msg.key = key;
    strcpy(msg.value1, value1);
    msg.value2 = value2;
    msg.value3 = value3;
    if(mq_send(q_server, (char *)  &msg, sizeof(struct message_request), 0) == -1){
        perror("Error sending message queue\n");
        return -1;
    }
    if (mq_receive(q_client, (char *) &res, sizeof(int), 0) == -1){
        perror("Error receiving message queue keys.c\n");
        return -1;
    }
    if(mq_close(q_server) == -1){
        perror("Error closing q_server\n");
        return -1;
    }
    if(mq_close(q_client) == -1){
        perror("Error closing q_client\n");
        return -1;
    }
    if(mq_unlink("/CLIENT_ONE") == -1){
        perror("Error unlinking queue\n");
        return -1;
    }
    return res;
}


int get_value(int key, char *value1, int *value2, float *value3){
          /* don't forget error handeling" */
    struct message_request msg;
    struct message_response msg_res;
    printf("here1");
    mqd_t q_server;
    mqd_t q_client;
    /* server message queue */
    /* client message queue */
    struct mq_attr attr;
    attr.mq_maxmsg = 1;
    attr.mq_msgsize = sizeof(struct message_response);
    q_client = mq_open("/CLIENT_ONE", O_CREAT|O_RDONLY, 0700, &attr);
    if(q_client == -1){
        perror("Can't create client queue\n");
        return -1;
    }    
    
    q_server = mq_open("/ADD_SERVER", O_WRONLY);
    if(q_server == -1){
        perror("Can't create server queue\n");
        return -1;
    } 
    /* fill in request */
    strcpy(msg.q_name, "/CLIENT_ONE");
    msg.op = 2;
    msg.key = key;

    if(mq_send(q_server, (char *)  &msg, sizeof(struct message_request), 0) == -1){
        perror("Error sending message queue\n");
        return -1;
    }
    if (mq_receive(q_client, (char *) &msg_res, sizeof(struct message_response), 0) == -1){
        perror("Error receiving message queue keys.c\n");
        return -1;
    }
    if(mq_close(q_server) == -1){
        perror("Error closing q_server\n");
        return -1;
    }
    if(mq_close(q_client) == -1){
        perror("Error closing q_client\n");
        return -1;
    }
    if(mq_unlink("/CLIENT_ONE") == -1){
        perror("Error unlinking queue\n");
        return -1;
    }
  

    if (msg_res.err != -1) {
        *value2 = msg_res.value2;
        *value3 = msg_res.value3;
        strcpy(value1, msg_res.value1);
    }

    return  msg_res.err;
}

int delete_key(int key){
      /* don't forget error handeling" */
    struct message_request msg;
    int res;
 
    mqd_t q_server;
    mqd_t q_client;
    /* server message queue */
    /* client message queue */
    struct mq_attr attr;
    attr.mq_maxmsg = 1;
    attr.mq_msgsize = sizeof(int);
    q_client = mq_open("/CLIENT_ONE", O_CREAT|O_RDONLY, 0700, &attr);
    if(q_client == -1){
        perror("Can't create client queue\n");
        return -1;
    }    
    
    q_server = mq_open("/ADD_SERVER", O_WRONLY);
    if(q_server == -1){
        perror("Can't create server queue\n");
        return -1;
    } 
    /* fill in request */
    strcpy(msg.q_name, "/CLIENT_ONE");
    msg.op = 4;
    msg.key = key;
    if(mq_send(q_server, (char *)  &msg, sizeof(struct message_request), 0) == -1){
        perror("Error sending message queue\n");
        return -1;
    }
    if (mq_receive(q_client, (char *) &res, sizeof(int), 0) == -1){
        perror("Error receiving message queue keys.c\n");
        return -1;
    }
    if(mq_close(q_server) == -1){
        perror("Error closing q_server\n");
        return -1;
    }
    if(mq_close(q_client) == -1){
        perror("Error closing q_client\n");
        return -1;
    }
    if(mq_unlink("/CLIENT_ONE") == -1){
        perror("Error unlinking queue\n");
        return -1;
    }
    return res;
}

int modify_value(int key, char *value1, int value2, float value3){
   /* don't forget error handeling" */

    if(strlen(value1) == 0 )
        return -1;

    struct message_request msg;
 
    mqd_t q_server;
    mqd_t q_client;
    /* server message queue */
    /* client message queue */
    int res;
    struct mq_attr attr;
    attr.mq_maxmsg = 1;
    attr.mq_msgsize = sizeof(int);
    q_client = mq_open("/CLIENT_ONE", O_CREAT|O_RDONLY, 0700, &attr);
    if(q_client == -1){
        perror("Can't create client queue\n");
        return -1;
    }    
    
    q_server = mq_open("/ADD_SERVER", O_WRONLY);
    if(q_server == -1){
        perror("Can't create server queue\n");
        return -1;
    } 
    /* fill in request */
    strcpy(msg.q_name, "/CLIENT_ONE");
    msg.op = 3;
    msg.key = key;
    strcpy(msg.value1, value1);
    msg.value2 = value2;
    msg.value3 = value3;
    if(mq_send(q_server, (char *)  &msg, sizeof(struct message_request), 0) == -1){
        perror("Error sending message queue\n");
        return -1;
    }
    if (mq_receive(q_client, (char *) &res, sizeof(int), 0) == -1){
        perror("Error receiving message queue keys.c\n");
        return -1;
    }
    if(mq_close(q_server) == -1){
        perror("Error closing q_server\n");
        return -1;
    }
    if(mq_close(q_client) == -1){
        perror("Error closing q_client\n");
        return -1;
    }
    if(mq_unlink("/CLIENT_ONE") == -1){
        perror("Error unlinking queue\n");
        return -1;
    }
    return res;
}

int exist(int key){
    printf("exist\n");

    struct message_request msg;
 
    mqd_t q_server;
    mqd_t q_client;
    /* server message queue */
    /* client message queue */
    int res;
    struct mq_attr attr;
    attr.mq_maxmsg = 1;
    attr.mq_msgsize = sizeof(int);
    q_client = mq_open("/CLIENT_ONE", O_CREAT|O_RDONLY, 0700, &attr);
    if(q_client == -1){
        perror("Can't create client queue\n");
        return -1;
    }    
    
    q_server = mq_open("/ADD_SERVER", O_WRONLY);
    if(q_server == -1){
        perror("Can't create server queue\n");
        return -1;
    } 
    /* fill in request */
    strcpy(msg.q_name, "/CLIENT_ONE");
    msg.op = 5;
    msg.key = key;
    if(mq_send(q_server, (char *)  &msg, sizeof(struct message_request), 0) == -1){
        perror("Error sending message queue\n");
        return -1;
    }
    if (mq_receive(q_client, (char *) &res, sizeof(int), 0) == -1){
        perror("Error receiving message queue keys.c\n");
        return -1;
    }
    if(mq_close(q_server) == -1){
        perror("Error closing q_server\n");
        return -1;
    }
    if(mq_close(q_client) == -1){
        perror("Error closing q_client\n");
        return -1;
    }
    if(mq_unlink("/CLIENT_ONE") == -1){
        perror("Error unlinking queue\n");
        return -1;
    }
    return res;
}

int num_items(){
    /* don't forget error handeling" */
    struct message_request msg;
    long int res;
 
    mqd_t q_server;
    mqd_t q_client;
    /* server message queue */
    /* client message queue */
    struct mq_attr attr;
    attr.mq_maxmsg = 1;
    attr.mq_msgsize = sizeof(int);
    q_client = mq_open("/CLIENT_ONE", O_CREAT|O_RDONLY, 0700, &attr);
    if(q_client == -1){
        perror("Can't create client queue\n");
        return -1;
    }    
    
    q_server = mq_open("/ADD_SERVER", O_WRONLY);
    if(q_server == -1){
        perror("Can't create server queue\n");
        return -1;
    } 
    /* fill in request */
    strcpy(msg.q_name, "/CLIENT_ONE");
    msg.op = 6;
    if(mq_send(q_server, (char *)  &msg, sizeof(struct message_request), 0) == -1){
        perror("Error sending message queue\n");
        return -1;
    }
    
    if (mq_receive(q_client, (char *) &res, sizeof(res), 0) == -1){
        perror("Error receiving message queue keys.c\n");
        return -1;
    }
    if(mq_close(q_server) == -1){
        perror("Error closing q_server\n");
        return -1;
    }
    if(mq_close(q_client) == -1){
        perror("Error closing q_client\n");
        return -1;
    }
    if(mq_unlink("/CLIENT_ONE") == -1){
        perror("Error unlinking queue\n");
        return -1;
    }
    return res;
}