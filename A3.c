
#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h> 

#define n 10

void *student();


pthread_mutex_t lock; //= PTHREAD_MUTEX_INITIALIZER; // Mutex

/** Problem formulation
 * The TA's office is small only has one desk with a chair and a computer 
 * 3 Chairs in the hallway outside office where students can sit and wait 
 * When no students TA sits in chair and takes a nap (wait student)
 *  1) If student comes when TA sleeps they wake the TA (signal TA)
 *  2) If student comes and finds TA helping another student waits in chair  (Claim chair)
 *   2.1) If No chairs available, student will come back at later time 
 * Use POSIX threads, mutex locks, and semaphores 
 */

/** 
 * Begin by creating n students (threads) 
 * TA will be 1 separate thread
 * 
 * Execution: 
 *   Student threads alternate b/w programming and asking for help 
 *     - Sleeps and asks for help
 *     - If TA available they obtain help 
 *     - If TA available not available: Sit in chair, or get back to programming if no chairs
 *        wait(chair) 
 *     - if(ta available) { get help } 
 *     - else if (chair available) { sit in chair }  
 *     - else (sleep)
 *   If TA sleeping when student arrives they wake the TA up (signal(TA)) 
 *   When TA finishes they must check hall to see if student waiting (check chair semaphore) 
 *     - Else go to nap
 *  
 *  To model students programming and TA helping, have them sleep for random period of time
 *  
 *   Chair semaphore (3) 
 *   TA Semaphore (1)
 */


// The TA’s office is rather
// small and has room for only one desk with a chair and computer. There are three chairs in the
// hallway outside the office where students can sit and wait if the TA is currently helping another
// student. When there are no students who need help during office hours, the TA sits at the desk and
// takes a nap. If a student arrives during office hours and finds the TA sleeping, the student must
// awaken the TA to ask for help. If a student arrives and finds the TA currently helping another
// student, the student sits on one of the chairs in the hallway and waits. If no chairs are available,
// the student will come back at a later time.
// Using POSIX threads, mutex locks, and semaphores, implement a solution that
// coordinates the activities of the TA and the students. Details for this assignment are
// provided below.


// pthread_mutex_lock(&lock);

// // Critical section: access shared data safely

// pthread_mutex_unlock(&lock);


/* barber shop monitor */
/* Based off of my 2SD3 lecture notes */

int barber = 0, chair = 0, exit = 0;
int chair = 3; 
int TA = 0; 
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

// Ask TA for help
//  *     - Sleeps and asks for help
//  *     - If TA available they obtain help 
//  *     - If TA not available: Sit in chair, or get back to programming if no chairs
//  *        wait(chair) 
//  *     - if(ta available) { get help } 
//  *     - else if (chair available) { sit in chair }  
//  *     - else (sleep)
//  *   If TA sleeping when student arrives they wake the TA up (signal(TA)) 
//  *   When TA finishes they must check hall to see if student waiting (check chair semaphore) 
//  *     - Else go to nap


// Request TA for help 
void *getHelp(int stu){ 
    pthread_mutex_lock(&mutex);                            
        printf("%d Waiting for TA\n",stu);

        if(ta_aval > 0){
            if(chair_aval > 0){
                while(TA_Aval == 0) sem_wait(&chair);
            }
        }else{
            
        }

        while (barber == 0){
            pthread_mutex_unlock(&mutex);  
            sem_wait(&barberAvailable);
            pthread_mutex_lock(&mutex);  
        } //pthread_cond_wait(&barberAvailable, &mutex);
        barber -= 1;
        chair += 1; pthread_cond_signal(&chairOccupied);
        printf("%d waiting for exit door\n", cust);
        while (exit == 0) pthread_cond_wait(&exitOpen, &mutex);
        exit -= 1; pthread_cond_signal(&customerLeft);
    pthread_mutex_unlock(&mutex);        
}



void *TA_func(){ 
    while(true){ 
        getNextStudent();  /* wait for a student to sit in the chair */
        printf("Helping student\n");
        finishedHelping();      /* allow the student to leave; returns after the customer left */
        sleep(1);
    }
}

void *student(void *t){ 
    int stu = (int) t;

    while(true){ 
        // int s = rand() % 4;
        // Sleep random amount 0 - 4 seconds
        getHelp(stu);
        sleep((rand() % 4));
    }
}

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
    int numcust = atoi(argv[1]);
    srand(42);

    pthread_t students[n];
    pthread_t ta_worker;
    // pthread_t workers[NUM_THREADS]; /* Store thread identifiers*/
    // pthread_attr_t students_attr[NUM_DEPOSIT];
    // pthread_attr_t ta_attr;

    // Create n student threads
    for(int i = 0; i < n; i ++){ 
        pthread_create(&students[i],NULL,student, (void*) i);
    }
    // Create TA thread
    pthread_create(&ta_worker,NULL,TA_func,NULL);

    for(int i = 0; i < n; i++){ 
        ptread_join(students[i],NULL);
    }
    pthread_join(ta_worker,NULL);
}




