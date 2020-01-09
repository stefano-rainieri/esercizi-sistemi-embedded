#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

pthread_t main_id;

void *body(void *arg) {
    int parameter = *(int *)arg;
    printf("parameter: %d \n", parameter);

    pthread_t body_id = pthread_self();
    printf("body_id == main_id: %d \n", pthread_equal(main_id, body_id));

    return (void *)5678;
}

int main() {
    pthread_attr_t attribute;
    pthread_t thread;
    void *return_value;

    int parameter = 1234;

    puts("pthread_attr_init");
    pthread_attr_init(&attribute);

    main_id = pthread_self();
    int error = pthread_create(&thread, &attribute, body, (void *)&parameter);
    pthread_attr_destroy(&attribute);

    pthread_join(thread, &return_value);
    printf("return_value: %d \n", (int)return_value);

    return 0;
}