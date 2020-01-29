// Reference:
// https://atcoder.jp/contests/chokudai004/submissions/7234003
// https://atcoder.jp/contests/chokudai004/submissions/7233272
// http://gasin.hatenadiary.jp/entry/2019/09/03/162613

// How to improve:
// http://shindannin.hatenadiary.com/entry/20121224/1356364040
// https://topcoder.g.hatena.ne.jp/tomerun/20171216/1513436397

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

const int N = 50;
bool visited[N+2][N+2];
int bfs_buff[2704];
int diff[] = {-1, 1, -50, 50};

struct State{
    int field[N+2][N+2];
    int dropped[N+2][N+2];
    int score;
    int score_l, score_r;

    void init() {
        std::fill(field[0], field[0] + 2704, -1);
        std::fill(dropped[0], dropped[0] + 2704, -1);
        score = 0;
        for(int i=1; i<N+1; ++i){
            for(int j=1; j<N+1; ++j){
                field[i][j] = xrand() % 3;
            }
        }
        calcScore();
    }

    void finalize() {
        score = 0;
        calcScore();
    }

    void dropLine(int y) {
        for(int i=1; i<N+1; ++i){
            dropped[i][y] = 0;
        }
        int h = N;
        for(int j=N; j>0; j--){
            if(field[j][y] == 0){ continue;}
            else if(field[j][y] <= 3){
                dropped[h][y] = field[j][y];
                h--;
            } 
            else if(field[j][y] <= 4){
                h = j;
                dropped[h][y] = field[j][y];
                h--;
            }
        }
    }

    int bfs(int x, int y, int& l, int& r){
        cout << x << " " << y << endl;
        int que_size = 1;
        int color = dropped[x][y];
        bfs_buff[0] = x * 52 + y;
        for(int i=0; i<que_size; i++){
            for(int j=0; j<4; ++i){
                int nx = (bfs_buff[i] + diff[j]) / 52;
                int ny = (bfs_buff[i] + diff[j]) % 52;
                cout << bfs_buff[i] + diff[j] << endl;
                if(dropped[nx][ny] == color && !visited[nx][ny]){
                    bfs_buff[que_size++] = nx * 52 + ny;
                    visited[nx][ny] = true;
                    l = min(l, ny);
                    r = max(r, ny);
                }
            }
        }
        return que_size;
    }

    void evaluateLine(int y) {
        std::fill(visited[0], visited[0] + 2704, false);
        if(y >= score_l and y <= score_r){ score = 0; }
        for(int i=1; i<N+1; ++i){
            if(dropped[i][y] == 1 || dropped[i][y] == 2){
                if(visited[i][y]){ continue; }
                int l = y, r = y;
                int ret = bfs(i, y, l, r);
                if(ret > score){
                    score_l = l; 
                    score_r = r;
                    score = ret; 
                }
            }
        }
    }

    void evaluateAll() {
        score = 0;
        std::fill(visited[0], visited[0] + 2704, false);
        for(int j=1; j<N+1; ++j){
            for(int i=1; i<N+1; ++i){
                if(dropped[i][j] == 1 || dropped[i][j] == 2){
                    if(visited[i][j]){ continue; }
                    int l = j, r = j;
                    int ret = bfs(i, j, l, r);
                    if(ret > score){
                        score_l = l; 
                        score_r = r;
                        score = ret; 
                    }
                }
            }
        }
    }

    void calcScore() {
        // Drop boxes
        for(int i=1; i<N+1; ++i){
            dropLine(i);
        }
        evaluateAll();
    }

    void updateScore(int x, int y, int p, int v) {
    }

    void output(){
        for(int i=0; i<N+2; ++i){
            for(int j=0; j<N+2; ++j){
                if(j>0) printf(" ");
                cout << field[i][j];
            }
            printf("\n");
        }
        printf("===\n");
        for(int i=0; i<N+2; ++i){
            for(int j=0; j<N+2; ++j){
                if(j>0) printf(" ");
                cout << dropped[i][j];
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
    // Temperature must be > 0.0
    double temp_upper = 30.0;
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
            int x = xrand() % N;
            int y = xrand() % N;
            int v = xrand() % 5;
            if(v == state.field[x+1][y+1]){ continue; }
            itr_cnt++;

            // Save previous states
            int score = state.score;
            int prev = state.field[x+1][y+1];

            // Update states
            state.field[x+1][y+1] = v;
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
