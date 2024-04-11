#include <iostream>
#include <cmath>
#include <thread>
#include <mutex>
#include <iomanip>

using namespace std;

// Mutex synchronizacji wspoldzielonej sumy
mutex sum_mutex;

// funkcja do obliczania fragmentu sumy
void partial_sum(int start, int stop, double &result) {
    double partial = 0.0;
    for (int i = start; i <= stop; ++i) {
        partial += 1.0 / i;
    }

    // Lock muteksa przed dodaniem
    sum_mutex.lock();
    result += partial;
    // unlock po zmianie
    sum_mutex.unlock();
}

int main() {
    int n; // liczba elementów sumy  
    int p; // kazdy z p procesów liczy pewien fragment sumy
    cout << "Podaj liczbe elementow sumy: ";
    cin >> n;
    cout << "Podaj liczbe procesow: ";
    cin >> p;

    // frahment dla kazdego procesu
    int f_sum = n / p;
    //cout << f_sum << endl;
    thread proc[p];
    double euler_gamma = 0;

    // watki do obliczenia fragmetow sumy
    for (int i = 0; i < p; ++i) {
        int start = i * f_sum + 1;
        int stop = (i + 1) * f_sum;
        if (i == p - 1) {
            // Adjust the end for the last process to ensure all elements are covered
            stop = n;
        }
        proc[i] = thread(partial_sum, start, stop, ref(euler_gamma));
    }


    for (int i = 0; i < p; ++i) {
        proc[i].join();
    }

    euler_gamma -= log(n);

    cout << "Stala eulera dla n=" << n << " wynosi: "<< setprecision(4) << euler_gamma << endl;

    return 0;
}
