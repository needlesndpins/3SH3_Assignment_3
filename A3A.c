#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h> 
#include <semaphore.h>
// #include <fcntl.h>
#include <stdio.h>



#define n 10
// Use 4 students
// 1 with TA, 3 sitting in chairs
#define MAX_WAITING 4 // Max amount of students waiting is 4 

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER; // Mutex

bool ta_sleep = true; // Store if TA is sleeping

int wait_queue[MAX_WAITING];   // Queue of student semaphore IDs 
int head = 0;                  // Head of queue 
int tail = 0; 
int waiting_students = 0;      // Store number of students waiting for TA_help

sem_t student_sem[n];         // Array of student semaphores
sem_t student_left;           // Semaphore to signal if student has left
sem_t alarm_clock;            // Alarm clock semaphore to wake up TA


void *student(void *arg); 
void *ta();// Tail of queue


// Student will call this function to get help
void getHelp(int id) {

    pthread_mutex_lock(&lock);

    if (waiting_students < MAX_WAITING) {

        waiting_students++;

        if (ta_sleep) {
            ta_sleep = false;
            sem_post(&alarm_clock);
            printf("Student %d woke up TA\n", id);
        }

        // Throw every student inside queue
        wait_queue[tail] = id;
        tail = (tail + 1) % MAX_WAITING;

        // Avoid student waiting at door from siting in chair
        if(waiting_students != 1){ 
            printf("Student %d is sitting in chair.",id);
        }

        pthread_mutex_unlock(&lock);

        // Have student wait on their semaphore for TA to help them
        sem_wait(&student_sem[id]);

        printf("Student %d is getting help\n", id);
        sleep(rand() % 2);
        printf("Student %d has left\n", id);

        // Say student has left
        sem_post(&student_left);

        // MUST DECREMENT HERE, OR ELSE HAVE 4 PEOPLE IN QUEUE
        pthread_mutex_lock(&lock); 
        waiting_students --; 
        pthread_mutex_unlock(&lock);

    } else {

        // printf("No chairs, student %d leaves\n", id);
        pthread_mutex_unlock(&lock);
    }


}

void getNextStudent() {

    // Wait for student to wake up TA
    sem_wait(&alarm_clock);

    // Continuously loop until if condition met 
    // Explicitly state break condition
    while (true) {

        pthread_mutex_lock(&lock);

        // Keep going till no more students
        if (waiting_students == 0) {
            ta_sleep = true;
            printf("TA is sleeping\n");
            pthread_mutex_unlock(&lock);
            break;
        }

        // Get next student from waiting queue
        int id = wait_queue[head];
        head = (head + 1) % MAX_WAITING;

        // waiting_students--;

        pthread_mutex_unlock(&lock);

        // Notify student TA is ready
        sem_post(&student_sem[id]);

        // Wait for student to leave before getting next
        sem_wait(&student_left);
    }

}


void *student(void *arg){ 
    int id = *(int *)arg;
    while(true){ 
        // Sleep random time 0-4s
        sleep(rand() % 4); 
        getHelp(id);
        sleep(rand() % 8);
    }


}

void *ta(){ 
    while(true){ 
        getNextStudent();

    }
}



int main() {
    srand(42);

    pthread_t students[n];
    pthread_t ta_worker;

    // Semaphore of students, they wait on it
    // Each one goes in the wait_queue, students will wait on their semaphore 
    // TA will post the one ready for help
    for(int i = 0; i < n; i++){ 
        sem_init(&student_sem[i],0,0);
    }

    // Create alarm_clock and student left semaphore
    sem_init(&alarm_clock,0,0);
    sem_init(&student_left,0,0);

    // Array of student IDs
    // Avoid null pointer casting errors
    int student_ids[n];

    // Create n student threads
    for(int i = 0; i < n; i ++){ 
        student_ids[i] = i;  // Store the ID 
        pthread_create(&students[i],NULL,student,&student_ids[i]);
    }
    // Create TA thread
    pthread_create(&ta_worker,NULL,ta,NULL);

    for(int i = 0; i < n; i++){ 
        pthread_join(students[i],NULL);
    }
    pthread_join(ta_worker,NULL);
}
