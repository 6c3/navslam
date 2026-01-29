#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

// 執行緒要執行的函式
void* worker(void* arg) {
    int id = *((int*)arg);
    printf("Hello from thread %d!\n", id);
    return NULL;
}

int main() {
    pthread_t t1, t2;
    int id1 = 1, id2 = 2;

    // 建立兩個執行緒
    pthread_create(&t1, NULL, worker, &id1);
    pthread_create(&t2, NULL, worker, &id2);

    // 等待兩個執行緒結束
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("All threads finished!\n");
    return 0;
}
