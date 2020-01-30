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
 
// [a, b]
inline int xrand(int a, int b) { return a + xrand() % (b - a + 1); }
 
// [0.0, 1.0)
inline double xrand_uniform() { return xrand() * 2.328306436538696e-10; }

inline double xexp(double x){
    if(x < -20.0){ return 0.0; }
    x = 1.0 + x * 0.00390625;
    x *= x; x *= x; x *= x; x *= x;
    x *= x; x *= x; x *= x; x *= x;
    return x;
}

const int N = 50;
bool visited[N+2][N+2];
int bfs_buff_x[2704];
int bfs_buff_y[2704];
int changeable_x[2704];
int changeable_y[2704];
int changeable_size = 0;
int init_field[N+2][N+2];

struct State{
    int field[N+2][N+2];
    int dropped[N+2][N+2];
    int score[2];
    int score_l[2];
    int score_r[2];

    void init() {
        std::fill(field[0], field[0] + 2704, -1);
        std::fill(dropped[0], dropped[0] + 2704, -1);
        score[0] = 0; score[1] = 0;
        score_l[0] = 0; score_l[1] = 0;
        score_r[0] = 0; score_r[1] = 0;
        for(int i=1; i<N+1; ++i){
            for(int j=1; j<N+1; ++j){
                field[i][j] = init_field[i][j];
            }
        }
        calcScore();
    }

    void finalize() {
        score[0] = 0; score[1] = 0;
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
        int que_size = 1;
        int color = dropped[x][y];
        bfs_buff_x[0] = x;
        bfs_buff_y[0] = y;
        visited[x][y] = true;
        for(int i=0; i<que_size; i++){
            int nx = bfs_buff_x[i];
            int ny = bfs_buff_y[i];
            nx++;
            if(dropped[nx][ny] == color && !visited[nx][ny]){
                bfs_buff_x[que_size] = nx;
                bfs_buff_y[que_size] = ny;
                que_size++;
                visited[nx][ny] = true;
            }
            nx--;
            nx--;
            if(dropped[nx][ny] == color && !visited[nx][ny]){
                bfs_buff_x[que_size] = nx;
                bfs_buff_y[que_size] = ny;
                que_size++;
                visited[nx][ny] = true;
            }
            nx++;
            ny++;
            if(dropped[nx][ny] == color && !visited[nx][ny]){
                bfs_buff_x[que_size] = nx;
                bfs_buff_y[que_size] = ny;
                que_size++;
                visited[nx][ny] = true;
                r = max(r, ny);
            }
            ny--;
            ny--;
            if(dropped[nx][ny] == color && !visited[nx][ny]){
                bfs_buff_x[que_size] = nx;
                bfs_buff_y[que_size] = ny;
                que_size++;
                visited[nx][ny] = true;
                l = min(l, ny);
            }
            ny++;
        }
        return que_size;
    }

    void evaluateLine(int y) {
        std::fill(visited[0], visited[0] + 2704, false);
        // evaluate 1s
        if(y >= score_l[0] and y <= score_r[0]){ score[0] = 0; }
        for(int i=1; i<N+1; ++i){
            if(dropped[i][y] == 1 && !visited[i][y]){
                int l = y, r = y;
                int ret = bfs(i, y, l, r);
                if(ret > score[0]){
                    score_l[0] = l; 
                    score_r[0] = r;
                    score[0] = ret; 
                }
            }
        }
        // evaluate 2s
        if(y >= score_l[1] and y <= score_r[1]){ score[1] = 0; }
        for(int i=1; i<N+1; ++i){
            if(dropped[i][y] == 2 && !visited[i][y]){
                int l = y, r = y;
                int ret = bfs(i, y, l, r);
                if(ret > score[1]){
                    score_l[1] = l; 
                    score_r[1] = r;
                    score[1] = ret; 
                }
            }
        }
    }

    void evaluateAll() {
        score[0] = 0; score[1] = 0;
        std::fill(visited[0], visited[0] + 2704, false);
        for(int j=1; j<N+1; ++j){
            for(int i=1; i<N+1; ++i){
                if(visited[i][j]){ continue; }
                int c = dropped[i][j];
                if(c == 1 || c == 2){
                    int l = j, r = j;
                    int ret = bfs(i, j, l, r);
                    if(ret > score[c-1]){
                        score_l[c-1] = l; 
                        score_r[c-1] = r;
                        score[c-1] = ret; 
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

    void updateScore(int x, int y) {
        dropLine(y);
        evaluateLine(y);
    }

    void output(){
        for(int i=1; i<N+1; ++i){
            for(int j=1; j<N+1; ++j){
                if(field[i][j] == 0){ cout << "."; }
                else if(field[i][j] == 1){ cout << "o"; }
                else if(field[i][j] == 2){ cout << "x"; }
                else if(field[i][j] == 3){ cout << "+"; }
                else if(field[i][j] == 4){ cout << "-"; }
            }
            printf("\n");
        }
        fflush(stdout);
    }
};

void input(){
    std::fill(init_field[0], init_field[0] + 2704, -1);
    for(int i=1; i<N+1; ++i){
        string s; cin >> s;
        for(int j=1; j<N+1; ++j){
            if(s[j-1] == '.'){ init_field[i][j] = 0; }
            else if(s[j-1] == 'o'){ init_field[i][j] = 1; }
            else if(s[j-1] == 'x'){ init_field[i][j] = 2; }
            if(init_field[i][j] == 0){
                changeable_x[changeable_size] = i;
                changeable_y[changeable_size] = j;
                changeable_size++;
            }
        }
    }
}

void solve(){
    // Temperature must be > 0.0
    double temp_upper = 1.0;
    double temp_lower = 0.1;
    State state;
    State best_state;
    int itr_cnt = 0;
    int max_score = 0;
    for(int trial=0; trial<5; trial++){
        auto start_cycle = get_cycle();
        double current_time = get_time(start_cycle);
        state.init();
        int prev_score[2];
        int prev;
        int prev_dropped[52];
        int prev_l[2];
        int prev_r[2];
        while(current_time < TIME_LIMIT){
            current_time = get_time(start_cycle);
            double temp = temp_upper + (temp_lower - temp_upper) * current_time * TIME_LIMIT_INV;
            double temp_inv = 1.0 / temp;
            // Each epoch has ITER_PER_EPOCH times iteration
            for(int i=0; i<ITER_PER_EPOCH; i++){
                int id = xrand() % changeable_size;
                int x = changeable_x[id];
                int y = changeable_y[id];
                int v = 3 - state.field[x][y];
                itr_cnt++;

                // Save previous states
                prev = state.field[x][y];
                for(int j=0; j<2; j++){
                    prev_score[j] = state.score[j];
                    prev_l[j] = state.score_l[j];
                    prev_r[j] = state.score_r[j];
                }
                for(int j=0; j<52; j++){
                    prev_dropped[j] = state.dropped[j][y];
                } 

                // Update states
                state.field[x][y] = v;
                state.updateScore(x, y);

                // If you want to hill climb, do transition only with score < state.score.
                if(prev_score[0] + prev_score[1] <= state.score[0] + state.score[1] || xrand_uniform() < xexp((state.score[0] + state.score[1] - prev_score[0] - prev_score[1]) * temp_inv)){
                // Transition (keep state)
                    if(current_time > 1.5 && state.score[0] + state.score[1] > max_score){
                        for(int i=0; i<N+2; i++){
                            for(int j=0; j<N+2; j++){
                                best_state.field[i][j] = state.field[i][j];
                            }
                        }
                        max_score = state.score[0] + state.score[1];
                    }
                }
                else{
                // Rollback state
                    state.field[x][y] = prev;
                    for(int j=0; j<52; j++){
                        state.dropped[j][y] = prev_dropped[j];
                    }
                    for(int j=0; j<2; j++){
                        state.score[j] = prev_score[j];
                        state.score_l[j] = prev_l[j];
                        state.score_r[j] = prev_r[j];
                    }
                }
            }
        }
    }
    best_state.finalize();
    cerr << "iterations: " << itr_cnt << endl;
    cerr << "score: " << best_state.score[0] + best_state.score[1] << endl;
    best_state.output();
}

int main(){
    input();
    solve();
    return 0;
}
