#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <iomanip>

using namespace std;
typedef long long int ll;
const unsigned long long int CYCLES_PER_SEC = 2800000000;
constexpr double TIME_LIMIT = 2.95;
double TEMP_HIGH = 30.0;
double TEMP_LOW = 10.0;

// Reference:
// https://atcoder.jp/contests/chokudai004/submissions/7234003
// https://atcoder.jp/contests/chokudai004/submissions/7233272
// http://gasin.hatenadiary.jp/entry/2019/09/03/162613

unsigned long long int getCycle(){
    unsigned int low, high;
    __asm__ volatile("rdtsc" : "=a" (low), "=d" (high));
    return ((unsigned long long int)low) | ((unsigned long long int)high << 32);
}

double getTime(unsigned long long int begin_cycle){
    return (double)(getCycle() - begin_cycle) / CYCLES_PER_SEC;
}

// [0, 2^64)
unsigned xrand() {
  static unsigned x = 314159265, y = 358979323, z = 846264338, w = 327950288;
  unsigned t = x ^ x << 11; x = y; y = z; z = w; return w = w ^ w >> 19 ^ t ^ t >> 8;
}
 
// [a, b]
int xrand(int a, int b) { return a + xrand() % (b - a + 1); }
 
// [0.0, 1.0)
double xrandUniform() { return xrand() / 4294967296.0; }

inline double exp1(double x){
    x = 1.0 + x / 256.0;
    x *= x; x *= x; x *= x; x *= x;
    x *= x; x *= x; x *= x; x *= x;
    return x;
}

int N = 30;

struct State{
    int field[30][30];
    int score;
    void init() {
    }
    void output() const{
        for(int x=0; x<N; ++x){
            for(int y=0; y<N; ++y){
                if(y>0) printf(" ");
                // print something here
            }
            printf("\n");
        }
        fflush(stdout);
    }
    void calcScore() {
    }
};

void input(){
}

void solve(){
    auto start_cycle = getCycle();
    State state;
    state.init();
    int cnt = 0;
    while(true){
        int x = xrand() % N + 1;
        int y = xrand() % N + 1;
        int v = xrand() % 10;
        if(v == state.field[x][y]){ continue; }
        cnt++;

        double now_t = getTime(start_cycle);
        if (now_t > TIME_LIMIT) break;

        double temp = TEMP_HIGH + (TEMP_LOW - TEMP_HIGH) * now_t / TIME_LIMIT;

        // Save previous states
        int score = state.score;
        int pre = state.field[x][y];

        // Update states
        state.field[x][y] = v;
        state.calcScore();

        if(score < state.score || xrandUniform() < exp1((state.score - score / temp))){
        // Got good result
        }
        else{
        // Bad result and rollback
            state.field[x][y] = pre;
            state.score = score;
        }
    }
    cerr << "iteration: " << cnt << endl;
    cerr << "score: " << state.score << endl;
    state.output();
}

int main(){
    input();
    solve();
    return 0;
}