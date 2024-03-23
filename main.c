#include <stdio.h>

#include <stdlib.h>

#include <pthread.h>

#include <time.h>

#include <unistd.h> 



#define LOWER_NUM 1

#define UPPER_NUM 10000

#define BUFFER_SIZE 100

#define MAX_COUNT 100



int buffer[BUFFER_SIZE];

int count = 0;

int consumed = 0;



pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t cond_full = PTHREAD_COND_INITIALIZER;



void* producer(void* arg) {

    FILE* all_file = fopen("all.txt", "w");



    for (int i = 0; i < MAX_COUNT; i++) {

        pthread_mutex_lock(&mutex);



        while (count == BUFFER_SIZE)

            pthread_cond_wait(&cond_full, &mutex);



        int num = LOWER_NUM + rand() % (UPPER_NUM - LOWER_NUM + 1);

        buffer[count++] = num;



        fprintf(all_file, "%d\n", num); // Write to all.txt



        pthread_cond_signal(&cond_full);

        pthread_mutex_unlock(&mutex);

    }



    fclose(all_file);

    pthread_exit(NULL);

}



void* consumer(void* arg) {

    int parity = *(int*)arg;

    char even_filename[] = "even.txt";

    char odd_filename[] = "odd.txt";

    FILE* even_file = fopen(even_filename, "a"); // Open in append mode

    FILE* odd_file = fopen(odd_filename, "a"); // Open in append mode



    if (even_file == NULL || odd_file == NULL) {

        perror("Error opening file");

        pthread_exit(NULL);

    }



    struct timespec sleep_time;

    sleep_time.tv_sec = 0;

    sleep_time.tv_nsec = 1000000; // 1 millisecond



    while (1) {

        pthread_mutex_lock(&mutex);



        if (consumed >= MAX_COUNT || count > 0) {

            if (count > 0) {

                int num = buffer[--count];

                if (num % 2 == 0) { // Check if num is even

                    printf("Writing %d to even.txt\n", num);

                    fprintf(even_file, "%d\n", num);

                    fflush(even_file); // Flush the file to ensure immediate writing

                } else { // num is odd

                    printf("Writing %d to odd.txt\n", num);

                    fprintf(odd_file, "%d\n", num);

                    fflush(odd_file); // Flush the file to ensure immediate writing

                }

                consumed++;

            }



            pthread_mutex_unlock(&mutex);

        } else {

            pthread_mutex_unlock(&mutex);

            nanosleep(&sleep_time, NULL); // Sleep for a short time to avoid busy waiting

        }



        if (consumed >= MAX_COUNT)

            break;

    }



    if (fclose(even_file) != 0 || fclose(odd_file) != 0) {

        perror("Error closing file");

        pthread_exit(NULL);

    }



    pthread_exit(NULL);

}



int main() {

    srand(time(NULL));



    pthread_t producer_thread, consumer_thread1, consumer_thread2;

    int even = 0;

    int odd = 1;



    pthread_create(&producer_thread, NULL, producer, NULL);

    pthread_create(&consumer_thread1, NULL, consumer, &even);

    pthread_create(&consumer_thread2, NULL, consumer, &odd);



    pthread_join(producer_thread, NULL);

    pthread_join(consumer_thread1, NULL);

    pthread_join(consumer_thread2, NULL);



    pthread_mutex_destroy(&mutex);

    pthread_cond_destroy(&cond_full);



    return 0;

}

