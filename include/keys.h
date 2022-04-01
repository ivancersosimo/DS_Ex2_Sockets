#include <unistd.h>
#include <sys/queue.h>

#define MAXSIZE 256

struct message_request {
    int key;
    char value1[MAXSIZE];
    int  value2;
    float value3;
    int op;
    char q_name[MAXSIZE];
};


struct message_response {

    char value1[MAXSIZE];
    int  value2;
    float value3;
    int err;
};

int init();
int set_value(int key, char *value1, int value2, float value3);
int get_value(int key, char *value1, int *value2, float *value3);
int modify_value(int key, char *value1, int value2, float value3);
int delete_key(int key);
int exist(int key);
int num_items();
