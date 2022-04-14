//#include <mqueue.h>
#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/socket.h>
#include "keys.h"


pthread_mutex_t mutex_msg, mymutex;
bool message_not_copied = true;
pthread_cond_t cond_msg;

char server_name[] = "/ADD_SERVER";
char client_name[] = "/CLIENT_ONE";
char msg_dir_name[] = "messages_dir";
mqd_t q_server, q_client;

int process_message(struct message_request *msg){
    struct message_request msg_local;
    
    if (pthread_mutex_lock(&mutex_msg) != 0){
        perror("Error mutex_lock\n");
        return -1;
    }
    msg_local = *msg;
    message_not_copied = false;
    if (pthread_cond_signal(&cond_msg) != 0){
        perror("Error cond_signal\n");
        return -1;
    }
    if (pthread_mutex_unlock(&mutex_msg) != 0){
        perror("Error mutex_unlock\n");
        return -1;
    }

    struct message_response msgres_local;
    DIR *msgdir;
    struct dirent * msgcont;
    char file_name[100], file_to_delete_path[50];
    char KeyString[20], v2str[20], v3str[20];
    char filecontent[sizeof(struct message_request)];
    int myres, mymsg, in_dir, file_cont, num_items;
    FILE *msg_mod;
    char buff[sizeof(struct message_request)];
    char to_read_value1[50], to_read_value2[50], to_read_value3[50];

    struct mq_attr c_attr;
    c_attr.mq_maxmsg = 1;
    c_attr.mq_msgsize = sizeof(int);

    struct mq_attr get_attr;
    get_attr.mq_maxmsg = 1;
    get_attr.mq_msgsize = sizeof(struct message_response);

    switch(msg_local.op){
        case 0:
            /*init()*/
            q_client = mq_open(client_name, O_CREAT|O_RDWR, 0700, &c_attr);
            if(q_client == -1){
                perror("Can't create client queue");
                return -1;
            }
            if ((msgdir = opendir(msg_dir_name)) == NULL){
                perror("Error opening directory\n");
                return -1;
            }
            errno = 0;
            while ((msgcont = readdir(msgdir)) != NULL){
                /*strcpy(file_to_delete_path, "");
                strcat(file_to_delete_path, msg_dir_name);
                strcat(file_to_delete_path, "/");
                strcat(file_to_delete_path, msgcont->d_name);*/
                sprintf(file_to_delete, "%s/%s", msg_dir_name, msgcont->d_name);
                            
                if (msgcont->d_type == DT_REG){
                    printf("%s\n",file_to_delete_path);
                    if (pthread_mutex_lock(&mymutex) != 0){
                        perror("Error mutex_lock\n");
                        return -1;
                    }
                    if (remove(file_to_delete_path) == -1){
                        perror("Error removign file\n");
                        return -1;
                    }
                    if (pthread_mutex_unlock(&mymutex) != 0){
                        perror("Error mutex_unlock\n");
                        return -1;
                    }
                    printf("File is removed\n");
                }
            }
            if(msgcont == NULL && errno != 0){
                perror("Error reading directory\n");
                return -1;
            }
            myres = 0;
            if(mq_send(q_client, (char *) &myres, sizeof(int), 0)==-1){
                perror("Error sending message queue\n");
                return -1;
            }
            if(mq_close(q_client) == -1){
                perror("Error closing client queue\n");
                return -1;
            }

            if(closedir(msgdir) == -1){
                perror("Error closing directory\n");
                return -1;
            }
            break;
        case 1:
            /*set_value() */
            q_client = mq_open(client_name, O_CREAT|O_RDWR, 0700, &c_attr);
            if(q_client == -1){
                perror("Can't create client queue");
                return -1;
            }
            if ((msgdir = opendir(msg_dir_name)) == NULL){
                perror("Error opening directory\n");
                return -1;
            }
            in_dir = 1;
            errno = 0;
            while ((msgcont = readdir(msgdir)) != NULL && in_dir == 1){
                printf("Set value, mkey: %d\n", msg_local.key);
                /*if (atoi(msgcont->d_name) == 0){
                    perror("Could not perform conversion\n");
                    return -1;
                }*/
                if (pthread_mutex_lock(&mymutex) != 0){
                    perror("Error mutex_lock\n");
                    return -1;
                }
                if (atoi(msgcont->d_name) != msg_local.key){
                    in_dir = 1; //key doesn't exist
                    printf("key no exists, in_dir = %d\n", in_dir);
                }
                else {
                    in_dir = 0; //key exists
                    printf("key exists, in_dir = %d\n", in_dir);
                    myres = -1;
                    printf("myres: %d\n ",myres);
                }
                if (pthread_mutex_unlock(&mymutex) != 0){
                    perror("Error mutex_unlock\n");
                    return -1;
                }
            }
            if(msgcont == NULL && errno != 0){
                perror("Error reading directory\n");
                return -1;
            }
            if (in_dir == 1){
                strcpy(file_name, "");
                /*if (snprintf(KeyString, sizeof(KeyString), "%d", msg_local.key) < 0){
                    perror("Could not perform conversion\n");
                    return -1;
                }*/
                /*snprintf(KeyString, sizeof(KeyString), "%d", msg_local.key);
                strcat(file_name, msg_dir_name);
                strcat(file_name, "/");
                strcat(file_name, KeyString);*/
                sprintf(file_name, "%s/%d", msg_dir_name, msg_local.key)
                printf("filename: %s\n",file_name);

                /*strcpy(filecontent, "");
                if (snprintf(v2str, sizeof(v2str), "%d", msg_local.value2) < 0){
                    perror("Could not perform conversion\n");
                    return -1;
                }
                if (snprintf(v3str, sizeof(v3str), "%f", msg_local.value3) < 0){
                    perror("Could not perform conversion\n");
                    return -1;
                }
                strcat(filecontent, msg_local.value1);
                strcat(filecontent, ";");
                strcat(filecontent, v2str);
                strcat(filecontent, ";");
                strcat(filecontent, v3str);
                strcat(filecontent, ";");*/
                sprintf(filecontent, "%s;%d;%f;", msg_local.value1, msg_local.value2, msg_local.value3);
                printf("filecontent: %s\n",filecontent);
                if((mymsg = open(file_name, O_CREAT | O_RDWR, 0644)) == -1){
                    perror("Error opening file\n");
                    return -1;
                }
                if (pthread_mutex_lock(&mymutex) != 0){
                    perror("Error mutex_lock\n");
                    return -1;
                }
                if (write(mymsg, &filecontent, sizeof(filecontent)) == -1){
                    perror("Error writing in file\n");
                    return -1;
                }
                if (pthread_mutex_unlock(&mymutex) != 0){
                        perror("Error mutex_unlock\n");
                        return -1;
                    }
                if (close(mymsg) == -1){
                    perror("Error closing file\n");
                    return -1;
                }
                myres = 0;
            }
            printf("myres2: %d\n ",myres);
            if(mq_send(q_client, (char *) &myres, sizeof(int), 0) == -1){
                perror("mqsend");
                return -1;
            }
            if (mq_close(q_client) == -1){
                perror("mq_close");
                return -1;
            }
            
            if(closedir(msgdir) == -1){
                perror("closedir");
                return -1;
            }
            break;
        case 2:
            /*get_value()*/
            q_client = mq_open(client_name, O_CREAT|O_RDWR, 0700, &get_attr);
            if(q_client == -1){
                perror("Can't create client queue");
                return -1;
            }
            if ((msgdir = opendir(msg_dir_name)) == NULL){
                perror("Error opening directory\n");
                return -1;
            }
            printf("Get value, dir open\n");
            in_dir = 1;
            errno = 0;
            while (((msgcont = readdir(msgdir)) != NULL) && in_dir == 1){
                printf("Get value, filename: %s", msgcont->d_name);
                if (pthread_mutex_lock(&mymutex) != 0){
                    perror("Error mutex_lock\n");
                    return -1;
                }
                if(atoi(msgcont->d_name) == 0){
                    perror("Could not perform conversion\n");
                    return -1;
                }
                if (atoi(msgcont->d_name) != msg_local.key){
                    in_dir = 1; //key doesn't exist
                    printf("key no exists, in_dir = %d\n", in_dir);
                    msgres_local.err = -1;
                }
                else {
                    in_dir = 0; //key exists
                    printf("key exists, in_dir = %d\n", in_dir);
                }
                if (pthread_mutex_unlock(&mymutex) != 0){
                    perror("Error mutex_unlock\n");
                    return -1;
                }
            }
            if(msgcont == NULL && errno != 0){
                perror("Error reading directory\n");
                return -1;
            }
            if (in_dir == 0){
                /*strcpy(file_name, "");
                if (snprintf(KeyString, sizeof(KeyString), "%d", msg_local.key) < 0){
                    perror("Could not perform conversion\n");
                    return -1;
                }
                strcat(file_name, msg_dir_name);
                strcat(file_name, "/");
                strcat(file_name, KeyString);*/
                sprintf(file_name, "%s/%d", msg_dir_name, msg_locsal.key);
                printf("filename: %s\n",file_name);

                if((mymsg = open(file_name, O_RDONLY, 0644)) == -1){
                    perror("Error opening file\n");
                    return -1;
                }
                printf("open works\n");
                if (pthread_mutex_lock(&mymutex) != 0){
                    perror("Error mutex_lock\n");
                    return -1;
                }
                if((file_cont = read(mymsg, &filecontent, sizeof(filecontent))) == -1){
                    perror("Error reading file\n");
                    return -1;
                }
                if (pthread_mutex_unlock(&mymutex) != 0){
                    perror("Error mutex_unlock\n");
                    return -1;
                }
                printf("file read\n");
                char *strtrunc = strtok(filecontent, ";");
                printf("strtrunc: %s\n", strtrunc);
                printf("srtok done\n");
                strcpy(msgres_local.value1, (char*)strtrunc);
                printf("Value1: %s\n", msgres_local.value1);
                strtrunc = strtok(NULL, ";");
                if (atoi(strtrunc) == 0){
                    perror("Could not perform conversion\n");
                    return -1;
                }
                msgres_local.value2 = atoi(strtrunc);
                printf("Value2: %d\n", msgres_local.value2);
                strtrunc = strtok(NULL, ";");
                if (atof(strtrunc) == 0.0){
                    perror("Could not perform conversion\n");
                    return -1;
                }
                msgres_local.value3 = (float)atof(strtrunc);
                printf("Value3: %f\n", msgres_local.value3);
                msgres_local.err = 0;
                if (close(mymsg) == -1){
                    perror("Error closing file\n");
                    return -1;
                }
            }

            if (mq_send(q_client, (char *)  &msgres_local, sizeof(struct message_response), 0) == -1){
                perror("Error sending message queue\n");
                return -1;
            }
            if (mq_close(q_client) == -1){
                perror("Error closing client queue\n");
            }
            if (closedir(msgdir) == -1){
                perror("Error closing directory\n");
                return -1;
            }
            break;
        case 3:
            /*modify_value()*/
            q_client = mq_open(client_name, O_CREAT|O_RDWR, 0700, &c_attr);
            if(q_client == -1){
                perror("Can't create client queue");
                return -1;
            }
            if ((msgdir = opendir(msg_dir_name)) == NULL){
                perror("Error opening directory\n");
                return -1;
            }
            printf("Modify value, dir open\n");
            in_dir = 1;
            errno = 0;
            while (((msgcont = readdir(msgdir)) != NULL) && in_dir == 1){
                printf("Modify value, filename: %s", msgcont->d_name);
                if (pthread_mutex_lock(&mymutex) != 0){
                    perror("Error mutex_lock\n");
                    return -1;
                }
                if (atoi(msgcont->d_name) == 0){
                    perror("Could not perform conversion\n");
                    return -1;
                }
                if (atoi(msgcont->d_name) != msg_local.key){
                    in_dir = 1; //key doesn't exist
                    printf("key no exists, in_dir = %d\n", in_dir);
                    myres = -1;
                }
                else {
                    in_dir = 0; //key exists
                    printf("key exists, in_dir = %d\n", in_dir);
                }
                if (pthread_mutex_unlock(&mymutex) != 0){
                    perror("Error mutex_unlock\n");
                    return -1;
                }
            }
            if(msgcont == NULL && errno != 0){
                perror("Error reading directory\n");
                return -1;
            }
            if (in_dir == 0){
                /*strcpy(file_name, "");
                if (snprintf(KeyString, sizeof(KeyString), "%d", msg_local.key) < 0){
                    perror("Could not perform conversion\n");
                    return -1;
                }
                strcat(file_name, msg_dir_name);
                strcat(file_name, "/");
                strcat(file_name, KeyString);*/
                sprintf(file_name, "%s/%d", msg_dir_name, msg_locsal.key);
                printf("modify filename: %s\n",file_name);

                /*strcpy(filecontent, "");
                if (snprintf(v2str, sizeof(v2str), "%d", msg_local.value2) < 0){
                    perror("Could not perform conversion\n");
                    return -1;
                }
                if (snprintf(v3str, sizeof(v3str), "%f", msg_local.value3) < 0){
                    perror("Could not perform conversion\n");
                    return -1;
                }
                strcat(filecontent, msg_local.value1);
                strcat(filecontent, ";");
                strcat(filecontent, v2str);
                strcat(filecontent, ";");
                strcat(filecontent, v3str);
                strcat(filecontent, ";");*/
                sprintf(filecontent, "%s;%d;%f;", msg_local.value1, msg_locsal.value2, msg_local.value3);
                printf("modify filecontent: %s\n",filecontent);
                if((msg_mod = fopen(file_name, "w")) == NULL){
                    perror("Error opening file\n");
                    return -1;
                }
                if (pthread_mutex_lock(&mymutex) != 0){
                    perror("Error mutex_lock\n");
                    return -1;
                }
                if(fwrite(filecontent, 1, strlen(filecontent), msg_mod) != strlen(filecontent)){
                    perror("Error writing in file\n");
                    return -1;
                }
                if (pthread_mutex_unlock(&mymutex) != 0){
                    perror("Error mutex_unlock\n");
                    return -1;
                }
                if (fclose(msg_mod) == EOF){
                    perror("Error closing file\n");
                    return -1;
                }
                myres = 0;
            }
            
            if(mq_send(q_client, (char *)  &myres, sizeof(int), 0) ==-1){
                perror("mq_send");
                return -1;
            }
            if (mq_close(q_client) == -1){
                perror("Error closing client queue\n");
                return -1;
            }
            if (closedir(msgdir) == -1){
                perror("Error closing directory\n");
                return -1;
            }
            break;
        case 4:
            /*delete_key()*/
            q_client = mq_open(client_name, O_CREAT|O_RDWR, 0700, &c_attr);
            if(q_client == -1){
                perror("Can't create client queue");
                return -1;
            }
            if ((msgdir = opendir(msg_dir_name)) == NULL){
                perror("Error opening directory\n");
                return -1;
            }
            printf("Delete key, dir open\n");
            in_dir = 1;
            errno = 0;
            while (((msgcont = readdir(msgdir)) != NULL) && in_dir == 1){
                printf("Delete key, filename: %s\n", msgcont->d_name);
                printf(" fname: %d\n", atoi(msgcont->d_name));
                printf("mykey : %d\n", msg_local.key);
                if (pthread_mutex_lock(&mymutex) != 0){
                    perror("Error mutex_lock\n");
                    return -1;
                }
                if (atoi(msgcont->d_name)){
                    perror("Could not perform conversion\n");
                    return -1;
                }
                if (atoi(msgcont->d_name) != msg_local.key){
                    printf("key no exists, in_dir = %d\n", in_dir);
                    in_dir = 1;
                    myres = -1;
                }
                else {
                    in_dir = 0; //key exists
                    printf("key exists, in_dir = %d\n", in_dir);
                    /*strcpy(file_to_delete_path, "");
                    strcat(file_to_delete_path, msg_dir_name);
                    strcat(file_to_delete_path, "/");
                    strcat(file_to_delete_path, msgcont->d_name);*/
                    sprintf(file_to_delete_path, "%s/%s", msg_dir_name, msgcont->d_name);
                    if (remove(file_to_delete_path) == -1){
                        perror("Error removign file\n");
                        return -1;
                    }
                    myres = 0;
                }
                if (pthread_mutex_unlock(&mymutex) != 0){
                    perror("Error mutex_unlock\n");
                    return -1;
                }
            }
            if(msgcont == NULL && errno != 0){
                perror("Error reading directory\n");
                return -1;
            }
            if(mq_send(q_client, (char *) &myres, sizeof(int), 0)== -1){
                perror("mq_send");
                return -1;
            }
            if( mq_close(q_client) ==-1){
                perror("mq_close");
                return -1;
            }
            if( closedir(msgdir)){
                perror("closedir");
                return -1;
            }
            break;
        case 5:
            /*exist()*/
            q_client = mq_open(client_name, O_CREAT|O_RDWR, 0700, &c_attr);
            if(q_client == -1){
                perror("Can't create client queue");
                return -1;
            }
            if ((msgdir = opendir(msg_dir_name)) == NULL){
                perror("Error opening directory\n");
                return -1;
            }
            printf("Get value, dir open\n");
            in_dir = 1;
            errno = 0;
            while (((msgcont = readdir(msgdir)) != NULL) && in_dir == 1){
                printf("Get value, filename: %s", msgcont->d_name);
                printf(" fname: %d\n", atoi(msgcont->d_name));
                printf("mykey : %d\n", msg_local.key);
                if (pthread_mutex_lock(&mymutex) != 0){
                    perror("Error mutex_lock\n");
                    return -1;
                }
                if (atoi(msgcont->d_name) == 0){
                    perror("Could not perform conversion\n");
                    return -1;
                }
                if (atoi(msgcont->d_name) != msg_local.key){
                    printf("key no exists, in_dir = %d\n", in_dir);
                    in_dir = 1;
                    myres = 0;
                }
                else {
                    in_dir = 0; //key exists
                    printf("key exists, in_dir = %d\n", in_dir);
                    myres = 1;
                }
                if (pthread_mutex_unlock(&mymutex) != 0){
                    perror("Error mutex_unlock\n");
                    return -1;
                }
            }
            if(msgcont == NULL && errno != 0){
                perror("Error reading directory\n");
                return -1;
            }
           if(mq_send(q_client, (char *) &myres, sizeof(int), 0) == -1){
                perror("mq_send");
                return -1;
            }
            if(mq_close(q_client)==-1){
                perror("mq_close");
                return -1;
            }
            if (closedir(msgdir) == -1){
                perror("Error closing directory\n");
                return -1;
            }
            break;
        case 6:
            /*num_items*/
            q_client = mq_open(client_name, O_CREAT|O_RDWR, 0700, &c_attr);
            if(q_client == -1){
                perror("Can't create client queue");
                return -1;
            }
            num_items = 0;
            if ((msgdir = opendir(msg_dir_name)) == NULL){
                perror("Error opening directory\n");
                return -1;
            } 
            printf("hola");
            errno = 0;
            while ((msgcont = readdir(msgdir)) != NULL) {
                if (pthread_mutex_lock(&mymutex) != 0){
                    perror("Error mutex_lock\n");
                    return -1;
                }
                if (msgcont->d_type == DT_REG) { /* If the entry is a regular file */
                    num_items++;
                }
                if (pthread_mutex_unlock(&mymutex) != 0){
                    perror("Error mutex_unlock\n");
                    return -1;
                }
            }
            if(msgcont == NULL && errno != 0){
                perror("Error reading directory\n");
                return -1;
            } 
            printf("number of files: %d\n", num_items);   
            if (mq_send(q_client, (char *) &num_items, sizeof(int), 0) == -1){
                perror("mq_send");
                return -1;
            }
            if( mq_close(q_client) == -1){
                perror("mq_close");
                return -1;
            }
            if( closedir(msgdir) == -1){
                perror("closedir");
                return -1;
            }
            break;
        default:
            return 0;
            break;
    }
    pthread_exit(0);
}


int main(void) {

    struct message_request mymessage;
    //struct message_response myresponse;
   
    struct mq_attr q_attr;
    pthread_attr_t t_attr; 
    pthread_t threadId;

    q_attr.mq_maxmsg = 5;
    q_attr.mq_msgsize = sizeof(struct message_request);

    q_server = mq_open(server_name, O_CREAT | O_RDONLY, 0644, &q_attr);
    if(q_server == -1){
        perror("Can't create server queue");
        return -1;
    }

    if (pthread_mutex_init(&mutex_msg, NULL) != 0){
        perror("Error mutex_init\n");
        return -1;
    }
    if (pthread_mutex_init(&mymutex, NULL) != 0){
        perror("Error mutex_init\n");
        return -1;
    }
    if (pthread_cond_init(&cond_msg, NULL) != 0){
        perror("Error cond_init\n");
        return -1;
    }
    if (pthread_attr_init(&t_attr) != 0){
        perror("Error attr_init\n");
        return -1;
    }

    if (pthread_attr_setdetachstate(&t_attr, PTHREAD_CREATE_DETACHED) != 0){
        perror("Error attr_setdetachstate\n");
        return -1;
    }

    while(1) {
        if (mq_receive(q_server, (char *) &mymessage, sizeof(struct message_request), 0) == -1){
            perror("Error receiving message queue in server.c\n");
            return -1;
        }

        
        if (pthread_create(&threadId, &t_attr, (void *)process_message, &mymessage) != 0){
            perror("Error creating thread\n");
            return -1;
        }
        if (pthread_mutex_lock(&mutex_msg) != 0){
            perror("Error mutex_lock\n");
            return -1;
        }
        while (message_not_copied){
            if (pthread_cond_wait(&cond_msg, &mutex_msg) != 0){
                perror("Error cond_wait\n");
                return -1;
            }
        }
        message_not_copied = true;
        if (pthread_mutex_unlock(&mutex_msg) != 0){
            perror("Error mutex_unlock\n");
            return -1;
        }
    }

    return 0;
}


