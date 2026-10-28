#ifndef THEORY_H
#define THEORY_H

inline double theoryStaticRandomOrder(int n) {
    return (n + 1) / 2.0;
}

inline void hotColdProbabilities(int n, int hotCount, int hotPercent, double& pHot, double& pCold) {
    pHot = (hotPercent / 100.0) / hotCount;
    pCold = ((100 - hotPercent) / 100.0) / (n - hotCount);
}

inline double theoryOptimalStatic(int n, int hotCount, int hotPercent) {
    double pHot, pCold;
    hotColdProbabilities(n, hotCount, hotPercent, pHot, pCold);
    double sum = 0;
    for (int i = 1; i <= n; i++) sum += i * (i <= hotCount ? pHot : pCold);
    return sum;
}

inline double theoryMoveToFront(int n, int hotCount, int hotPercent) {
    double pHot, pCold;
    hotColdProbabilities(n, hotCount, hotPercent, pHot, pCold);
    double h = hotCount;
    double c = n - hotCount;
    return 1.0 + h * (h - 1) / 2.0 * pHot + c * (c - 1) / 2.0 * pCold
         + 2.0 * h * c * pHot * pCold / (pHot + pCold);
}

#endif
