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
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include "keys.h"
#include <inttypes.h> /* strtoimax */

pthread_mutex_t mutex_msg, mymutex;
bool message_not_copied = true;
pthread_cond_t cond_msg;

char msg_dir_name[] = "messages_dir";

int sd, sc;


int process_message(int myop){
    struct message_request msg_local;
    
    /*if (pthread_mutex_lock(&mutex_msg) != 0){
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
    }*/

    struct message_response msgres_local;
    int err, sd;
    DIR *msgdir;
    struct dirent * msgcont;
    char file_name[100], file_to_delete_path[50];
    char KeyString[20], v2str[20], v3str[20];
    char filecontent[sizeof(struct message_request)];
    int myres, mymsg, in_dir, file_cont, num_items;
    FILE *msg_mod;
    char buff[sizeof(struct message_request)];
    char to_read_value1[50], to_read_value2[50], to_read_value3[50];

    switch(myop){
        case 0:
            /*init()*/
            if ((msgdir = opendir(msg_dir_name)) == NULL){
                perror("Error opening directory\n");
                close(sc);
                close(sd);
                pthread_exit(0);
                return -1;
            }
            errno = 0;
            while ((msgcont = readdir(msgdir)) != NULL){
                sprintf(file_to_delete_path, "%s/%d", msg_dir_name, *msgcont->d_name);
                            
                if (msgcont->d_type == DT_REG){
                    printf("%s\n",file_to_delete_path);
                    if (pthread_mutex_lock(&mymutex) != 0){
                        perror("Error mutex_lock\n");
                        close(sc);
                        closedir(msgdir);
                        close(sd);
                        pthread_exit(0);
                        return -1;
                    }
                    if (remove(file_to_delete_path) == -1){
                        perror("Error removign file\n");
                        pthread_mutex_unlock(&mymutex);
                        close(sc);
                        closedir(msgdir);
                        close(sd);
                        pthread_exit(0);
                        return -1;
                    }
                    if (pthread_mutex_unlock(&mymutex) != 0){
                        perror("Error mutex_unlock\n");
                        close(sc);
                        closedir(msgdir);
                        close(sd);
                        pthread_exit(0);
                        return -1;
                    }
                    printf("File is removed\n");
                }
            }
            if(msgcont == NULL && errno != 0){
                perror("Error reading directory\n");
                close(sc);
                closedir(msgdir);
                close(sd);
                pthread_exit(0);
                return -1;
            }
            myres = 0;
            err = sendMessage(sc, (char *) &myres, sizeof(int));  // envía la operacion
            if (err == -1){
                printf("Error sending\n");
                close(sc);
                closedir(msgdir);
                close(sd);
                pthread_exit(0);
                return -1;
            }
            if(close(sc) == -1){
                perror("Error closing socket\n");
                closedir(msgdir);
                close(sd);
                pthread_exit(0);
                return -1;
            }

            if(closedir(msgdir) == -1){
                perror("Error closing directory\n");
                close(sd);
                pthread_exit(0);
                return -1;
            }
            break;
        case 1:
            /*set_value() */
            err = recvMessage(sc, (char *) &msg_local.key, sizeof(int));  // envía la operacion
            if (err == -1){
                printf("Error receiving\n");
                close(sc);
                close(sd);
                pthread_exit(0);
                return -1;
            }
            /*err = recvMessage(sd, (char *) &msg_local.value1, sizeof(int));  // envía la operacion
            if (err == -1){
                printf("Error receiving\n");
                return -1;
            }*/
            err = recvMessage(sc, (char *) &msg_local.value2, sizeof(int));  // envía la operacion
            if (err == -1){
                printf("Error receiving\n");
                close(sc);
                close(sd);
                pthread_exit(0);
                return -1;
            }
            err = recvMessage(sc, (char *) &msg_local.value3, sizeof(float));  // envía la operacion
            if (err == -1){
                printf("Error receiving\n");
                close(sc);
                close(sd);
                pthread_exit(0);
                return -1;
            }
            if ((msgdir = opendir(msg_dir_name)) == NULL){
                perror("Error opening directory\n");
                close(sc);
                close(sd);
                pthread_exit(0);
                return -1;
            }
            in_dir = 1;
            errno = 0;
            while ((msgcont = readdir(msgdir)) != NULL && in_dir == 1){
                printf("Set value, mkey: %d\n", msg_local.key);
                if (atoi(msgcont->d_name) == 0){
                    perror("Could not perform conversion\n");
                    close(sc);
                    closedir(msgdir);
                    close(sd);
                    pthread_exit(0);
                    return -1;
                }
                if (pthread_mutex_lock(&mymutex) != 0){
                    perror("Error mutex_lock\n");
                    close(sc);
                    closedir(msgdir);
                    close(sd);
                    pthread_exit(0);
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
                    close(sc);
                    closedir(msgdir);
                    close(sd);
                    pthread_exit(0);
                    return -1;
                }
            }
            if(msgcont == NULL && errno != 0){
                perror("Error reading directory\n");
                close(sc);
                close(sd);
                pthread_exit(0);
                return -1;
            }
            if (in_dir == 1){
                sprintf(file_name, "%s/%d", msg_dir_name, msg_local.key);
                printf("filename: %s\n",file_name);

                sprintf(filecontent, "%s;%d;%f;", msg_local.value1, msg_local.value2, msg_local.value3);
                printf("filecontent: %s\n",filecontent);
                if((mymsg = open(file_name, O_CREAT | O_RDWR, 0644)) == -1){
                    perror("Error opening file\n");
                    close(sc);
                    closedir(msgdir);
                    close(sd);
                    pthread_exit(0);
                    return -1;
                }
                if (pthread_mutex_lock(&mymutex) != 0){
                    perror("Error mutex_lock\n");
                    close(mymsg);
                    close(sc);
                    closedir(msgdir);
                    close(sd);
                    pthread_exit(0);
                    return -1;
                }
                if (write(mymsg, &filecontent, sizeof(filecontent)) == -1){
                    perror("Error writing in file\n");
                    pthread_mutex_unlock(&mymutex);
                    close(mymsg);
                    close(sc);
                    closedir(msgdir);
                    close(sd);
                    pthread_exit(0);
                    return -1;
                }
                if (pthread_mutex_unlock(&mymutex) != 0){
                        perror("Error mutex_unlock\n");
                        close(mymsg);
                        close(sc);
                        closedir(msgdir);
                        close(sd);
                        pthread_exit(0);
                        return -1;
                    }
                if (close(mymsg) == -1){
                    perror("Error closing file\n");
                    close(sc);
                    closedir(msgdir);
                    close(sd);
                    pthread_exit(0);
                    return -1;
                }
                myres = 0;
            }
            printf("myres2: %d\n ",myres);
            err = sendMessage(sc, (char *) &myres, sizeof(int));  // envía la operacion
            if (err == -1){
                printf("Error receiving\n");
                close(sc);
                closedir(msgdir);
                close(sd);
                pthread_exit(0);
                return -1;
            }
            if (close(sc) == -1){
                perror("Error closing spcket\n");
                closedir(msgdir);
                close(sd);
                pthread_exit(0);
                return -1;
            }
            if(closedir(msgdir) == -1){
                perror("Error closing directory\n");
                close(sd);
                pthread_exit(0);
                return -1;
            }
            break;
        case 2:
            /*get_value()*/
            err = recvMessage(sc, (char *) &msg_local.key, sizeof(int));  // envía la operacion
            if (err == -1){
                printf("Error receiving\n");
                close(sc);
                close(sd);
                pthread_exit(0);
                return -1;
            }
            if ((msgdir = opendir(msg_dir_name)) == NULL){
                perror("Error opening directory\n");
                close(sc);
                close(sd);
                pthread_exit(0);
                return -1;
            }
            printf("Get value, dir open\n");
            in_dir = 1;
            errno = 0;
            while (((msgcont = readdir(msgdir)) != NULL) && in_dir == 1){
                printf("Get value, filename: %s", msgcont->d_name);
                if (pthread_mutex_lock(&mymutex) != 0){
                    perror("Error mutex_lock\n");
                    close(sc);
                    closedir(msgdir);
                    close(sd);
                    pthread_exit(0);
                    return -1;
                }
                if(atoi(msgcont->d_name) == 0){
                    perror("Could not perform conversion\n");
                    pthread_mutex_unlock(&mymutex);
                    close(sc);
                    closedir(msgdir);
                    close(sd);
                    pthread_exit(0);
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
                    close(sc);
                    closedir(msgdir);
                    close(sd);
                    pthread_exit(0);
                    return -1;
                }
            }
            if(msgcont == NULL && errno != 0){
                perror("Error reading directory\n");
                close(sc);
                closedir(msgdir);
                close(sd);
                pthread_exit(0);
                return -1;
            }
            if (in_dir == 0){
                sprintf(file_name, "%s/%d", msg_dir_name, msg_local.key);
                printf("filename: %s\n",file_name);

                if((mymsg = open(file_name, O_RDONLY, 0644)) == -1){
                    perror("Error opening file\n");
                    close(sc);
                    closedir(msgdir);
                    close(sd);
                    pthread_exit(0);
                    return -1;
                }
                printf("open works\n");
                if (pthread_mutex_lock(&mymutex) != 0){
                    perror("Error mutex_lock\n");
                    close(mymsg);
                    close(sc);
                    closedir(msgdir);
                    close(sd);
                    pthread_exit(0);
                    return -1;
                }
                if((file_cont = read(mymsg, &filecontent, sizeof(filecontent))) == -1){
                    perror("Error reading file\n");
                    pthread_mutex_unlock(&mymutex);
                    close(mymsg);
                    close(sc);
                    closedir(msgdir);
                    close(sd);
                    pthread_exit(0);
                    return -1;
                }
                if (pthread_mutex_unlock(&mymutex) != 0){
                    perror("Error mutex_unlock\n");
                    close(mymsg);
                    close(sc);
                    closedir(msgdir);
                    close(sd);
                    pthread_exit(0);
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
                    close(mymsg);
                    close(sc);
                    closedir(msgdir);
                    close(sd);
                    pthread_exit(0);
                    return -1;
                }
                msgres_local.value2 = atoi(strtrunc);
                printf("Value2: %d\n", msgres_local.value2);
                strtrunc = strtok(NULL, ";");
                if (atof(strtrunc) == 0.0){
                    perror("Could not perform conversion\n");
                    close(mymsg);
                    close(sc);
                    closedir(msgdir);
                    close(sd);
                    pthread_exit(0);
                    return -1;
                }
                msgres_local.value3 = (float)atof(strtrunc);
                printf("Value3: %f\n", msgres_local.value3);
                msgres_local.err = 0;
                if (close(mymsg) == -1){
                    perror("Error closing file\n");
                    close(sc);
                    closedir(msgdir);
                    close(sd);
                    pthread_exit(0);
                    return -1;
                }
            }
            
            /*err = sendMessage(sd, (char *) &msg_local.value1, sizeof(char));  // envía la operacion
            if (err == -1){
                printf("Error sending\n");
                return -1;
            }*/
            err = sendMessage(sc, (char *) &msg_local.value2, sizeof(int));  // envía la operacion
            if (err == -1){
                printf("Error sending\n");
                close(sc);
                closedir(msgdir);
                close(sd);
                pthread_exit(0);
                return -1;
            }
            err = sendMessage(sc, (char *) &msg_local.value3, sizeof(float));  // envía la operacion
            if (err == -1){
                printf("Error sending\n");
                close(sc);
                closedir(msgdir);
                close(sd);
                pthread_exit(0);
                return -1;
            }
            if (close(sc) == -1){
                perror("Error closing socket\n");
                closedir(msgdir);
                close(sd);
                pthread_exit(0);
                return -1;
            }
            if (closedir(msgdir) == -1){
                perror("Error closing directory\n");
                close(sd);
                pthread_exit(0);
                return -1;
            }
            break;
        case 3:
            /*modify_value()*/
            err = recvMessage(sc, (char *) &msg_local.key, sizeof(int));  // envía la operacion
            if (err == -1){
                printf("Error receiving\n");
                close(sc);
                close(sd);
                pthread_exit(0);
                return -1;
            }
            /*err = recvMessage(sd, (char *) &msg_local.value1, sizeof(int));  // envía la operacion
            if (err == -1){
                printf("Error receiving\n");
                return -1;
            }*/
            err = recvMessage(sc, (char *) &msg_local.value2, sizeof(int));  // envía la operacion
            if (err == -1){
                printf("Error receiving\n");
                close(sc);
                close(sd);
                pthread_exit(0);
                return -1;
            }
            err = recvMessage(sc, (char *) &msg_local.value3, sizeof(float));  // envía la operacion
            if (err == -1){
                printf("Error receiving\n");
                close(sc);
                close(sd);
                pthread_exit(0);
                return -1;
            }
            if ((msgdir = opendir(msg_dir_name)) == NULL){
                perror("Error opening directory\n");
                close(sc);
                close(sd);
                pthread_exit(0);
                return -1;
            }
            printf("Modify value, dir open\n");
            in_dir = 1;
            errno = 0;
            while (((msgcont = readdir(msgdir)) != NULL) && in_dir == 1){
                printf("Modify value, filename: %s", msgcont->d_name);
                if (pthread_mutex_lock(&mymutex) != 0){
                    perror("Error mutex_lock\n");
                    close(sc);
                    closedir(msgdir);
                    close(sd);
                    pthread_exit(0);
                    return -1;
                }
                if (atoi(msgcont->d_name) == 0){
                    perror("Could not perform conversion\n");
                    pthread_mutex_unlock(&mymutex);
                    close(sc);
                    closedir(msgdir);
                    close(sd);
                    pthread_exit(0);
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
                    close(sc);
                    closedir(msgdir);
                    close(sd);
                    pthread_exit(0);
                    return -1;
                }
            }
            if(msgcont == NULL && errno != 0){
                perror("Error reading directory\n");
                close(sc);
                closedir(msgdir);
                close(sd);
                pthread_exit(0);
                return -1;
            }
            if (in_dir == 0){
                sprintf(file_name, "%s/%d", msg_dir_name, msg_local.key);
                printf("modify filename: %s\n",file_name);
                sprintf(filecontent, "%s;%d;%f;", msg_local.value1, msg_local.value2, msg_local.value3);
                printf("modify filecontent: %s\n",filecontent);
                if((msg_mod = fopen(file_name, "w")) == NULL){
                    perror("Error opening file\n");
                    close(sc);
                    closedir(msgdir);
                    close(sd);
                    pthread_exit(0);
                    return -1;
                }
                if (pthread_mutex_lock(&mymutex) != 0){
                    perror("Error mutex_lock\n");
                    fclose(msg_mod);
                    close(sc);
                    closedir(msgdir);
                    close(sd);
                    pthread_exit(0);
                    return -1;
                }
                if(fwrite(filecontent, 1, strlen(filecontent), msg_mod) != strlen(filecontent)){
                    perror("Error writing in file\n");
                    pthread_mutex_unlock(&mymutex);
                    fclose(msg_mod);
                    close(sc);
                    closedir(msgdir);
                    close(sd);
                    pthread_exit(0);
                    return -1;
                }
                if (pthread_mutex_unlock(&mymutex) != 0){
                    perror("Error mutex_unlock\n");
                    fclose(msg_mod);
                    close(sc);
                    closedir(msgdir);
                    close(sd);
                    pthread_exit(0);
                    return -1;
                }
                if (fclose(msg_mod) == EOF){
                    perror("Error closing file\n");
                    close(sc);
                    closedir(msgdir);
                    close(sd);
                    pthread_exit(0);
                    return -1;
                }
                myres = 0;
            }
            err = sendMessage(sc, (char *) &myres, sizeof(int));  // envía la operacion
            if (err == -1){
                printf("Error sending\n");
                close(sc);
                closedir(msgdir);
                close(sd);
                pthread_exit(0);
                return -1;
            }
            if (close(sc) == -1){
                perror("Error closing socket\n");
                closedir(msgdir);
                close(sd);
                pthread_exit(0);
                return -1;
            }
            if (closedir(msgdir) == -1){
                perror("Error closing directory\n");
                close(sd);
                pthread_exit(0);
                return -1;
            }
            break;
        case 4:
            /*delete_key()*/
            err = recvMessage(sc, (char *) &msg_local.key, sizeof(int));  // envía la operacion
            if (err == -1){
                printf("Error receiving\n");
                close(sc);
                close(sd);
                pthread_exit(0);
                return -1;
            }
            if ((msgdir = opendir(msg_dir_name)) == NULL){
                perror("Error opening directory\n");
                close(sc);
                close(sd);
                pthread_exit(0);
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
                    close(sc);
                    closedir(msgdir);
                    close(sd);
                    pthread_exit(0);
                    return -1;
                }
                if (atoi(msgcont->d_name)){
                    perror("Could not perform conversion\n");
                    pthread_mutex_unlock(&mymutex);
                    close(sc);
                    closedir(msgdir);
                    close(sd);
                    pthread_exit(0);
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
                    sprintf(file_to_delete_path, "%s/%d", msg_dir_name, *msgcont->d_name);
                    if (remove(file_to_delete_path) == -1){
                        perror("Error removign file\n");
                        pthread_mutex_unlock(&mymutex);
                        close(sc);
                        closedir(msgdir);
                        close(sd);
                        pthread_exit(0);
                        return -1;
                    }
                    myres = 0;
                }
                if (pthread_mutex_unlock(&mymutex) != 0){
                    perror("Error mutex_unlock\n");
                    close(sc);
                    closedir(msgdir);
                    close(sd);
                    pthread_exit(0);
                    return -1;
                }
            }
            if(msgcont == NULL && errno != 0){
                perror("Error reading directory\n");
                close(sc);
                closedir(msgdir);
                close(sd);
                pthread_exit(0);
                return -1;
            }
            err = sendMessage(sc, (char *) &myres, sizeof(int));  // envía la operacion
            if (err == -1){
                printf("Error receiving\n");
                close(sc);
                closedir(msgdir);
                close(sd);
                pthread_exit(0);
                return -1;
            }
            if(close(sc) ==-1){
                perror("Error closing socket\n");
                closedir(msgdir);
                close(sd);
                pthread_exit(0);
                return -1;
            }
            if(closedir(msgdir)){
                perror("Error closing directory\n");
                close(sd);
                pthread_exit(0);
                return -1;
            }
            break;
        case 5:
            /*exist()*/
            err = recvMessage(sc, (char *) &msg_local.key, sizeof(int));  // envía la operacion
            if (err == -1){
                printf("Error receiving\n");
                close(sc);
                close(sd);
                pthread_exit(0);
                return -1;
            }
            if ((msgdir = opendir(msg_dir_name)) == NULL){
                perror("Error opening directory\n");
                close(sc);
                closedir(msgdir);
                close(sd);
                pthread_exit(0);
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
                    close(sc);
                    closedir(msgdir);
                    close(sd);
                    pthread_exit(0);
                    return -1;
                }
                if (atoi(msgcont->d_name) == 0){
                    perror("Could not perform conversion\n");
                    pthread_mutex_unlock(&mymutex);
                    close(sc);
                    closedir(msgdir);
                    close(sd);
                    pthread_exit(0);
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
                    close(sc);
                    closedir(msgdir);
                    close(sd);
                    pthread_exit(0);
                    return -1;
                }
            }
            if(msgcont == NULL && errno != 0){
                perror("Error reading directory\n");
                close(sc);
                closedir(msgdir);
                close(sd);
                pthread_exit(0);
                return -1;
            }
            err = sendMessage(sc, (char *) &myres, sizeof(int));  // envía la operacion
            if (err == -1){
                printf("Error sending\n");
                close(sc);
                closedir(msgdir);
                close(sd);
                pthread_exit(0);
                return -1;
            }
            if(close(sc)==-1){
                perror("socket close");
                closedir(msgdir);
                close(sd);
                pthread_exit(0);
                return -1;
            }
            if (closedir(msgdir) == -1){
                perror("Error closing directory\n");
                close(sd);
                pthread_exit(0);
                return -1;
            }
            break;
        case 6:
            /*num_items*/
            num_items = 0;
            if ((msgdir = opendir(msg_dir_name)) == NULL){
                perror("Error opening directory\n");
                close(sc);
                closedir(msgdir);
                close(sd);
                pthread_exit(0);
                return -1;
            } 
            printf("hola");
            errno = 0;
            while ((msgcont = readdir(msgdir)) != NULL) {
                if (pthread_mutex_lock(&mymutex) != 0){
                    perror("Error mutex_lock\n");
                    close(sc);
                    closedir(msgdir);
                    close(sd);
                    pthread_exit(0);
                    return -1;
                }
                if (msgcont->d_type == DT_REG) { /* If the entry is a regular file */
                    num_items++;
                }
                if (pthread_mutex_unlock(&mymutex) != 0){
                    perror("Error mutex_unlock\n");
                    close(sc);
                    closedir(msgdir);
                    close(sd);
                    pthread_exit(0);
                    return -1;
                }
            }
            if(msgcont == NULL && errno != 0){
                perror("Error reading directory\n");
                close(sc);
                closedir(msgdir);
                close(sd);
                pthread_exit(0);
                return -1;
            } 
            printf("number of files: %d\n", num_items);   
            err = sendMessage(sc, (char *) &num_items, sizeof(int));  // envía la operacion
            if (err == -1){
                printf("Error sending\n");
                close(sc);
                closedir(msgdir);
                close(sd);
                pthread_exit(0);
                return -1;
            }
            if(close(sc) == -1){
                perror("Error closing socket\n");
                closedir(msgdir);
                close(sd);
                pthread_exit(0);
                return -1;
            }
            if(closedir(msgdir) == -1){
                perror("Error closing directory\n");
                close(sd);
                pthread_exit(0);
                return -1;
            }
            break;
        default:
            return 0;
            break;
    }
    if(close(sd) == -1){
        perror("Error closing socket\n");
        pthread_exit(0);
        return -1;
    }
    pthread_exit(0);
}

static bool str_to_uint16(const char *str, uint16_t *res)
{
  char *end;
  errno = 0;
  intmax_t val = strtoimax(str, &end, 10);
  if (errno == ERANGE || val < 0 || val > UINT16_MAX || end == str || *end != '\0')
    return false;
  *res = (uint16_t) val;
  return true;
}

int main(void) {
    //int argc, char *argv[]
    /*if(argc != 2){
        printf("./server <port>\n");
        return -1;
    }*/

    struct message_request mymessage;

    struct sockaddr_in server_addr,  client_addr;
	socklen_t size;
    int val;

    int err;
    int op;
   
    pthread_attr_t t_attr; 
    pthread_t threadId;
    
  //  uint16_t myport = (short ) atoi(argv[1]);
    //uint16_t myport; 
    //uint16_t intport = atoi(argv[1]);
    //myport = (unsigned short int) intport;
    //uint16_t myport = *argv[1];
    uint16_t myport = 4200;

    // printf("my port: %hu\n", myport);
    if ((sd =  socket(AF_INET, SOCK_STREAM, 0))<0){
            printf ("SERVER: Error in socket");
            return (0);
    }
    val = 1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *) &val, sizeof(int));

	bzero((char *)&server_addr, sizeof(server_addr));
    	server_addr.sin_family      = AF_INET;
    	server_addr.sin_addr.s_addr = INADDR_ANY;
    	server_addr.sin_port        = htons(myport);

    err = bind(sd, (const struct sockaddr *)&server_addr, sizeof(server_addr));
	if (err == -1) {
		printf("Error bind\n");
        close(sd);
		return -1;
	}

    err = listen(sd, SOMAXCONN);
	if (err == -1) {
		printf("Error listen\n");
        close(sd);
		return -1;
	}

    size = sizeof(client_addr);


    if (pthread_mutex_init(&mutex_msg, NULL) != 0){
        perror("Error mutex_init\n");
        close(sd);
        return -1;
    }
    if (pthread_mutex_init(&mymutex, NULL) != 0){
        perror("Error mutex_init\n");
        close(sd);
        return -1;
    }
    if (pthread_cond_init(&cond_msg, NULL) != 0){
        perror("Error cond_init\n");
        close(sd);
        return -1;
    }
    if (pthread_attr_init(&t_attr) != 0){
        perror("Error attr_init\n");
        close(sd);
        return -1;
    }

    if (pthread_attr_setdetachstate(&t_attr, PTHREAD_CREATE_DETACHED) != 0){
        perror("Error attr_setdetachstate\n");
        close(sd);
        return -1;
    }

    while(1) {
        printf("Waiting connection...\n");
    	sc = accept(sd, (struct sockaddr *)&client_addr, (socklen_t *)&size);

		if (sc == -1) {
			printf("Error accept\n");
            close(sc);
            close(sd);
			return -1;
		}
		printf("Accepted connection IP: %s   Port: %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        err = recvMessage(sc, (char *) &op, sizeof(int));  // envía la operacion
        if (err == -1){
            printf("Error receiving\n");
            close(sc);
            close(sd);
            return -1;
        }
        
        if (pthread_create(&threadId, &t_attr, (void *)process_message, &op) != 0){
            perror("Error creating thread\n");
            close(sc);
            close(sd);
            return -1;
        }
        /*if (pthread_mutex_lock(&mutex_msg) != 0){
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
        }*/
    }

    return 0;
}


