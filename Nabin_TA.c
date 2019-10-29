//Nabin

//Question : Make sure to use mutex locks or semaphores, or other tools, to synchronize the tasks correctly.
//Answer:  Yes I have used both mutex locks and semaphores in my code. They are commented as defined.


//Question : How many threads/processes are there?
//Answer : There are two processes : One for Teaching Assistant and One for the Students.
//			The number of Student Threads will be the number of students for the program . The number of the threads for Teaching Assistant is One.


//Question : What does each thread simulate? The simulation needs to reflect the real case of the problem.
//Answer : Each Student Thread simulate each students, either they are waiting in the chair or programming. 
//			TA thread is responsible for taking nap and assisting the students.


//Question : Will each thread be blocked at some time of simulation? If yes, explain the circumstances.
//Answer : Yes, the threads will be blocked for various purposes. 
// 			1.When a student goes to TA's chair for the help, mutex locks and updates the chair details before any other thread access the chair count value.Once, the seats are calculated , the mutex will unlock it.
//			2. When all the chairs are empty, the sem_post will unlock the TA_Nap semaphore. Similarly,sem_wait will perform the semaphore operation on the Student_Sem so that only one student will go inside the TA room for assistance.


// Question : When will a blocked thread be waked up, and by whom?
//Answer : 	Blocked thread will be waked up by mutex locks and semaphore.When all the chairs are empty, the sem_post will unlock the TA_Nap semaphore. Also,When a student goes to TA's chair for the help, mutex locks and updates the chair details before any other thread access the chair count value.


//Question : How many mutex locks/semaphores are there in your code, what is the purpose for having each?
// Answer : I have 1 mutex lock and 3 semaphores in my code. 
//			1 mutex lock is responsible for managing and calculating the Chairs Availibility. If the chair is not available, the student goes back to programming.
//			3 Semaphores : They are for TA taking nap , for students and for chairs. 
//			TA_Nap will be unlocked by sem_post semaphore if the chairs are empty. 
//			Student_Sem will be locked by sem_wait while another student is getting help from the TA.
//			Chairs_Sem will be locked by sem_wait while student leaves the chair.
//			ChairAvailable will be locked and unlocked by mutex to update the chair count.


//Question : How does your program terminate?
//Answer : I have used ALARMhandler to terminate my program. After 15 seconds , the program will exit and the TA will go home for the day. 


//Question : Introduce randomness in code. The thread/process needs to sleep for a random period of time to simulate the execution of any critical section and reminder section by using rand(), sleep() functions. 
// Answer : Yes, I have introduced rand() and sleep() function in my program. rand() will return a random number and sleep() makes the calling thread to sleep for specified number of seconds.




#include <pthread.h>	//Create POSIX threads.
#include <signal.h>		// To terminate the program.
#include <time.h>			//Wait for a random time.
#include <unistd.h>			//Thread calls sleep for specified number of seconds.
#include <semaphore.h>		//To create semaphores
#include <stdlib.h>			
#include <stdio.h>			//Input Output

pthread_t *Students;	 // Process thread will be the number of students available.
pthread_t TA;			// Process thread will be 1.
#define sleep_max 5	// Defining the number of sleeping time.
#define seats 3			// the number of chairs available in front of TA's room is 3.
int Chairs = 0;		
int Position = 0;


//Declare different Semaphores and Mutex Lock
sem_t TA_Nap;
sem_t Student_Sem;
sem_t Chairs_Sem[seats];
pthread_mutex_t ChairAvailable;

//Functions
void *Student_Status(void *threadID);
void *TA_Status();


static void ALARMhandler(int remind)
{
	printf(" \n ---------------- The Teaching Assistant Goes Home ------------ \n BYE !! \n See You Tomorrow \n");
	exit(EXIT_SUCCESS);
}

void *Student_Status(void *threadID)
{
	int Time_Spent_Programming;
	while(1)
	{
		printf("\t Student %ld is busy in his programming assignment.\n",(long)threadID);
		Time_Spent_Programming = rand() % sleep_max ; //takes random number between 0 to 2.
		sleep(Time_Spent_Programming);	// seconds student takes while programming.
		printf("\t The Student %ld wants to meet TA for assignment help. \n", (long)threadID);
		pthread_mutex_lock(&ChairAvailable); //lock so that only one thread can work.
		int count = Chairs;
		pthread_mutex_unlock(&ChairAvailable); //unlock the mutex now.
		if(count < seats )	// There are three available chairs and student want to sit.
		{
			if(count == 0)
				sem_post(&TA_Nap); //unlocking the semaphore TA_Nap. so, when all chairs are empty , the TA will take a nap.
			else
				printf("\t The Student %ld occupied the chair and waiting for Teaching Assistant. \n",(long)threadID);
			
			//lock
			pthread_mutex_lock(&ChairAvailable); //lock so that only one thread can work.
			int index = (Position + Chairs) % seats;
			Chairs++;
			printf("\t Student occupied the chair.\n \t Available Chairs = %d\n",3 - Chairs);
			pthread_mutex_unlock(&ChairAvailable); // unlocking mutex.
			//unlock
			sem_wait(&Chairs_Sem[index]);	//Students leave the chair.lock the operation on the semaphore.
			printf(" \t Teaching Assistant is helping the student %ld .\n",(long)threadID);
			sem_wait(&Student_Sem);	//Student waits.
			printf("Student %ld left Teaching Assistant room. \n",(long)threadID);
		}
		else
			printf(" There are no Chairs Available.So,\n \t The Student %ld will return at another time. \n",(long)threadID);	//There is no chair for student to sit. So, the student goes to programming again.
	}
}


void *TA_Status()
{
	while(1)
	{
		sem_wait(&TA_Nap);
		printf("------- The Sleeping Teaching Assisant is now awaked by the Student. -------\n");
				
		while(1)
		{
			//lock
			pthread_mutex_lock(&ChairAvailable);
			if(Chairs == 0)
			{
				//if all the chairs are empty, we break the loop.
				pthread_mutex_unlock(&ChairAvailable);
				break;
			}
			//if there is student waiting for TA.
			sem_post(&Chairs_Sem[Position]);
			Chairs--;
			printf(" The Student left the chair and went to Teaching Assistant Office.\n \t Available Chairs = %d \n", 3 - Chairs);
			Position = (Position + 1) % seats;
			pthread_mutex_unlock(&ChairAvailable);
			//unlock
			printf(" Teaching Assisant is now busy helping Student.\n");
			sleep(sleep_max);
			sem_post(&Student_Sem);
			usleep(200);
		}
	}
}

int main(int argc, char* argv[])
{
	int total_students;	//number of students are entered by users. Default we have defined as 5 users.
	int n;
	srand(time(NULL));
	
	//Initializing Mutex Lock and Semaphores.
	sem_init(&TA_Nap, 0, 0);
	sem_init(&Student_Sem, 0, 0);
	for(n = 0 ; n < seats; ++n)
		sem_init(&Chairs_Sem[n], 0, 0);
	pthread_mutex_init(&ChairAvailable, NULL);
	if(argc < 2)
	{
		printf(" Enter the Number of Students who are Programming today.:\n");
		scanf("%d", & total_students);

	}
	signal(SIGALRM, ALARMhandler); 
	alarm(15);
	//Memory for students
	Students = (pthread_t*) malloc(sizeof(pthread_t)*total_students);
	//Creating Teaching Assistant thread and N student threads.
	pthread_create(&TA, NULL, TA_Status, NULL);
	for(n = 0; n < total_students; n++)
		pthread_create(&Students[n],NULL, Student_Status,(void*) (long)n);
	//Waiting for Teaching Assisant thread and N Student threads.
	pthread_join(TA, NULL);
	for(n = 0; n < total_students; n++)
		pthread_join(Students[n],NULL);
	//Free Allocated Memory.
	free(Students);
	return 0;	
}
		
		