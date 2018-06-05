#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
typedef struct Obj{
    int* ptr;
}Obj;


void* initObj(void* arg)
{
    int value = 10;
    Obj* obj = (Obj*)arg;
    //int* ptrInside = (int*) malloc(sizeof(int));
    //*ptrInside = 0;

    
    *obj->ptr = value;

    printf("*obj->ptr[%p] = %d\n",obj->ptr,*obj->ptr);
    while(1){};

    return NULL;
}

int main()
{
    printf("test ptr\n");
    Obj* myObj = (Obj*)malloc(sizeof(Obj));
    myObj->ptr = NULL;
    myObj->ptr = (int*)malloc(sizeof(int));

    pthread_t render_thread;
    pthread_create(&render_thread, NULL, initObj, myObj);
    //initObj(myObj,10);
    
    printf("after init \n");
    printf("my obj-ptr %p\n",myObj->ptr);
    printf("outer *obj->ptr[%p] = %d\n",myObj->ptr,*myObj->ptr);

    return 0;
}