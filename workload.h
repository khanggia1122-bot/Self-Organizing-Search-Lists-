#ifndef WORKLOAD_H
#define WORKLOAD_H

class Rng {
private:
    unsigned long long state;
public:
    Rng(unsigned long long seed) : state(seed != 0 ? seed : 88172645463325252ULL) {}
    unsigned long long next() {
        state ^= state >> 12;
        state ^= state << 25;
        state ^= state >> 27;
        return state * 2685821657736338717ULL;
    }
    int nextInt(int bound) { return (int)(next() % (unsigned long long)bound); }
};

inline void shuffle(int* a, int n, Rng& rng) {
    for (int i = n - 1; i > 0; i--) {
        int j = rng.nextInt(i + 1);
        int t = a[i];
        a[i] = a[j];
        a[j] = t;
    }
}

inline void makeShuffledKeys(int* out, int n, Rng& rng) {
    for (int i = 0; i < n; i++) out[i] = i;
    shuffle(out, n, rng);
}

inline void makeSkewedQueries(int* out, int count, int n, int hotCount, int hotPercent, Rng& rng) {
    for (int i = 0; i < count; i++) {
        if (rng.nextInt(100) < hotPercent) out[i] = rng.nextInt(hotCount);
        else out[i] = hotCount + rng.nextInt(n - hotCount);
    }
}

inline void makeUniformQueries(int* out, int count, int n, Rng& rng) {
    for (int i = 0; i < count; i++) out[i] = rng.nextInt(n);
}

#endif
