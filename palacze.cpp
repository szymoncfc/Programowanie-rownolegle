#include <iostream>
#include <thread>
#include <mutex>
#include <semaphore.h>

// semafory
sem_t ubijacz_semafor;
sem_t zapalki_semafor;
sem_t screen_semafor;


const int l = 1; // ilosc ubijaczy
const int m = 1; // ilosc pudelek zapalek


void mess(int ids, const std::string& message) {
    sem_wait(&screen_semafor); // Czekamy na dostęp do ekranu
    std::cout << "Palacz " << ids <<  message << std::endl;
    sem_post(&screen_semafor); // Zwalniamy dostęp do ekranu
}

// funkcja palacza
void smoker(int id) {
    while (true) {
        // czekanie na zasoby

        mess(id, " czeka na ubijacz");
        sem_wait(&ubijacz_semafor);
        mess(id, " ubija");
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        sem_post(&ubijacz_semafor);
        mess(id, " zwraca ubijacz");
        

        mess(id, " czeka na zapalki");
        sem_wait(&zapalki_semafor);
    
        mess(id, " zapala fajke");
        std::this_thread::sleep_for(std::chrono::milliseconds(200));        
        sem_post(&zapalki_semafor);
        mess(id, " zwraca zapalki");

        // symulacja palenia
        mess(id, " pali fajke");
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

int main() {
    // inicjalizacja semaforow
    sem_init(&ubijacz_semafor, 0, l); 
    sem_init(&zapalki_semafor, 0, m); 
    sem_init(&screen_semafor, 0, 1); // do wyswietlania komunikatow

    // watki palaczy
    const int k = 3;
    std::thread palacze[k];
    for (int i = 0; i < k; ++i) {
        palacze[i] = std::thread(smoker, i+1);
    }

    for (auto& t : palacze) {
        t.join();
    }

    sem_destroy(&ubijacz_semafor);
    sem_destroy(&zapalki_semafor);
    sem_destroy(&screen_semafor);

    return 0;
}