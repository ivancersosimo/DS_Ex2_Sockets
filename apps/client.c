#include "keys.h"
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char * argv[]) {
    if(argc < 2){
        printf("./client <op> <key> <value1> <value2> <value3>\n");
        printf ("op codes:\n");
        printf("\t0    init()\n");
        printf("\t1    set_value(key, value1, value2, va)\n");
        printf("\t2    get_value(key, value1, value2, value3)\n");
        printf("\t3    modify_value(key, value1, value2, value3)\n");
        printf("\t4    delete_key(key)\n");
        printf("\t5    exist(key)\n");
        printf("\t6    num_items()\n");
        
        return -1;
    }
    int op = atoi(argv[1]);
    int key;
    char *value1;
    int value2; 
    float value3;
    if(op > 0 && op != 2 && op != 6 && op != 5 && op != 4){
        key = atoi(argv[2]);
        value1 = argv[3];
        value2 = atoi(argv[4]);
        value3 =  atof(argv[5]);
    }
    int error;

    switch (op){
        case 0:
            if (argc != 2){
                printf("init()");
                return -1;
            }
            printf("init res: %d", init());
            break;
        case 1:
            if(argc != 6){
                 printf("set_value(key, value1, value2, value3)\n");
                 return -1;
            }
            printf("client set value\n");
            printf(" Set value res: %d\n" ,set_value(key, value1, value2, value3));
            break;
        case 2:
            if(argc != 3){
                 printf("get_value(key)\n");
                 return -1;
            }
            printf("client get value\n");
            printf("argv[2] %s \n", argv[2]);
            key = atoi(argv[2]);
            int get_res = get_value(key, value1, &value2, &value3);
            printf("Get value res: %d\n" ,get_res);
            //get_value(key, value1, &value2, &value3);
            if(get_res !=-1)
                printf("Value1: %s\n Value2: %d\n Value3: %f\n", value1, value2, value3);
            break;
        case 3:
            if(argc != 6){
                 printf("modify_value(key, valu1, value2, value3)\n");
                 return -1;
            }
            printf("modify res: %d\n" ,modify_value(key, value1, value2, value3));
            break;
        case 4:
            if (argc != 3){
              printf("delete_key(key)\n"); 
              return -1;
            }
            key = atoi(argv[2]);
            printf("delete res: %d\n",delete_key(key));
            break;
        case 5:
            if(argc != 3){
                 printf("exist(key)\n");
                 return -1;
            }
            key = atoi(argv[2]);
            printf("key: %d\n", key);
            printf("Exist:%d\n", exist(key));
            break;
        case 6:
            if(argc != 2){
                 printf("num_items()\n");
                 return -1;
            }
            printf("number of items: %d\n", num_items());
            break;
        default:
            return 0;
            break;
    }
return 0;
}