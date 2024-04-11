#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <semaphore.h>

using namespace std;

// semafory reprezentuja widdelce
const int num_filosophers = 5;
sem_t forks[num_filosophers];

// symulacja filozofa
void philosopher(int id) {
    //this_thread::sleep_for(chrono::milliseconds(100));
    while (true) {

        cout << "Filozof " << id << " rozmysla." << endl;
        this_thread::sleep_for(chrono::milliseconds(1000)); // symulacja rozmyslania

        cout << "Filozof " << id << " jest glodny." << endl;
        
        // najpierw podnosi widelec z nizszym numerem lewy
        sem_wait(&forks[id]);
        cout << "Filozof " << id << " podniosl lewy widelec nr " << id << endl;

        // nastepnie z numerem wyzszym prawy
        sem_wait(&forks[(id + 1) % num_filosophers]);
        cout << "Filozof " << id << " podniosl prawy widelec nr " << (id + 1) % num_filosophers << endl;

        cout << "Filozof " << id << " je." << endl;
        this_thread::sleep_for(chrono::milliseconds(2000)); // symulacja jedzienia

        // oddaje widelce
        cout << "Filozof " << id << " odlozyl prawy widelec nr " << (id + 1) % num_filosophers << endl;
        sem_post(&forks[(id + 1) % num_filosophers]);
        cout << "Filozof " << id << " odlozyl lewy widelec nr " << id << endl;
        sem_post(&forks[id]);
        //cout << "Filozof " << id << " odlozyl prawy widelec nr " << (id + 1) % num_filosophers << endl;
        //sem_post(&forks[(id + 1) % num_filosophers]);
    }
}

int main() {

    const int num_filosophers = 5;

    // inicjalizacja semaforow
    for (int i = 0; i < num_filosophers; ++i) {
        sem_init(&forks[i], 0, 1); 
    }

    // watki filozofow
    thread filozofowie[num_filosophers];
    for (int i = 0; i < num_filosophers; ++i) {
    }

    for (int i = 0; i < num_filosophers; ++i) {
        filozofowie[i].join();
    }

    for (int i = 0; i < num_filosophers; ++i) {
        sem_destroy(&forks[i]);
    }

    return 0;
}
