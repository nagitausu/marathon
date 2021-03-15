#pragma GCC optimize ("Ofast")
#pragma GCC optimize ("unroll-loops")
#pragma GCC target ("avx")

#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <iomanip>
#include <cstdlib>

using namespace std;

constexpr unsigned long long int CYCLES_PER_SEC = 2800000000;
constexpr double CYCLE_SEC = 1.0 / CYCLES_PER_SEC;
constexpr double TIME_LIMIT = 1.95;
constexpr double TIME_LIMIT_INV = 1.0 / TIME_LIMIT;
constexpr int ITER_PER_EPOCH = 100;

unsigned long long int get_cycle(){
    unsigned int low, high;
    __asm__ volatile("rdtsc" : "=a" (low), "=d" (high));
    return ((unsigned long long int)low) | ((unsigned long long int)high << 32);
}

double get_time(unsigned long long int begin_cycle){
    return (double)(get_cycle() - begin_cycle) * CYCLE_SEC;
}

// Fast randoms
// [0, 2^64)
inline unsigned xrand() {
  static unsigned x = 314159265, y = 358979323, z = 846264338, w = 327950288;
  unsigned t = x ^ x << 11; x = y; y = z; z = w; return w = w ^ w >> 19 ^ t ^ t >> 8;
}
 
// [a, b)
inline int xrand(int a, int b) { return a + xrand() % (b - a); }
 
// [0.0, 1.0)
inline double xrand_uniform() { return xrand() * 2.328306436538696e-10; }

inline double xexp(double x){
    if(x < -20.0){ return 0.0; }
    x = 1.0 + x * 0.00390625;
    x *= x; x *= x; x *= x; x *= x;
    x *= x; x *= x; x *= x; x *= x;
    return x;
}

int D;
int c[26];
int s[365][26];
int t[365];
int accum[400];

struct State{
    int schedule[365];
    int score;
    vector<vector<int>> to_be_hold;

    State() : to_be_hold(26, vector<int>(1, 0)){
    }

    void init() {
        accum[0] = 0;
        for(int i=1; i<400; ++i){
            accum[i] = accum[i-1] + i;
        }
        score = 0;
        for(int i=0; i<D; ++i){
            int val = 0;
            for(int j=0; j<26; ++j){
                int tmp = s[i][j] + (i + 1 - to_be_hold[j].back()) * c[j];
                if(tmp > val){
                    schedule[i] = j;
                    val = tmp;
                }
            }
            to_be_hold[schedule[i]].push_back(i + 1);
        }
        for(int i=0; i<26; ++i){
            to_be_hold[i].push_back(D+1);
        }
        calcScore();
    }

    void calcScore() {
        score = 0;
        int last[26] = {0};
        for (int d=0; d<D; ++d){
            // Add bonus
            score += s[d][schedule[d]];
            last[schedule[d]] = d + 1;
            // Sub frastration
            for (int i=0; i<26; ++i){
                score -= c[i] * (d + 1 - last[i]);
            }
        }
    }

    void remove(int x, int v){
        auto p = find(to_be_hold[v].begin(), to_be_hold[v].end(), x);
        p--;
        int l = *p;
        p++; p++;
        int r = *p;
        p--;
        to_be_hold[v].erase(p);
        score += (accum[x - l - 1] + accum[r - x - 1] - accum[r - l - 1]) * c[v];
        score -= s[x-1][v];
    }

    void add(int x, int v){
        int l = 0; int r = 0;
        for(int i=0; i<(int)to_be_hold[v].size(); ++i){
            if(to_be_hold[v][i] > x){
                r = to_be_hold[v][i]; 
                l = to_be_hold[v][i-1];
                to_be_hold[v].insert(to_be_hold[v].begin() + i, x);
                break;
            }
        }
        score += (accum[r - l - 1] - accum[x - l - 1] - accum[r - x - 1]) * c[v];
        score += s[x-1][v];
    }

    void pointUpdate(int x, int p, int v) {
        remove(x + 1, p);
        add(x + 1, v);
        schedule[x] = v;
    }

    void swapUpdate(int x, int y) {
        remove(x + 1, schedule[x]);
        remove(y + 1, schedule[y]);
        add(y + 1, schedule[x]);
        add(x + 1, schedule[y]);
        swap(schedule[x], schedule[y]);
    }

    void output(){
        for(int i=0; i<D; ++i){
            cout << schedule[i] + 1;
            printf("\n");
        }
        fflush(stdout);
    }
};

void solve(float temperature){
    auto start_cycle = get_cycle();
    double current_time = get_time(start_cycle);
    // Temperature must be > 0.0
    double temp_upper = temperature;
    double temp_lower = 0.1;
    State state;
    state.init();
    int itr_cnt = 0;
    while(current_time < TIME_LIMIT){
        current_time = get_time(start_cycle);
        double temp = temp_upper + (temp_lower - temp_upper) * current_time * TIME_LIMIT_INV;
        double temp_inv = 1.0 / temp;
        // Each epoch has ITER_PER_EPOCH times iteration
        for(int i=0; i<ITER_PER_EPOCH; i++){
            // Point Update
            if(itr_cnt % 2 == 0){
                int x = xrand() % D;
                int v = xrand() % 26;
                if(v == state.schedule[x]){ continue; }
                itr_cnt++;

                // Save previous states
                int score = state.score;
                int prev = state.schedule[x];

                // Update states
                state.pointUpdate(x, prev, v);
                // If you want to hill climb, do transition only with score < state.score.
                if(score < state.score || xrand_uniform() < xexp((state.score - score) * temp_inv)){
                // Transition (keep state)
                }
                else{
                // Rollback state
                    state.pointUpdate(x, v, prev);
                }
            }
            // Swap
            else{
                int x = xrand() % (D - 20);
                int y = x + xrand(1, 20);
                itr_cnt++;

                // Save previous states
                int score = state.score;

                // Update states
                state.swapUpdate(x, y);
                // If you want to hill climb, do transition only with score < state.score.
                if(score < state.score || xrand_uniform() < xexp((state.score - score) * temp_inv)){
                // Transition (keep state)
                }
                else{
                // Rollback state
                    state.swapUpdate(y, x);
                }
            }
        }
    }
    state.output();
    cerr << "iterations: " << itr_cnt << endl;
    cerr << "score: " << state.score << endl;
}

int main(int argc, char *argv[]){
    // Input
    cin >> D;
    for (int i=0; i<26; ++i){
        cin >> c[i]; 
    }
    for (int i=0; i<D; ++i){
        for (int j=0; j<26; ++j){
            cin >> s[i][j]; 
        }
    }
    float temperature = 500.0;
    if (argc > 1){
        temperature = atof(argv[1]);
    }
    solve(temperature);
    return 0;
}
