// Reference:
// https://atcoder.jp/contests/chokudai004/submissions/7234003
// https://atcoder.jp/contests/chokudai004/submissions/7233272
// http://gasin.hatenadiary.jp/entry/2019/09/03/162613

// How to improve:
// http://shindannin.hatenadiary.com/entry/20121224/1356364040
// https://topcoder.g.hatena.ne.jp/tomerun/20171216/1513436397

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
inline double xrand_uniform() { return xrand() * 2.3283064365386963e-10; }

inline double xexp(double x){
    x = 1.0 + x / 256.0;
    x *= x; x *= x; x *= x; x *= x;
    x *= x; x *= x; x *= x; x *= x;
    return x;
}

const int N = 10;

struct State{
    int field[N][N];
    int score;

    void init() {
        score = 0;
        for(int i=0; i<N; ++i){
            for(int j=0; j<N; ++j){
                field[i][j] = 0;
            }
        }
        calcScore();
    }

    void calcScore() {
        for(int i=0; i<N; ++i){
            for(int j=0; j<N; ++j){
                score += field[i][j];
            }
        }
    }

    void updateScore(int x, int y, int p, int v) {
        score += v - p; 
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
}

void solve(){
    auto start_cycle = get_cycle();
    double current_time = get_time(start_cycle);
    double temp_upper = 10.0;
    double temp_lower = 0.1;
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
            int v = xrand() % 10;
            if(v == state.field[x][y]){ continue; }
            itr_cnt++;

            // Save previous states
            int score = state.score;
            int prev = state.field[x][y];

            // Update states
            state.field[x][y] = v;
            state.updateScore(x, y, prev, v);

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
    cerr << "iterations: " << itr_cnt << endl;
    cerr << "score: " << state.score << endl;
    state.output();
}

int main(){
    input();
    solve();
    return 0;
}