#include "keys.h"
#include <stdio.h>
#include <stdlib.h>

void print_op_code(){
    printf("\t0    init()\n");
    printf("\t1    set_value(key, value1, value2, value3)\n");
    printf("\t2    get_value(key, value1, value2, value3)\n");
    printf("\t3    modify_value(key, value1, value2, value3)\n");
    printf("\t4    delete_key(key)\n");
    printf("\t5    exist(key)\n");
    printf("\t6    num_items()\n");
}

int main(int argc, char * argv[]) {
    if(argc < 1){
        printf("IP_TUPLES=<ip> PORT_TUPLES=<port> ./client\n");
        return -1;
    }
    //int op = atoi(argv[1]);
    int op ;
    int32_t key;
    char *value1;
    int32_t value2; 
    float value3;
    /*
    if(op > 0 && op != 2 && op != 6 && op != 5 && op != 4 && op < 7){
        key = atoi(argv[2]);
        value1 = argv[3];
        value2 = atoi(argv[4]);
        printf("client Value2: %d\n", value2);
        value3 =  atof(argv[5]);
        printf("client value3: %f\n", value3);
    }
    */
    while(1){
        printf("Enter operation code: ");
        scanf("%d", &op);
        while (0<op>7){
            printf ("op codes:\n");
            print_op_code();
        }

        int error, res, check;

        switch (op){
            case 0:
                res = init();
                printf("init res: %d\n", res);
                break;
            case 1:
                printf("Client set value\n");
                printf("<key> <value1> <value2> <value3>\n");
                check = scanf("%d %s %d %f", &key, value1, &value2, &value3);
                while(check != 4){
                    printf("Arguments data type is not valid\n");
                    printf("<key> <value1> <value2> <value3>\n");
                    check = scanf("%d %s %d %f", &key, value1, &value2, &value3);
                }
                res = set_value(key, value1, value2, value3);
                printf(" Set value res: %d\n", res);
                break;
            case 2:
                printf("Client get value\n");
                printf("<key>\n");
                check = scanf("%d", &key);
                while(check != 1){
                    printf("Argument data type is not valid\n");
                    printf("<key>\n");
                    check = scanf("%d", &key);
                }
                int get_res = get_value(key, value1, &value2, &value3);
                printf("Get value res: %d\n" ,get_res);
                //get_value(key, value1, &value2, &value3);
                if(get_res !=-1)
                    printf("Value1: %s\n Value2: %d\n Value3: %f\n", value1, value2, value3);
                break;
            case 3:
                printf("Client modify value\n");
                printf("<key> <value1> <value2> <value3>\n");
                check = scanf("%d %s %d %f", &key, value1, &value2, &value3);
                while(check != 4){
                    printf("Arguments data type is not valid\n");
                    printf("<key> <value1> <value2> <value3>\n");
                    check = scanf("%d %s %d %f", &key, value1, &value2, &value3);
                }
                res = modify_value(key, value1, value2, value3);
                printf("modify res: %d\n", res);
                break;
            case 4:
                printf("CLient delete key\n"); 
                printf("<key>\n");
                scanf("%d", &key);
                check = scanf("%d", &key);
                while(check != 1){
                    printf("Argument data type is not valid\n");
                    printf("<key>\n");
                    check = scanf("%d", &key);
                }
                res = delete_key(key);
                printf("delete res: %d\n", res);
                break;
            case 5:
                printf("Client exist\n");
                printf("<key>\n");
                check = scanf("%d", &key);
                while(check != 1){
                    printf("Argument data type is not valid\n");
                    printf("<key>\n");
                    check = scanf("%d", &key);
                }
                printf("key: %d\n", key);
                res = exist(key);
                printf("Exist:%d\n", res);
                break;
            case 6:
                printf("Client num_items\n");
                res = num_items();
                printf("number of items: %d\n", res);
                break;
            case 7:
                printf("Exit\n");
                res = exit_key();
                printf("exit res: %d\n", res);
                return 0;
                break;
            default:
                printf ("op codes:\n");
                print_op_code();
                break;
        }
    }
return 0;
}