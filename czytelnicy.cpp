#include <iostream>
#include <thread>
#include <semaphore.h>

using namespace std;


int data = 0;
int read_count = 4;
int waiting_writers = 0;	
int working_writers = 0;
int waiting_readers = 0;
int working_readers = 0;

sem_t writers_sem;
sem_t readers_sem;
sem_t data_sem;
sem_t writers_data_sem;


// Reader
void reader(int id) {

    this_thread::sleep_for(chrono::milliseconds(500));
    while (true) {

        sem_wait(&data_sem);
        waiting_readers++;
        if (waiting_writers == 0) {
            // jezeli nie ma pisarzy wpuszczamy czytlnikow
            working_readers++;	
			sem_post(&data_sem); //dostep
        }
        else{
			sem_post(&data_sem); //dostep
			sem_wait(&readers_sem);	//czytelnik czeka na dostep
		}

        // Read data
        cout << "Czytelnik " << id << " odczytal: " << data << endl;
        read_count++;
        this_thread::sleep_for(chrono::milliseconds(1000));
        sem_wait(&data_sem);
		working_readers--; 
		waiting_readers--;

		if(working_readers == 0 && read_count >=3){	//jeli nie ma juz czytelników i wystapily 3 odczyty mozemy wpuszczac pisarzy

			working_writers++;
			sem_post(&writers_sem);

		}
        else {
            working_readers++;
			sem_post(&readers_sem);	//wpuszczamy czytelników    
        }

		
		sem_post(&data_sem);
    }
}

// Writer 
void writer(int id) {
    while (true) {

        sem_wait(&data_sem); //blok
		waiting_writers++;

        if(waiting_readers == 0){ 
            //jezeli nie ma czekajacych czytelników wpuszczamy pisarzy
			working_writers++;
			sem_post(&data_sem); //dostep
		}
		
		else{
			sem_post(&data_sem);
			sem_wait(&writers_sem);	   //jezeli sa czytelnicy czekamy na dostep
		}

        sem_wait(&writers_data_sem);	//pisarz wchodzi blok dla innych pisarzy
        srand(time(0));
        data = rand() % 100;
        cout << "Pisarz " << id << " zmodyfikowal dane: " << data << endl;
        read_count =0; 
        this_thread::sleep_for(chrono::milliseconds(1000));
        sem_post(&writers_data_sem); 	//koniec pisania
        sem_wait(&data_sem);
        working_writers--; 
        waiting_writers--;
            

        if(working_writers == 0){	//jezeli wszyscy pisarze wyszli wpusczczamy czytelników

			working_readers++;
			sem_post(&readers_sem);	//wpuszczamy czytelników

		} 
		
		sem_post(&data_sem);

    }
}

int main() {

    
    sem_init(&writers_sem, 0, 0);		//semafory do zliczania
	sem_init(&readers_sem,0,0);        	
	sem_init(&data_sem,0, 1);		
	sem_init(&writers_data_sem,0,1);   //dostep do pisania

    const int num_readers = 2;
    const int num_writers = 2;

    thread readerst[num_readers];
    thread writerst[num_writers];

    // watki czytelnikow
    for (int i = 0; i < num_readers; ++i) {
        readerst[i] = thread(reader, i + 1);
    }

    // watki pisarzy
    for (int i = 0; i < num_writers; ++i) {
        writerst[i] = thread(writer, i + 1);
    }

    for (int i = 0; i < num_readers; ++i) {
        readerst[i].join();
    }
    for (int i = 0; i < num_writers; ++i) {
        writerst[i].join();
    }


    sem_destroy(&writers_sem);
    sem_destroy(&readers_sem);
    sem_destroy(&data_sem);
    sem_destroy(&writers_data_sem);

    return 0;
}