#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

#define LOWER_NUM 1
#define UPPER_NUM 10000
#define BUFFER_SIZE 100
#define MAX_COUNT 10000

int buffer[BUFFER_SIZE];
int buffer_index = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t empty_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t full_cond = PTHREAD_COND_INITIALIZER;
int producer_finished = 0;
int even_count = 0;
int odd_count = 0;



// Producer function to generate random numbers and write them to a file

void *producer(void *arg) {

    // Open file for writing

    FILE *file = fopen("all.txt", "w");

    if (file == NULL) {

        perror("Error opening file");

        exit(EXIT_FAILURE);

    }
    while (1) {

        // Lock mutex before accessing shared resources

        pthread_mutex_lock(&lock);
        while (buffer_index == BUFFER_SIZE) {

            pthread_cond_wait(&empty_cond, &lock);

        }
        // Check if the required number of elements has been produced

        if (even_count + odd_count >= MAX_COUNT) {
            producer_finished = 1;
            pthread_cond_broadcast(&full_cond);
            pthread_mutex_unlock(&lock);
            fclose(file);
            break;

        }
        int number = rand() % 10000 + 1;

        // Add number to buffer
        buffer[buffer_index++] = number;
        fprintf(file, "%d\n", number);
        if (number % 2 == 0) {
            even_count++;

        } else {
            odd_count++;

        }
        // Signal that buffer is not empty

        pthread_cond_signal(&full_cond);

        // Unlock mutex

        pthread_mutex_unlock(&lock);

        // Introduce a small delay

        usleep(100);

    }



    return NULL;

}
// Customer function to read numbers from buffer and write them to a file based on parity

void *customer(void *arg) {

    // Determine whether to write even or odd numbers to file

    int parity = *((int *)arg);

    char filename[20];

    sprintf(filename, "%s.txt", (parity == 0) ? "even" : "odd");

    FILE *file = fopen(filename, "a");

    // Loop to read numbers from buffer

    while (true) {
        pthread_mutex_lock(&lock);
        if (buffer_index == 0 && producer_finished) {
            pthread_mutex_unlock(&lock);

            fclose(file);

            break;

        }

        if (buffer_index > 0) {

            int number = buffer[buffer_index-1];

            if (number % 2 == parity) {

                if (file != NULL) {

                    fprintf(file, "%d\n", number);

                }

                

                buffer_index--;

            }

        }

        

        pthread_mutex_unlock(&lock);

    }



    return NULL;

}

int main() {

    // Seed the random number generator

    srand(time(NULL));



    // Initialize thread identifiers for producer and customers

    pthread_t prod_tid, cust1_tid, cust2_tid;

    int cust_odd_parity = 0;

    int cust_even_parity = 1;
    pthread_create(&prod_tid, NULL, producer, NULL);

    pthread_create(&cust1_tid, NULL, customer, &cust_odd_parity);

    pthread_create(&cust2_tid, NULL, customer, &cust_even_parity);
    pthread_join(prod_tid, NULL);

    pthread_join(cust1_tid, NULL);

    pthread_join(cust2_tid, NULL);

    return 0;

}

