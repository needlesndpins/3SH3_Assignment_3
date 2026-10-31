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

sem_t *student_sem[n];         // Array of student semaphores
sem_t *student_left;           // Semaphore to signal if student has left
sem_t *alarm_clock;            // Alarm clock semaphore to wake up TA


void *student(void *arg); 
void *ta();// Tail of queue

void PrintQueue() {

    // pthread_mutex_lock(&lock);

    printf("Waiting Queue: ");

    for (int i = 0; i < waiting_students; i++) {
        int index = (head + i) % MAX_WAITING;
        printf("%d ", wait_queue[index]);
    }

    printf("\n");

    // pthread_mutex_unlock(&lock);
}



// Student will call this function to get help
void getHelp(int id) {

    pthread_mutex_lock(&lock);

    if (waiting_students < MAX_WAITING) {

        waiting_students++;

        if (ta_sleep) {
            ta_sleep = false;
            sem_post(alarm_clock);
            printf("Student %d woke up TA\n", id);
        }

        // Throw every student inside queue
        wait_queue[tail] = id;
        tail = (tail + 1) % MAX_WAITING;

        // Avoid student waiting at door from siting in chair
        if(waiting_students != 1){ 
            printf("Student %d is sitting in chair. ",id);
            PrintQueue();
        }

        pthread_mutex_unlock(&lock);

        // Have student wait on their semaphore for TA to help them
        sem_wait(student_sem[id]);

        printf("Student %d is getting help\n", id);
        sleep(rand() % 2);
        printf("Student %d has left\n", id);

        // Say student has left
        sem_post(student_left);

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
    sem_wait(alarm_clock);

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

        // waiting_students--;   // Mark student as having left queue 

        pthread_mutex_unlock(&lock);

        // Notify student TA is ready
        sem_post(student_sem[id]);

        // Wait for student to leave before getting next
        sem_wait(student_left);
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


void clearSemaphores() {

    for (int i = 0; i < n; i++) {
        char name[32];

        sprintf(name, "/student_sem%d", i);
        sem_unlink(name);
    }

    sem_unlink("/alarm_clock");
    sem_unlink("/student_left");
    sem_unlink("/ta_help");
}


int main(int argc, char *argv[]) {
    // int numcust = atoi(argv[1]);
    clearSemaphores();
    srand(42);

    pthread_t students[n];
    pthread_t ta_worker;


    // wait_queue[0] = sem_open("/wait_queue0", O_CREAT, 0600, 1);
    // wait_queue[1] = sem_open("/wait_queue1", O_CREAT, 0600, 1);
    // wait_queue[2] = sem_open("/wait_queue2", O_CREAT, 0600, 1);
    // wait_queue[3] = sem_open("/wait_queue3", O_CREAT, 0600, 1);


    // Semaphore of students, they wait on it
    // Each one goes in the wait_queue, students will wait on their semaphore 
    // TA will post the one ready for help
    student_sem[0] = sem_open("/student_sem0", O_CREAT, 0600, 0);
    student_sem[1] = sem_open("/student_sem1", O_CREAT, 0600, 0);
    student_sem[2] = sem_open("/student_sem2", O_CREAT, 0600, 0);
    student_sem[3] = sem_open("/student_sem3", O_CREAT, 0600, 0);
    student_sem[4] = sem_open("/student_sem4", O_CREAT, 0600, 0);
    student_sem[5] = sem_open("/student_sem5", O_CREAT, 0600, 0);
    student_sem[6] = sem_open("/student_sem6", O_CREAT, 0600, 0);
    student_sem[7] = sem_open("/student_sem7", O_CREAT, 0600, 0);
    student_sem[8] = sem_open("/student_sem8", O_CREAT, 0600, 0);
    student_sem[9] = sem_open("/student_sem9", O_CREAT, 0600, 0);

    // Student has arrived
    alarm_clock = sem_open("/alarm_clock", O_CREAT, 0600, 0);   // Used to signal TA to WAKE UP
    student_left = sem_open("/student_left", O_CREAT, 0600, 0);

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
