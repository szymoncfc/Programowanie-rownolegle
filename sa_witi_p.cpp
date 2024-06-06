#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <limits>
#include <random>

using namespace std;

class Zadanie {
public:
    int p;  // czas wykonania
    int w;  // waga zadania
    int d;  // deadline pożądany termin zakończenia
    int nr; // nr zadania, do wyświetlenia permutacji

    Zadanie(int pp, int ww, int dd, int n) : p(pp), w(ww), d(dd), nr(n) {}
};

int witi(const vector<Zadanie> &v) {
    int ti = 0; // spoznienie
    int c = 0;
    int witi = 0;

    for (const auto& zad : v) {
        c += zad.p;
        ti = max(0, c - zad.d);
        witi += ti * zad.w;
    }

    return witi;
}

vector<Zadanie> swap_elements(vector<Zadanie>& vec, mt19937& rng) {
    if (vec.size() < 2) {
        cerr << "Wektor musi zawierać przynajmniej dwa elementy, aby dokonać zamiany." << endl;
        return vec;
    }

    uniform_int_distribution<int> dist(0, vec.size() - 1);
    int index1 = dist(rng);
    int index2 = dist(rng);

    while (index1 == index2) {
        index2 = dist(rng);
    }

    swap(vec[index1], vec[index2]);

    return vec;
}

void sa_witi(const vector<Zadanie>& zadania, double init_temperature, double cooling_rate, int max_iter, 
                              int& global_best_delay, vector<Zadanie>& global_best_perm, 
                              mutex& global_mutex, condition_variable& cv, int thread_id) 
    {

    vector<Zadanie> best_permutation = zadania;
    int best_delay = witi(best_permutation);

    vector<Zadanie> current_permutation = best_permutation;
    int current_delay = best_delay;
    double current_temperature = init_temperature;

    random_device rd;
    mt19937 rng(rd() + thread_id); // Różne seedy dla różnych wątków
    uniform_real_distribution<double> dist(0.0, 1.0);

    for (int iter = 0; iter < max_iter; ++iter) {
        vector<Zadanie> new_permutation = current_permutation;
        new_permutation = swap_elements(new_permutation, rng);

        int new_delay = witi(new_permutation);
        int delay_difference = new_delay - current_delay;

        if (delay_difference < 0 || exp(-delay_difference / current_temperature) > dist(rng)) {
        //if (delayDifference < 0 || exp(-delayDifference / currentTemperature) > ((double)rand() / RAND_MAX)) {
            current_permutation = new_permutation;
            current_delay = new_delay;
        }

        if (current_delay < best_delay) {
            best_permutation = current_permutation;
            best_delay = current_delay;
        }

        current_temperature *= cooling_rate;

        if (iter % (max_iter / 10) == 0) { // Synchronizacja co pewien czas
            //unique_lock<mutex> lock(globalMutex);
            global_mutex.lock();
            if (best_delay < global_best_delay) {
                global_best_delay = best_delay;
                global_best_perm = best_permutation;
            } else {
                best_permutation = global_best_perm;
                best_delay = global_best_delay;
            }
            cv.notify_all();
            global_mutex.unlock();
        }
    }
}

vector<Zadanie> sa_witi_parallel(const vector<Zadanie>& zadania, double init_temperature, double cooling_rate, int max_iterations, int num_threads) {
    // zmienne globalne
    int global_best_delay = numeric_limits<int>::max();
    vector<Zadanie> global_best_perm;
    mutex global_mutex;
    condition_variable cv;

    vector<thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.push_back(thread(sa_witi, ref(zadania), init_temperature, cooling_rate, max_iterations, 
                                 ref(global_best_delay), ref(global_best_perm), ref(global_mutex), ref(cv), i));
    }

    for (auto& t : threads) {
        t.join();
    }

    return global_best_perm;
}

int main() {
    vector<Zadanie> zadania;
    int N, var1, var2, var3;
    ifstream plik("data2.txt");

    plik >> N;
    for (int j = 0; j < N; ++j) {
        plik >> var1 >> var2 >> var3;
        zadania.emplace_back(var1, var2, var3, j + 1);
    }

    double temperature = 1000;
    double cooling_rate = 0.98;
    int max_iterations = 500;
    int num_threads = 2; // liczba wątków

    srand(time(nullptr));

    auto started = chrono::high_resolution_clock::now();
    vector<Zadanie> optimalPermutation = sa_witi_parallel(zadania, temperature, cooling_rate, max_iterations, num_threads);
    auto done = chrono::high_resolution_clock::now();
    double tot_time = chrono::duration_cast<chrono::milliseconds>(done - started).count();

    std::cout << "Czas: " << tot_time << " ms" << endl;
    std::cout << "Kolejnosc wykonywania zadan: ";
    for (const auto& zadanie : optimalPermutation) {
        std::cout << zadanie.nr << " ";
    }
    std::cout << endl;

    std::cout << "Minimalna suma wazonych opoznien wiTi: " << witi(optimalPermutation) << endl;

    return 0;
}
