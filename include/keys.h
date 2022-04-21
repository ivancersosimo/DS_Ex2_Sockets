#include <unistd.h>
#include <sys/socket.h>

#define MAXSIZE 256

struct message_request {
    int32_t key;
    char value1[MAXSIZE];
    int32_t  value2;
    float value3;
    int op;
};


int init();
int set_value(int32_t key, char *value1, int32_t value2, float value3);
int get_value(int32_t key, char *value1, int32_t *value2, float *value3);
int modify_value(int32_t key, char *value1, int32_t value2, float value3);
int delete_key(int32_t key);
int exist(int32_t key);
int num_items();
int exit_key();
int sendMessage(int socket, char *buffer, int len);
int recvMessage(int socket, char *buffer, int len);
ssize_t readLine(int fd, void *buffer, size_t n);
