#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <limits.h>
#include <fstream>
#include <chrono>
#include <string>
#include <random>
#include <ctime>
#include <omp.h>

using namespace std;

class Job {
public:
    int nr;
    vector<int> machine;

    Job(int d) : nr(d) {}
};

int cmax(vector<Job> vec) 
{
    int cmax = 0;
    int *act_time = new int[vec[0].machine.size()-1];

    for(int n=0;n<vec.size();n++)
    {
        for(int m=0;m<vec[0].machine.size();m++)
        {
            if(n==0)
            {
                if(m==0)
                {
                    act_time[0]=vec[n].machine[m];
                }
                else
                {
                    act_time[m]=vec[n].machine[m]+act_time[m-1];
                }

            }
            else
            {
                if(m==0)
                {
                    act_time[0] = max(0,act_time[0]) + vec[n].machine[m];
                }
                else
                {
                    act_time[m] = max(act_time[m-1],act_time[m]) + vec[n].machine[m];
                }

            }
        }

    }

    cmax = act_time[vec[0].machine.size()-1];
    delete act_time;
    return cmax;    


}

void wyswietl(vector<Job>& v) {
    std::cout << "Uszeregowanie: ";
    for (auto& z : v) {
        std::cout << z.nr << " ";
    }
    std::cout << endl;
}

vector<Job> random_sequence(vector<Job> vec) {
    random_device rd;
    mt19937 g(rd());
    shuffle(vec.begin(), vec.end(), g);
    return vec;
}

vector<Job> cross(vector<Job> parent1, vector<Job> parent2) {
    int size = parent1.size();
    vector<Job> child(size, Job(0));
    vector<bool> chosen(size + 1, false);
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, size - 1);

    int cross_point = dis(gen);

    for (int i = 0; i <= cross_point; i++) {
        child[i] = parent1[i];
        chosen[parent1[i].nr] = true;
    }

    int index = cross_point + 1;
    for (int i = 0; i < size; i++) {
        if (!chosen[parent2[i].nr]) {
            child[index++] = parent2[i];
            chosen[parent2[i].nr] = true;
        }
    }

    return child;
}

void mutation(vector<Job>& seq) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, seq.size() - 1);

    int idx1 = dis(gen);
    int idx2 = dis(gen);

    swap(seq[idx1], seq[idx2]);
}

void genetic_alg(vector<vector<Job>>& population, int generations, double mutation_rate, int thread_id) {
    random_device rd;
    mt19937 rng(rd());
    uniform_real_distribution<double> dist(0.0, 1.0);

    for (int gen = 0; gen < generations; gen++) {
        sort(population.begin(), population.end(), [](vector<Job>& a, vector<Job>& b) {
            return cmax(a) < cmax(b);
        });

        vector<vector<Job>> new_population;
        int a = 0;
        int b = 1;

        for (int i = 0; i < population.size() / 2; i++) {
            vector<Job> parent1 = population[a];
            vector<Job> parent2 = population[b];
            a += 2;
            b += 2;
            vector<Job> child1 = cross(parent1, parent2);
            vector<Job> child2 = cross(parent2, parent1);

            if ((double)dist(rng) < mutation_rate) {
                mutation(child1);
            }
            if ((double)dist(rng) < mutation_rate) {
                mutation(child2);
            }

            new_population.push_back(child1);
            new_population.push_back(child2);
        }

        population = new_population;
    }
}

vector<Job> island_model(vector<Job>& jobs_seq, int num_islands, int population_size, int generations, double mutation_rate, int migration_interval) {
    vector<vector<vector<Job>>> islands(num_islands, vector<vector<Job>>(population_size));

    #pragma omp parallel for
    for (int i = 0; i < num_islands; i++) {
        for (int j = 0; j < population_size; j++) {
            islands[i][j] = random_sequence(jobs_seq);
        }
    }

    for (int gen = 0; gen < generations; gen++) {
        #pragma omp parallel for
        for (int i = 0; i < num_islands; i++) {
            genetic_alg(islands[i], 1, mutation_rate, i);
        }

        if (gen % migration_interval == 0) {
            #pragma omp barrier
            #pragma omp single
            {
                for (int i = 0; i < num_islands; i++) {
                    int next_island = (i + 1) % num_islands;
                    int migrant_count = population_size / 10;
                    for (int j = 0; j < migrant_count; j++) {
                        swap(islands[i][j], islands[next_island][j]);
                    }
                }
            }
        }
    }

    vector<Job> best_cmax;
    int bestCmax = INT_MAX;

    #pragma omp parallel for
    for (int i = 0; i < num_islands; i++) {
        sort(islands[i].begin(), islands[i].end(), [](vector<Job>& a, vector<Job>& b) {
            return cmax(a) < cmax(b);
        });

        int island_best_cmax = cmax(islands[i][0]);
        #pragma omp critical
        {
            if (island_best_cmax < bestCmax) {
                bestCmax = island_best_cmax;
                //std::cout<<"Znaleziono lepsze rozwiazanie: "<< bestCmax << " wyspa: " << i<<std::endl;
                best_cmax = islands[i][0];
            }
        }
    }

    return best_cmax;
}

int main() {
    int N, M, zad, t;
    vector<Job> vec;

    ifstream file("nehdata.txt");
    file >> N;
    file >> M;

    for (int i = 0; i < N; i++) {
        Job a(i + 1);
        for (int j = 0; j < M; j++) {
            file >> zad;
            a.machine.push_back(zad);
        }
        vec.push_back(a);
    }

    std::cout << "Ilosc zadan: " << N;
    std::cout << ", ilosc maszyn: " << M << endl;

    int num_islands = 4;
    omp_set_num_threads(num_islands);

    auto started = std::chrono::high_resolution_clock::now();
    vector<Job> best_solution = island_model(vec, num_islands, 60, 60, 0.01, 10);
    auto done = std::chrono::high_resolution_clock::now();
    t = std::chrono::duration_cast<std::chrono::milliseconds>(done - started).count();

    int cmax_n = cmax(best_solution);
    std::cout << "Cmax: " << cmax_n << endl;
    std::cout << "Czas: " << t << " ms" << endl;

    wyswietl(best_solution);

    return 0;
}
