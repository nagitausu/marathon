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

using namespace std;

constexpr unsigned long long int CYCLES_PER_SEC = 2800000000;
constexpr double CYCLE_SEC = 1.0 / CYCLES_PER_SEC;
constexpr double TIME_LIMIT = 2.95;
constexpr double TIME_LIMIT_INV = 1.0 / TIME_LIMIT;

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
 
// [a, b]
inline int xrand(int a, int b) { return a + xrand() % (b - a + 1); }
 
// [0.0, 1.0)
inline double xrand_uniform() { return xrand() * 2.328306436538696e-10; }

inline double xexp(double x){
    if(x < -20.0){ return 0.0; }
    x = 1.0 + x / 256.0;
    x *= x; x *= x; x *= x; x *= x;
    x *= x; x *= x; x *= x; x *= x;
    return x;
}

int N;
int B1, B2, B3;
int lower[30][30];
int upper[30][30];

struct State{
    int field[30][30];
    int score;

    void init() {
        score = 0;
        for(int i=0; i<N; ++i){
            for(int j=0; j<N; ++j){
                field[i][j] = xrand(lower[i][j], upper[i][j]);
            }
        }
        for(int i=0; i<N; ++i){
            score += calcScore(i, i);
        }
    }

    void finalize(){
        score = 0;
        for(int i=0; i<N; ++i){
            score += calcScore(i, i);
        }
    }

    int calcScore(int x, int y) {
        int ret = 0;
        // Check w
        static int cumsum[31];
        int cs = 0;
        for(int i=0; i<N; ++i){
            cs += field[x][i];
            cumsum[i+1] = cs;
        }
        for(int i=0; i<N+1; ++i){
            for(int j=i+1; j<N+1; ++j){
                int diff = cumsum[j] - cumsum[i];
                if(diff == B1 || diff == B2 || diff == B3){ret++;}
            }
        }
        // Check h
        cs = 0;
        for(int i=0; i<N; ++i){
            cs += field[i][y];
            cumsum[i+1] = cs;
        }
        for(int i=0; i<N+1; ++i){
            for(int j=i+1; j<N+1; ++j){
                int diff = cumsum[j] - cumsum[i];
                if(diff == B1 || diff == B2 || diff == B3){ret++;}
            }
        }
        return ret;
    }

    int calcDiffScore(int x, int y) {
        int ret = 0;
        // Check w
        static int cumsum[31];
        int cs = 0;
        for(int i=0; i<N; ++i){
            cs += field[x][i];
            cumsum[i+1] = cs;
        }
        int left = max(0, y-9);
        int right = min(y+11, N+1);
        for(int i=left; i<y+1; ++i){
            for(int j=y+1; j<right; ++j){
                int diff = cumsum[j] - cumsum[i];
                if(diff == B1 || diff == B2 || diff == B3){ret++;}
            }
        }
        // Check h
        cs = 0;
        for(int i=0; i<N; ++i){
            cs += field[i][y];
            cumsum[i+1] = cs;
        }
        int up = max(0, x-9);
        int down = min(x+11, N+1);
        for(int i=up; i<x+1; ++i){
            for(int j=x+1; j<down; ++j){
                int diff = cumsum[j] - cumsum[i];
                if(diff == B1 || diff == B2 || diff == B3){ret++;}
            }
        }
        return ret;
    }

    void updateStates(int x, int y, int v) {
        int before = calcDiffScore(x, y);
        field[x][y] = v;
        int after = calcDiffScore(x, y);
        score += (after - before);
    }

    void output(){
        for(int i=0; i<N; ++i){
            for(int j=0; j<N; ++j){
                if(j>0) printf(" ");
                cout << field[i][j];
            }
            printf("\n");
        }
        fflush(stdout);
    }
};

void input(){
    cin >> N >> B1 >> B2 >> B3;
    for(int i=0; i<N; i++){
        for(int j=0; j<N; j++){
            cin >> lower[i][j];
        }
    }
    for(int i=0; i<N; i++){
        for(int j=0; j<N; j++){
            cin >> upper[i][j];
        }
    }
}

void solve(){
    auto start_cycle = get_cycle();
    double current_time = get_time(start_cycle);
    // Temperature must be > 0.0
    double temp_upper = 2.0;
    double temp_lower = 0.01;
    State state;
    state.init();
    int itr_cnt = 0;
    while(current_time < TIME_LIMIT){
        current_time = get_time(start_cycle);
        double temp = temp_upper + (temp_lower - temp_upper) * current_time * TIME_LIMIT_INV;
        double temp_inv = 1.0 / temp;
        // Each epoch has 100 iteration
        for(int i=0; i<100; i++){
            int x = xrand() % N;
            int y = xrand() % N;
            int v = xrand(lower[x][y], upper[x][y]);
            if(v == state.field[x][y]){ continue; }
            itr_cnt++;

            // Save previous states
            int score = state.score;
            int prev = state.field[x][y];

            // Update states
            state.updateStates(x, y, v);

            // If you want to hill climb, do transition only with score < state.score.
            if(score < state.score || xrand_uniform() < xexp((state.score - score) * temp_inv)){
            // Transition (keep state)
            }
            else{
            // Rollback state
                state.field[x][y] = prev;
                state.score = score;
            }
        }
    }
    state.finalize();
    cerr << "iterations: " << itr_cnt << endl;
    cerr << "score: " << state.score << endl;
    state.output();
}

int main(){
    input();
    solve();
    return 0;
}
