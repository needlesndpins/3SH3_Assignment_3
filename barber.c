#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>

/* barber shop monitor */

int barber = 0, chair = 0, exit = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;          /* monitor lock */
pthread_cond_t barberAvailable = PTHREAD_COND_INITIALIZER;  /* signalling condition: barber > 0 */
pthread_cond_t chairOccupied = PTHREAD_COND_INITIALIZER;    /* signalling condition: chair > 0 */
pthread_cond_t exitOpen = PTHREAD_COND_INITIALIZER;         /* signalling condition: exit > 0 */
pthread_cond_t customerLeft = PTHREAD_COND_INITIALIZER;     /* signalling condition: exit = 0 */

/* monitor invariant: 0 <= barber <= 1 && 0 <= chair <= 1 && 0 <= exit <= 1 */

void getHaircut(long cust) {
    pthread_mutex_lock(&mutex);                             /* monitor entry */
        printf("%d waiting for barber\n", cust);
        while (barber == 0) pthread_cond_wait(&barberAvailable, &mutex);
        barber -= 1;
        chair += 1; pthread_cond_signal(&chairOccupied);
        printf("%d waiting for exit door\n", cust);
        while (exit == 0) pthread_cond_wait(&exitOpen, &mutex);
        exit -= 1; pthread_cond_signal(&customerLeft);
    pthread_mutex_unlock(&mutex);                            /* monitor exit */
}
void getNextCustomer() {
    pthread_mutex_lock(&mutex);                              /* monitor entry */
        barber += 1 ; pthread_cond_signal(&barberAvailable);
        printf("waiting for customer to sit in chair\n");
        while (chair == 0) pthread_cond_wait(&chairOccupied, &mutex);
        chair -= 1;
    pthread_mutex_unlock(&mutex);                            /* monitor exit */
}
void finishedCut() {
    pthread_mutex_lock(&mutex);                              /* monitor entry */
        exit += 1; pthread_cond_signal(&exitOpen);
        printf("waiting for customer to leave\n");
        while (exit > 0) pthread_cond_wait(&customerLeft, &mutex);
    pthread_mutex_unlock(&mutex);                            /* monitor exit */
}

/* end barber shop monitor */

void *villagebarber() {
    while (true) {
        getNextCustomer();  /* wait for a customer to sit in the barber's chair */
        printf("cutting hair\n"); sleep(1);
        finishedCut();      /* allow the customer to leave; returns after the customer left */
    }
}
void *customer(void *t) {
    long cust = (long) t;
    while (true) {
        printf("%d lives happily\n", cust); sleep(2);
        getHaircut(cust);
    }
}

int main(int argc, char *argv[]) {
    int students = atoi(argv[1]);
    long i;
    pthread_t b, c[students];
    pthread_create(&b, NULL, &villagebarber, NULL);
    for (i = 0; i < students; i++) pthread_create(&c[i], NULL, &customer, (void *) i);
    pthread_join(b, NULL);
    for (i = 0; i < students; i++) pthread_join(c[i], NULL);
}
