#pragma GCC optimize ("Ofast")
#pragma GCC optimize ("unroll-loops")
#pragma GCC target ("avx")

#include <string.h>
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
constexpr double TIME_LIMIT = 4.90;
constexpr double TIME_LIMIT_INV = 1.0 / TIME_LIMIT;
constexpr int ITER_PER_EPOCH = 100;

constexpr int LEFT = 0;
constexpr int RIGHT = 1;
constexpr int UP = 2;
constexpr int DOWN = 3;

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

int n;
int px[200];
int py[200];
int pR[200];


struct State{
    // includes left, right, up, down
    int rects[4][200];
    double score;

    State(){}

    void init() {
        score = 0.0;
        for(int i=0; i<n; ++i){
            rects[LEFT][i] = px[i];
            rects[RIGHT][i] = px[i] + 1;
            rects[UP][i] = py[i];
            rects[DOWN][i] = py[i] + 1;
        }
        calcScore();
    }

    double single_score(int i){
        double r, s, c;
        r = pR[i];
        s = (rects[1][i] - rects[0][i]) * (rects[3][i] - rects[2][i]);
        if(r < s){
            swap(r, s);
        }
        c = 1.0 - r / s;
        return 1.0 - c * c;
    }

    void calcScore() {
        double res = 0;
        for(int i=0; i<n; ++i){
            res += single_score(i);
        }
        score = res;
    }

    void output(){
        for(int i=0; i<n; ++i){
            cout << rects[0][i] << " "  << rects[2][i] << " " << rects[1][i] << " " << rects[3][i] << "\n";
        }
        fflush(stdout);
    }

    bool update(int x, int nl, int nr, int nu, int nd){
        rects[LEFT][x] = nl;
        rects[RIGHT][x] = nr;
        rects[UP][x] = nu;
        rects[DOWN][x] = nd;
        for(int i=0; i<n; ++i){
            if(i == x){continue;}
            if(is_collided(x, i)){return 1;}
        }
        calcScore();
        return 0;
    }

    bool is_collided(int i, int j){
        if(rects[RIGHT][i] <= rects[LEFT][j]){return 0;}
        if(rects[RIGHT][j] <= rects[LEFT][i]){return 0;}
        if(rects[DOWN][i] <= rects[UP][j]){return 0;}
        if(rects[DOWN][j] <= rects[UP][i]){return 0;}
        return 1;
    }

    bool is_in(int i){
        if(px[i] < rects[LEFT][i]){return 0;}
        if(px[i] >= rects[RIGHT][i]){return 0;}
        if(py[i] < rects[UP][i]){return 0;}
        if(py[i] >= rects[DOWN][i]){return 0;}
        return 1;
    }
};

void solve(float temperature){
    auto start_cycle = get_cycle();
    double current_time = get_time(start_cycle);
    // Temperature must be > 0.0
    double temp_upper = temperature;
    double temp_lower = 0.1;
    int pl, pr, pu, pd;
    State state;
    state.init();
    int itr_cnt = 0;
    int update_cnt = 0;
    int revert_cnt = 0;
    while(current_time < TIME_LIMIT){
        current_time = get_time(start_cycle);
        double temp = temp_upper + (temp_lower - temp_upper) * current_time * TIME_LIMIT_INV;
        double temp_inv = 1.0 / temp;
        // Each epoch has ITER_PER_EPOCH times iteration
        for(int i=0; i<ITER_PER_EPOCH; i++){
            // choose advertizement and direction
            int x = xrand() % n;
            int nl = state.rects[LEFT][x] + xrand(-2, 3);
            int nr = state.rects[RIGHT][x] + xrand(-2, 3);
            int nu = state.rects[UP][x] + xrand(-2, 3);
            int nd = state.rects[DOWN][x] + xrand(-2, 3);
            if(nr < px[x] || nl >= px[x] || nd < py[x] || nu >= py[x]){continue;}
            if(nr > 10000 || nl < 0 || nd > 10000 || nu < 0){continue;}
            // int area = (state.rects[RIGHT][x] - state.rects[LEFT][x]) * (state.rects[DOWN][x] - state.rects[UP][x]);
            // if(area < pR[x] && (nr - nl) * (nd - nu) < area){continue;}
            itr_cnt++;

            // Save previous states
            double prev_score = state.score;
            pl = state.rects[LEFT][x];
            pr = state.rects[RIGHT][x];
            pu = state.rects[UP][x];
            pd = state.rects[DOWN][x];

            // Update states
            bool ret = state.update(x, nl, nr, nu, nd);

            // If you want to hill climb, do transition only with score < state.score.
            if(!ret && (prev_score < state.score || xrand_uniform() < xexp(100.0*(state.score - prev_score) * temp_inv))){
                // Transition (keep state)
                update_cnt++;
            }
            else{
                // Rollback state
                state.rects[LEFT][x] = pl;
                state.rects[RIGHT][x] = pr;
                state.rects[UP][x] = pu;
                state.rects[DOWN][x] = pd;
                revert_cnt++;
            }
        }
    }
    state.output();
    cerr << "iterations: " << itr_cnt << endl;
    cerr << "update/revert: " << update_cnt << " " << revert_cnt << endl;
    cerr << "score: " << state.score << endl;
}

int main(int argc, char *argv[]){
    // Input
    cin >> n;
    for (int i=0; i<n; ++i){
        cin >> px[i]; 
        cin >> py[i]; 
        cin >> pR[i]; 
    }
    float temperature = 10.0;
    if (argc > 1){
        temperature = atof(argv[1]);
    }
    solve(temperature);
    return 0;
}