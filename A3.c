
#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h> 
#include <semaphore.h>
#include <stdint.h>


#define n 10

static pthread_mutex_t printf_mutex = PTHREAD_MUTEX_INITIALIZER;

//atmoic print
#define printf(...) do { \
    pthread_mutex_lock(&printf_mutex); \
    fprintf(stdout,__VA_ARGS__); \
    pthread_mutex_unlock(&printf_mutex); \
} while(0)



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

#define NUM_CHAIRS 3

// Synchronization primitives
pthread_mutex_t mutex;
sem_t ta_sem;
sem_t student_sem[NUM_CHAIRS];
sem_t finished;

// Shared state variables
int waiting_students = 0;
int next_seat_in = 0;
int next_seat_out = 0;


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
void getHelp(int stu){ 
    pthread_mutex_lock(&mutex);                            
        if (waiting_students < NUM_CHAIRS) {
            //claim a chair
            int chair_index = next_seat_in;
            next_seat_in = (next_seat_in + 1) % NUM_CHAIRS;
            waiting_students++;
            
            if(waiting_students != 1) printf("%d sat in a chair, Waiting students: %d\n", stu, waiting_students); //sit in chair

            //if the TA was asleep, wake the TA
            if (waiting_students == 1) {
                 printf("%d wakes TA\n", stu); 
                sem_post(&ta_sem);
            }
            pthread_mutex_unlock(&mutex);

            //wait for the TA to signal this specific chair
            sem_wait(&student_sem[chair_index]);

            //getting help
            printf("%d getting help from TA\n", stu);
            sleep(rand() % 2 + 1);
            printf("%d finished getting help\n", stu);

            sem_post(&finished); //signal the TA that the student has finished getting help
        } else {
            //chairs are full
            pthread_mutex_unlock(&mutex);
            printf("%d chairs are full. returning to programming\n", stu);
    } 
}

void getNextStudent(){
    sem_wait(&ta_sem); //wait for a student to arrive and wake up

    while (true) {
        pthread_mutex_lock(&mutex);
        if (waiting_students == 0) {
            pthread_mutex_unlock(&mutex);
            break; //return to napping
        }

        //next student
        int chair_index = next_seat_out;
        next_seat_out = (next_seat_out + 1) % NUM_CHAIRS;
        waiting_students--;
        
        printf("Helping a student\n");
        pthread_mutex_unlock(&mutex);

        //signal the next student that the TA is ready
        sem_post(&student_sem[chair_index]);

        sem_wait(&finished); //wait for student to finish before getting next
    }
    printf("TA is napping in the office\n");
}


void *TA_func(){ 
    while(true){ 
        getNextStudent();  // wait for a student to sit in the chair
        sleep(1);
    }
}

void *student(void *t){ 
    int stu = (uint64_t) t; //student id

    while(true){ 
        // int s = rand() % 4;
        // Sleep random amount 0 - 4 seconds
        sleep((rand() % 4));
        getHelp(stu);
        sleep((rand() % 4));
    }
}



int main(int argc, char *argv[]) {
    srand(42);

    pthread_mutex_init(&mutex, NULL);
    sem_init(&ta_sem, 0, 0);
    sem_init(&finished, 0, 0);
    for (int i = 0; i < NUM_CHAIRS; i++) {
        sem_init(&student_sem[i], 0, 0);
    }

    pthread_t students[n];
    pthread_t ta_worker;
    // pthread_t workers[NUM_THREADS]; /* Store thread identifiers*/
    // pthread_attr_t students_attr[NUM_DEPOSIT];
    // pthread_attr_t ta_attr;

    // Create n student threads
    for(uint64_t i = 0; i < n; i ++){ 
        pthread_create(&students[i],NULL,student, (void*) i);
    }
    // Create TA thread
    pthread_create(&ta_worker,NULL,TA_func,NULL);

    for(int i = 0; i < n; i++){ 
        pthread_join(students[i],NULL);
    }
    pthread_join(ta_worker,NULL);
}




