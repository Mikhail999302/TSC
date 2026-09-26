//// chi_square_test.cpp
////
//// Критерий хи-квадрат для проверки гипотезы о том, что выборка пар
//// (xi_1, xi_2) (выбор продукта на двух последовательных шагах)
//// подчиняется "Модели 2" (переходная матрица, где сначала разыгрывается
//// "повторный выбор/случайность" p_ch, а внутри него — реклама p_adv),
//// в то время как данные фактически генерируются из "Модели 1"
//// (реклама применяется первым шагом, а p_ch — внутри).
////
//// Параметры p_ch, p_adv и вектор p_i = P(xi_1 = i) считаются ИЗВЕСТНЫМИ
//// (не оцениваются по выборке), поэтому это классический критерий
//// согласия хи-квадрат Пирсона с полностью специфицированным
//// распределением и числом степеней свободы df = m*m - 1
//// (m*m ячеек совместного распределения (i,j), минус 1 линейная связь
////  из-за нормировки на общий объём выборки N).
////
//// Программа:
////   1) генерирует выборку размера N из Модели 1 (это ваша "первая
////      выборка"; при желании замените generateSample на свою
////      реализацию — интерфейс см. ниже);
////   2) считает статистику хи-квадрат относительно ожидаемых частот
////      Модели 2;
////   3) сравнивает p-value со уровнем значимости alpha;
////   4) повторяет это numTrials раз (по умолчанию 1000) и оценивает
////      эмпирическую МОЩНОСТЬ критерия как долю отклонений H0;
////   5) для контроля также считает эмпирический уровень значимости
////      (type I error), генерируя выборки из самой Модели 2 —
////      в этом случае доля отклонений должна быть близка к alpha.
////
//// Компиляция:
////   g++ -O2 -std=c++17 chi_square_test.cpp -o chi_square_test
////
//// Запуск:
////   ./chi_square_test
////
//// Все параметры (m, p_ch, p_adv, p_i, N, число испытаний, alpha)
//// задаются в структуре Params внутри main() — меняйте их там.
//
//#pragma once 
//#include <cmath>
//#include <cstdio>
//#include <iostream>
//#include <numeric>
//#include <random>
//#include <vector>
//
//// ----------------------------------------------------------------------
//// Регуляризованная неполная гамма-функция (нужна для p-value хи-квадрат)
//// Классический алгоритм (Numerical Recipes): ряд для P(a,x) при x < a+1,
//// непрерывная дробь для Q(a,x) при x >= a+1.
//// ----------------------------------------------------------------------
//
//static double gammln(double xx) {
//    static const double cof[6] = {
//        76.18009172947146,     -86.50532032941677,   24.01409824083091,
//        -1.231739572450155,    0.1208650973866179e-2, -0.5395239384953e-5 };
//    double x = xx, y = xx;
//    double tmp = x + 5.5;
//    tmp -= (x + 0.5) * std::log(tmp);
//    double ser = 1.000000000190015;
//    for (int j = 0; j < 6; j++) {
//        y += 1.0;
//        ser += cof[j] / y;
//    }
//    return -tmp + std::log(2.5066282746310005 * ser / x);
//}
//
//static double gammaSeries(double a, double x) {
//    if (x <= 0.0) return 0.0;
//    double gln = gammln(a);
//    double ap = a;
//    double sum = 1.0 / a;
//    double del = sum;
//    for (int n = 1; n <= 500; n++) {
//        ap += 1.0;
//        del *= x / ap;
//        sum += del;
//        if (std::fabs(del) < std::fabs(sum) * 1e-14) break;
//    }
//    return sum * std::exp(-x + a * std::log(x) - gln);
//}
//
//static double gammaContFrac(double a, double x) {
//    const double FPMIN = 1e-300;
//    double gln = gammln(a);
//    double b = x + 1.0 - a;
//    double c = 1.0 / FPMIN;
//    double d = 1.0 / b;
//    double h = d;
//    for (int i = 1; i <= 500; i++) {
//        double an = -i * (i - a);
//        b += 2.0;
//        d = an * d + b;
//        if (std::fabs(d) < FPMIN) d = FPMIN;
//        c = b + an / c;
//        if (std::fabs(c) < FPMIN) c = FPMIN;
//        d = 1.0 / d;
//        double del = d * c;
//        h *= del;
//        if (std::fabs(del - 1.0) < 1e-14) break;
//    }
//    return std::exp(-x + a * std::log(x) - gln) * h;
//}
//
//// Q(a,x) = верхняя регуляризованная неполная гамма-функция
//static double gammaQ(double a, double x) {
//    if (x < 0.0 || a <= 0.0) return 1.0;
//    if (x == 0.0) return 1.0;
//    if (x < a + 1.0) {
//        return 1.0 - gammaSeries(a, x);
//    }
//    else {
//        return gammaContFrac(a, x);
//    }
//}
//
//// p-value для статистики хи-квадрат с df степенями свободы:
//// p = P(Chi2_df >= stat) = Q(df/2, stat/2)
//static double chiSquarePValue(double stat, int df) {
//    if (stat <= 0.0) return 1.0;
//    return gammaQ(df / 2.0, stat / 2.0);
//}
//
//// ----------------------------------------------------------------------
//// Параметры моделей
//// ----------------------------------------------------------------------
//
//struct Params {
//    int m;                 // число продуктов (i, j = 0..m-1)
//    double p_ch;            // параметр p_ch
//    double p_adv;           // параметр p_adv
//    std::vector<double> p;  // вектор p_i = P(xi_1 = i), i = 0..m-1 (сумма = 1)
//};
//
//// Модель 1 ("реклама первым шагом"): pi_ij = P(xi_2=j | xi_1=i)
////   i=j, j=0:   p_adv + (1-p_adv)(1-p_ch + p_ch*p0)
////   i=j, j!=0:  (1-p_adv)(1-p_ch + p_ch*p_j)
////   i!=j, j=0:  p_adv + (1-p_adv)*p_ch*p0
////   i!=j, j!=0: (1-p_adv)*p_ch*p_j
//static double piModel1(int i, int j, const Params& par) {
//    const double pa = par.p_adv, pc = par.p_ch;
//    const std::vector<double>& p = par.p;
//    if (i == j) {
//        if (j == 0) return pa + (1.0 - pa) * (1.0 - pc + pc * p[0]);
//        else        return (1.0 - pa) * (1.0 - pc + pc * p[j]);
//    }
//    else {
//        if (j == 0) return pa + (1.0 - pa) * pc * p[0];
//        else        return (1.0 - pa) * pc * p[j];
//    }
//}
//
//// Модель 2 ("случайность первым шагом, реклама внутри"): pi_ij = P(xi_2=j | xi_1=i)
////   i=j, j=0:   (1-p_ch) + p_ch*p_adv + p_ch*(1-p_adv)*p0
////   i=j, j!=0:  (1-p_ch) + p_ch*(1-p_adv)*p_j
////   i!=j, j=0:  p_ch*p_adv + p_ch*(1-p_adv)*p0
////   i!=j, j!=0: p_ch*(1-p_adv)*p_j
//static double piModel2(int i, int j, const Params& par) {
//    const double pa = par.p_adv, pc = par.p_ch;
//    const std::vector<double>& p = par.p;
//    if (i == j) {
//        if (j == 0) return (1.0 - pc) + pc * pa + pc * (1.0 - pa) * p[0];
//        else        return (1.0 - pc) + pc * (1.0 - pa) * p[j];
//    }
//    else {
//        if (j == 0) return pc * pa + pc * (1.0 - pa) * p[0];
//        else        return pc * (1.0 - pa) * p[j];
//    }
//}
//
//// Совместная вероятность P(xi_1=i, xi_2=j) = pi_ij * p_i
//static std::vector<double> jointProbs(const Params& par, bool model1) {
//    int m = par.m;
//    std::vector<double> probs(m * m);
//    for (int i = 0; i < m; i++) {
//        for (int j = 0; j < m; j++) {
//            double pij = model1 ? piModel1(i, j, par) : piModel2(i, j, par);
//            probs[i * m + j] = pij * par.p[i];
//        }
//    }
//    return probs;
//}
//
//// ----------------------------------------------------------------------
//// Генерация выборки (замените на свою реализацию при необходимости —
//// достаточно, чтобы функция возвращала таблицу сопряжённости m x m
//// наблюдённых частот пар (xi_1=i, xi_2=j) размера N).
//// ----------------------------------------------------------------------
//static std::vector<std::vector<int>> generateSample(int N, const Params& par,
//    bool fromModel1,
//    std::mt19937& rng) {
//    int m = par.m;
//    std::vector<double> probs = jointProbs(par, fromModel1);
//    std::discrete_distribution<int> dist(probs.begin(), probs.end());
//
//    std::vector<std::vector<int>> counts(m, std::vector<int>(m, 0));
//    for (int n = 0; n < N; n++) {
//        int idx = dist(rng);
//        counts[idx / m][idx % m]++;
//    }
//    return counts;
//}
//
//// ----------------------------------------------------------------------
//// Статистика хи-квадрат наблюдённой таблицы против ожидаемой модели
//// (ожидаемые вероятности берутся из piModel2, т.к. H0: "выборка из Модели 2")
//// ----------------------------------------------------------------------
//static double chiSquareStat(const std::vector<std::vector<int>>& counts, int N,
//    const Params& par) {
//    int m = par.m;
//    double stat = 0.0;
//    for (int i = 0; i < m; i++) {
//        for (int j = 0; j < m; j++) {
//            double e = piModel2(i, j, par) * par.p[i] * N;
//            if (e < 1e-10) continue;  // вырожденная ячейка (P=0 в модели)
//            double diff = counts[i][j] - e;
//            stat += diff * diff / e;
//        }
//    }
//    return stat;
//}
//
//// ----------------------------------------------------------------------
//// Один запуск критерия: сгенерировать выборку размера N из указанной
//// "истинной" модели, посчитать статистику и p-value, решить reject/not.
//// ----------------------------------------------------------------------
//struct TestResult {
//    double stat;
//    double pvalue;
//    bool reject;  // true, если H0 (данные из Модели 2) отклонена
//};
//
//static TestResult runOneTest(int N, const Params& par, bool sampleFromModel1,
//    double alpha, std::mt19937& rng) {
//    auto counts = generateSample(N, par, sampleFromModel1, rng);
//    double stat = chiSquareStat(counts, N, par);
//    int df = par.m * par.m - 1;
//    double pvalue = chiSquarePValue(stat, df);
//    TestResult res;
//    res.stat = stat;
//    res.pvalue = pvalue;
//    res.reject = (pvalue < alpha);
//    return res;
//}
//
//// Эмпирическая оценка мощности/уровня значимости: повторить критерий
//// numTrials раз, вернуть долю отклонений H0.
//static double empiricalRejectionRate(int N, int numTrials, const Params& par,
//    bool sampleFromModel1, double alpha,
//    std::mt19937& rng) {
//    int rejections = 0;
//    for (int t = 0; t < numTrials; t++) {
//        TestResult r = runOneTest(N, par, sampleFromModel1, alpha, rng);
//        if (r.reject) rejections++;
//    }
//    return static_cast<double>(rejections) / numTrials;
//}
//
//void criteria() {
//    // ---------------- Настройки: меняйте здесь ----------------
//    Params par;
//    par.m = 300;                          // число продуктов
//    par.p_ch = 0.4;                     // параметр p_ch
//    par.p_adv = 0.3;                    // параметр p_adv
//    par.p = { 0.4, 0.3, 0.2, 0.1 };       // вектор p_i, должен суммироваться в 1
//
//    int N = 1000;         // размер одной выборки (пар xi_1,xi_2)
//    int numTrials = 1000; // число повторений Монте-Карло для оценки мощности
//    double alpha = 0.05;  // уровень значимости критерия
//    // ------------------------------------------------------------
//
//    // нормировка p на всякий случай
//    double sumP = std::accumulate(par.p.begin(), par.p.end(), 0.0);
//    for (double& v : par.p) v /= sumP;
//
//    std::mt19937 rng(std::random_device{}());
//
//    int df = par.m * par.m - 1;
//    std::cout << "m = " << par.m << ", df = " << df << "\n";
//    std::cout << "p_ch = " << par.p_ch << ", p_adv = " << par.p_adv << "\n";
//    std::cout << "N = " << N << ", numTrials = " << numTrials
//        << ", alpha = " << alpha << "\n\n";
//
//    // 1) Мощность: H0 = "выборка из Модели 2", данные реально из Модели 1.
//    //    Доля отклонений H0 = эмпирическая мощность критерия.
//    double power = empiricalRejectionRate(N, numTrials, par,
//        /*sampleFromModel1=*/true, alpha, rng);
//    std::cout << "Эмпирическая мощность критерия (H1: данные из Модели 1): "
//        << power << "\n";
//
//    // 2) Контроль уровня значимости: генерируем выборки САМОЙ Модели 2
//    //    и проверяем ту же H0 — доля отклонений должна быть близка к alpha.
//    double typeIError = empiricalRejectionRate(N, numTrials, par,
//        /*sampleFromModel1=*/false, alpha,
//        rng);
//    std::cout << "Эмпирический уровень значимости (H0 верна, ожидание ~alpha): "
//        << typeIError << "\n";
//}