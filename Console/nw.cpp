
//
// Критерий хи-квадрат для двухшагового опроса (модели А и Б), встроенный в проект TSC.
//   * выборки генерируются генераторами TSC:
//       модель А: TSCGenerator::CGenerator_1Pos,       оценки: GroupedTSC::CTSCModel_1Pos
//       модель Б: TSCGenerator::CGenerator_1Pos_mine,  оценки: GroupedTSC::CTSCModel_1Pos_mine
//   * при numUnknown == 2 простые и одношаговые оценки p_ch, p_adv берутся из TSC;
//   * при numUnknown == 1 TSC не подходит (он оценивает оба параметра сразу),
//     поэтому используются собственные оценки (estimateSimple / oneStepUpdate);
//   * при numUnknown == 0 параметры известны и берутся из настроек.
// Параметры эксперимента задаются в начале chi2test::run() (setDistribution больше не нужна).
//
// numUnknown:
//   0 -> ВСЕ параметры (p_ch, p_adv) известны, ничего не оценивается, df не уменьшается;
//   1 -> неизвестен только p_ch (p_adv известен);
//   2 -> неизвестны p_ch и p_adv (оценки из TSC).
// p_i всегда известны (par.p).
//
// Три режима таблицы (параметр mode):
//   0 - полная таблица m*m ячеек, df = m*m - 1 - numUnknown;
//   1 - ОБЪЕДИНЕНИЕ: m-1 ячеек (i,i), i!=0; m-1 ячеек (i,0), i!=0 и одна
//       категория из (0,0) и всех (i,j), i!=j, j!=0.  df = 2m-2 - numUnknown;
//   2 - ОТБРАСЫВАНИЕ: только 2(m-1) информативных ячеек, df = 2m-3 - numUnknown.
// Обрезания оценок до [0,1] нет. Если pi_ij <= 0, p-value = 0.

//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>
#pragma once
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif
#include <algorithm>
#include <clocale>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <numeric>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include "HelperFunctions.h"

namespace chi2test {

    typedef std::vector<std::vector<int> > Counts;

    // ----------------------------------------------------------------------
    // p-value хи-квадрат через регуляризованную неполную гамма-функцию
    // ----------------------------------------------------------------------

    static double gammln(double xx) {
        static const double cof[6] = {
            76.18009172947146,   -86.50532032941677,   24.01409824083091,
            -1.231739572450155,  0.1208650973866179e-2, -0.5395239384953e-5 };
        double x = xx, y = xx;
        double tmp = x + 5.5;
        tmp -= (x + 0.5) * std::log(tmp);
        double ser = 1.000000000190015;
        for (int j = 0; j < 6; j++) {
            y += 1.0;
            ser += cof[j] / y;
        }
        return -tmp + std::log(2.5066282746310005 * ser / x);
    }

    static double gammaSeries(double a, double x) {
        if (x <= 0.0) return 0.0;
        double gln = gammln(a);
        double ap = a;
        double sum = 1.0 / a;
        double del = sum;
        for (int n = 1; n <= 500; n++) {
            ap += 1.0;
            del *= x / ap;
            sum += del;
            if (std::fabs(del) < std::fabs(sum) * 1e-14) break;
        }
        return sum * std::exp(-x + a * std::log(x) - gln);
    }

    static double gammaContFrac(double a, double x) {
        const double FPMIN = 1e-300;
        double gln = gammln(a);
        double b = x + 1.0 - a;
        double c = 1.0 / FPMIN;
        double d = 1.0 / b;
        double h = d;
        for (int i = 1; i <= 500; i++) {
            double an = -i * (i - a);
            b += 2.0;
            d = an * d + b;
            if (std::fabs(d) < FPMIN) d = FPMIN;
            c = b + an / c;
            if (std::fabs(c) < FPMIN) c = FPMIN;
            d = 1.0 / d;
            double del = d * c;
            h *= del;
            if (std::fabs(del - 1.0) < 1e-14) break;
        }
        return std::exp(-x + a * std::log(x) - gln) * h;
    }

    static double gammaQ(double a, double x) {
        if (x < 0.0 || a <= 0.0) return 1.0;
        if (x == 0.0) return 1.0;
        if (x < a + 1.0) return 1.0 - gammaSeries(a, x);
        return gammaContFrac(a, x);
    }

    // p = P(Chi2_df >= stat)
    static double chiSquarePValue(double stat, int df) {
        if (stat <= 0.0) return 1.0;
        return gammaQ(df / 2.0, stat / 2.0);
    }

    // ----------------------------------------------------------------------
    // Модели
    // ----------------------------------------------------------------------

    struct Params {
        int m;                  // число продуктов
        double p_ch;
        double p_adv;
        std::vector<double> p;  // p_i = P(xi_1 = i), ИЗВЕСТНЫ
    };

    // Модель 1 (А): реклама первой
    static double piModel1(int i, int j, const Params& par) {
        const double pa = par.p_adv, pc = par.p_ch;
        const std::vector<double>& p = par.p;
        if (i == j) {
            if (j == 0) return pa + (1.0 - pa) * (1.0 - pc + pc * p[0]);
            return (1.0 - pa) * (1.0 - pc + pc * p[j]);
        }
        if (j == 0) return pa + (1.0 - pa) * pc * p[0];
        return (1.0 - pa) * pc * p[j];
    }

    // Модель 2 (Б): лояльность первой
    static double piModel2(int i, int j, const Params& par) {
        const double pa = par.p_adv, pc = par.p_ch;
        const std::vector<double>& p = par.p;
        if (i == j) {
            if (j == 0) return (1.0 - pc) + pc * pa + pc * (1.0 - pa) * p[0];
            return (1.0 - pc) + pc * (1.0 - pa) * p[j];
        }
        if (j == 0) return pc * pa + pc * (1.0 - pa) * p[0];
        return pc * (1.0 - pa) * p[j];
    }

    static double piModel(bool model1, int i, int j, const Params& par) {
        return model1 ? piModel1(i, j, par) : piModel2(i, j, par);
    }

    static std::vector<double> jointProbs(const Params& par, bool model1) {
        int m = par.m;
        std::vector<double> probs(m * m);
        for (int i = 0; i < m; i++)
            for (int j = 0; j < m; j++)
                probs[i * m + j] = piModel(model1, i, j, par) * par.p[i];
        return probs;
    }

    // ----------------------------------------------------------------------
    // Оценка (простая или одношаговая)
    // ----------------------------------------------------------------------

    struct Est {
        double p_ch;
        double p_adv;
        bool outside;  // true, если хотя бы одна оценка вне [0, 1] (без обрезания!)
    };

    static bool outside01(double v) { return v < 0.0 || v > 1.0; }

    // ----------------------------------------------------------------------
    // ИНТЕГРАЦИЯ С TSC: генерация выборки и оценки
    // ----------------------------------------------------------------------

    // true, если в таблице TSC строка = xi_2, а столбец = xi_1 (проверьте по sanityCheckTsc)
    static bool g_transposeSample = false;
    static int g_tscFailed = 0;     // число сбоев TSC (исключений), для сообщений

    // Вектор параметров для генераторов TSC: p_0..p_{m-1}, p_ch, p_adv
    static TSCGenerator::doubles makeDistrib(const Params& par) {
        TSCGenerator::doubles d;
        for (size_t i = 0; i + 1 < par.p.size(); ++i) d.push_back(par.p[i]);
        d.push_back(par.p_ch);
        d.push_back(par.p_adv);
        return d;
    }

    // Плоский вектор m*m из TSC -> таблица частот counts[xi_1][xi_2]
    static void tableFromSample(const doubles& sample, int m, int N, Counts& counts) {
        if (static_cast<int>(sample.size()) != m * m) {
            std::cerr << "Размер выборки TSC = " << sample.size() << ", ожидалось m*m = " << m * m << "\n";
            std::exit(1);
        }
        counts.assign(m, std::vector<int>(m, 0));
        long total = 0;
        for (int i = 0; i < m; i++) {
            for (int j = 0; j < m; j++) {
                int c = static_cast<int>(std::floor(sample[i * m + j] + 0.5));
                if (g_transposeSample) counts[j][i] = c; else counts[i][j] = c;
                total += c;
            }
        }
        if (total != N) {
            std::cerr << "Сумма ячеек выборки TSC = " << total << " != N = " << N
                << ": выборка TSC - не таблица частот?\n";
            std::exit(1);
        }
    }

    // Оценки из TSC: индексы p_ch = m-1, p_adv = m (для m = 3 это 2 и 3, как в исходном main).
    static void readTscEstimates(GroupedTSC::CModel* model, int m, Est& s, Est& o) {
        const int iCh = m - 1, iAdv = m;

        model->ProcessInitialCalculation(true);
        if (static_cast<int>(model->GetEstimateCount()) != m + 1) {
            std::cerr << "TSC вернул " << model->GetEstimateCount() << " оценок, ожидалось m+1 = "
                << m + 1 << ": проверьте индексы p_ch и p_adv\n";
            std::exit(1);
        }
        static bool printedNames = false;   // один раз печатаем список оценок для проверки индексов
        if (!printedNames) {
            printedNames = true;
            std::cout << "Оценки TSC (индекс: имя = значение); p_ch = [" << iCh << "], p_adv = [" << iAdv << "]:\n";
            for (int i = 0; i < static_cast<int>(model->GetEstimateCount()); ++i)
                std::cout << "  [" << i << "] " << model->GetEstimateAt(i).first << " = "
                << model->GetEstimateAt(i).second.Value << "\n";
            std::cout << "\n";
        }
        s.p_ch = model->GetEstimateAt(iCh).second.Value;
        s.p_adv = model->GetEstimateAt(iAdv).second.Value;
        s.outside = outside01(s.p_ch) || outside01(s.p_adv);

        model->ProcessEstimatesCalculation();
        o.p_ch = model->GetEstimateAt(iCh).second.Value;
        o.p_adv = model->GetEstimateAt(iAdv).second.Value;
        o.outside = outside01(o.p_ch) || outside01(o.p_adv);
    }

    // Генерирует выборку генератором Gen (A или Б) с параметрами genPar и, если wantEst,
    // считает по ней оценки моделью TSC (testIsA: CTSCModel_1Pos или CTSCModel_1Pos_mine).
    // Возвращает false при исключении в TSC.
    template <class Gen>
    static bool tscRun(const Params& genPar, int N, long seed, bool wantEst, bool testIsA,
        Counts& counts, Est& s, Est& o) {
        try {
            TSCGenerator::doubles distrib = makeDistrib(genPar);
            std::auto_ptr<Gen> generator(new Gen(distrib, seed));
            generator->SetSize(N);
            generator->Generate();
            doubles sample = generator->GetSample();
            tableFromSample(sample, genPar.m, N, counts);
            if (!wantEst) return true;

            initDumper();
            MathModels::TLDotCacher ldotCacher;
            MathModels::TInfoCacher infoCacher;
            // как и в исходном main, sampleManager не удаляется (владение неизвестно)
            Tsc::TableManager::CTableManager<double>* sampleManager =
                Tsc::TableManager::TSampleManagerFactory::CreateFromGenerator(generator.get());
            if (testIsA) {
                std::auto_ptr<GroupedTSC::CModel> model(GroupedTSC::CModel::CreateCModel(
                    sampleManager->GetWidth(), sampleManager, false,
                    new GroupedTSC::CTSCModel_1Pos(sampleManager->GetWidth(), ETSCModel, ldotCacher, infoCacher)));
                readTscEstimates(model.get(), genPar.m, s, o);
            }
            else {
                std::auto_ptr<GroupedTSC::CModel> model(GroupedTSC::CModel::CreateCModel(
                    sampleManager->GetWidth(), sampleManager, false,
                    new GroupedTSC::CTSCModel_1Pos_mine(sampleManager->GetWidth(), ETSCModel, ldotCacher, infoCacher)));
                readTscEstimates(model.get(), genPar.m, s, o);
            }
            return true;
        }
        catch (std::exception& e) {
            if (++g_tscFailed <= 5) std::cout << "exception! " << e.what() << std::endl;
            return false;
        }
        catch (...) {
            if (++g_tscFailed <= 5) std::cout << "unknown exception in TSC" << std::endl;
            return false;
        }
    }

    // Зерно берем из нашего генератора: std::clock() имеет разрешение ~1 мс, и в быстром
    // цикле подряд идущие выборки получали бы одинаковое зерно (одинаковые выборки).
    static long nextSeed(std::mt19937& rng) {
        return static_cast<long>(rng() & 0x7fffffffu);
    }

    static Counts generateSample(int N, const Params& par, bool fromModel1, std::mt19937& rng) {
        Counts counts;
        Est s, o;
        long seed = nextSeed(rng);
        bool ok = fromModel1
            ? tscRun<TSCGenerator::CGenerator_1Pos>(par, N, seed, false, true, counts, s, o)
            : tscRun<TSCGenerator::CGenerator_1Pos_mine>(par, N, seed, false, true, counts, s, o);
        if (!ok) { std::cerr << "Не удалось сгенерировать выборку в TSC\n"; std::exit(1); }
        return counts;
    }

    // ----------------------------------------------------------------------
    // РЕЖИМЫ ТАБЛИЦЫ
    // ----------------------------------------------------------------------

    static bool isInformative(int i, int j) {
        return i != 0 && (j == 0 || i == j);
    }

    static const char* modeName(int mode) {
        static const char* names[3] = { "полная таблица", "объединение", "отбрасывание" };
        return names[mode];
    }

    // df = (число категорий) - 1 - (число оцениваемых параметров)
    static int chiDf(int m, int mode, int numEstimated) {
        int cells = (mode == 0) ? m * m : (mode == 1 ? 2 * (m - 1) + 1 : 2 * (m - 1));
        return cells - 1 - numEstimated;
    }

    static void requireDf(int df) {
        if (df < 1) {
            std::cerr << "df < 1 (" << df << "): для этого mode/m/numUnknown критерий не определен\n";
            std::exit(1);
        }
    }

    static double chiSquareStat(const Counts& counts, int N,
        const Params& par, bool testModel1, int mode = 0) {
        int m = par.m;
        double stat = 0.0, eRest = 0.0, nRest = 0.0;
        for (int i = 0; i < m; i++) {
            for (int j = 0; j < m; j++) {
                double e = piModel(testModel1, i, j, par) * par.p[i] * N;
                if (mode != 0 && !isInformative(i, j)) {
                    eRest += e;
                    nRest += counts[i][j];
                    continue;
                }
                if (e < 1e-10) continue;
                double diff = counts[i][j] - e;
                stat += diff * diff / e;
            }
        }
        if (mode == 1 && eRest > 1e-10) {   // объединенная категория
            double diff = nRest - eRest;
            stat += diff * diff / eRest;
        }
        return stat;                        // mode == 2: категория просто отброшена
    }

    // ----------------------------------------------------------------------
    // Критерий при ПОЛНОСТЬЮ известных параметрах
    // ----------------------------------------------------------------------

    static double empiricalRejectionRate(int N, int numTrials, const Params& par,
        bool sampleFromModel1, bool testModel1, double alpha, std::mt19937& rng,
        int mode = 0) {
        int rejections = 0;
        int df = chiDf(par.m, mode, 0);
        requireDf(df);
        for (int t = 0; t < numTrials; t++) {
            Counts counts = generateSample(N, par, sampleFromModel1, rng);
            double stat = chiSquareStat(counts, N, par, testModel1, mode);
            if (chiSquarePValue(stat, df) < alpha) rejections++;
        }
        return static_cast<double>(rejections) / numTrials;
    }

    // ----------------------------------------------------------------------
    // СОБСТВЕННЫЕ ОЦЕНКИ (используются только при numUnknown == 1; при 2 - оценки TSC)
    // ----------------------------------------------------------------------

    // Простая оценка (метод моментов)
    static Est estimateSimple(const Counts& counts, int N,
        const Params& par, bool model1, int numUnknown) {
        if (numUnknown == 0) {          // все параметры известны: берем из par
            Est e;
            e.p_ch = par.p_ch;
            e.p_adv = par.p_adv;
            e.outside = false;
            return e;
        }

        int m = par.m;
        const std::vector<double>& p = par.p;
        double p0 = p[0];
        double S2 = 0.0;
        for (size_t k = 0; k < p.size(); ++k) S2 += p[k] * p[k];

        int colSum0 = 0, trace = 0;
        for (int i = 0; i < m; i++) {
            colSum0 += counts[i][0];
            trace += counts[i][i];
        }
        double P2_0 = static_cast<double>(colSum0) / N;  // P(xi_2 = 0)
        double Q = static_cast<double>(trace) / N;        // P(xi_1 = xi_2)

        double pc, pa;
        if (numUnknown == 2) {
            if (model1) {
                pa = (P2_0 - p0) / (1.0 - p0);
                pc = (p0 * pa + (1.0 - pa) - Q) / ((1.0 - pa) * (1.0 - S2));
            }
            else {
                double x = (P2_0 - p0) / (1.0 - p0);  // p_ch * p_adv
                pc = (1.0 - Q + x * (p0 - S2)) / (1.0 - S2);
                pa = (std::fabs(pc) > 1e-12) ? x / pc : 0.0;
            }
        }
        else {  // numUnknown == 1: p_adv известен, оцениваем p_ch по Q
            pa = par.p_adv;
            if (model1)
                pc = (p0 * pa + (1.0 - pa) - Q) / ((1.0 - pa) * (1.0 - S2));
            else
                pc = (1.0 - Q) / (1.0 - pa * p0 - (1.0 - pa) * S2);
        }
        Est e;
        e.p_ch = pc;
        e.p_adv = (numUnknown == 2) ? pa : par.p_adv;
        e.outside = outside01(e.p_ch) || outside01(e.p_adv);
        return e;
    }

    // Один шаг метода Фишера от простой оценки. p_i фиксированы (= par.p).
    static Est oneStepUpdate(const Counts& counts, int N,
        const Params& par, const Est& start, bool model1, int numUnknown) {
        if (numUnknown == 0) return start;   // обновлять нечего

        int m = par.m;
        const double h = 1e-5;
        const int k = numUnknown;

        double U[2] = { 0.0, 0.0 };
        double Info[2][2] = { {0.0, 0.0}, {0.0, 0.0} };

        Params t0 = par;
        t0.p_ch = start.p_ch;
        t0.p_adv = start.p_adv;

        for (int i = 0; i < m; i++) {
            for (int j = 0; j < m; j++) {
                double pij = piModel(model1, i, j, t0);
                if (std::fabs(pij) < 1e-12) continue;  // деление на ~0

                double d[2] = { 0.0, 0.0 };
                {
                    Params a = t0, b = t0;
                    a.p_ch += h; b.p_ch -= h;
                    d[0] = (piModel(model1, i, j, a) - piModel(model1, i, j, b)) / (2 * h);
                }
                if (k == 2) {
                    Params a = t0, b = t0;
                    a.p_adv += h; b.p_adv -= h;
                    d[1] = (piModel(model1, i, j, a) - piModel(model1, i, j, b)) / (2 * h);
                }

                double nij = counts[i][j];
                double w = N * par.p[i] / pij;
                for (int r = 0; r < k; r++) {
                    U[r] += nij / pij * d[r];
                    for (int s = 0; s < k; s++) Info[r][s] += w * d[r] * d[s];
                }
            }
        }

        Est out = start;
        double dch = 0.0, dadv = 0.0;
        if (k == 1) {
            if (std::fabs(Info[0][0]) > 1e-10) dch = U[0] / Info[0][0];
        }
        else {
            double det = Info[0][0] * Info[1][1] - Info[0][1] * Info[1][0];
            if (std::fabs(det) > 1e-10) {
                dch = (U[0] * Info[1][1] - U[1] * Info[0][1]) / det;
                dadv = (Info[0][0] * U[1] - Info[1][0] * U[0]) / det;
            }
        }
        out.p_ch = start.p_ch + dch;
        out.p_adv = (k == 2) ? start.p_adv + dadv : par.p_adv;
        out.outside = outside01(out.p_ch) || outside01(out.p_adv);
        return out;
    }

    // Одно испытание: выборка (генератор TSC) + оценки для проверяемой модели.
    //   k == 2      -> оценки из TSC (модель testIsA);
    //   k == 0 или 1 -> собственные оценки по таблице частот.
    // genPar - параметры генерации, testPar - параметры проверяемой модели (для k < 2).
    // Возвращает false, если TSC выдал исключение (испытание пропускается).
    static bool drawTrial(int N, const Params& genPar, bool genIsA,
        const Params& testPar, bool testIsA, int k, std::mt19937& rng,
        Counts& counts, Est& s, Est& o) {
        long seed = nextSeed(rng);
        if (k == 2) {
            return genIsA
                ? tscRun<TSCGenerator::CGenerator_1Pos>(genPar, N, seed, true, testIsA, counts, s, o)
                : tscRun<TSCGenerator::CGenerator_1Pos_mine>(genPar, N, seed, true, testIsA, counts, s, o);
        }
        bool ok = genIsA
            ? tscRun<TSCGenerator::CGenerator_1Pos>(genPar, N, seed, false, testIsA, counts, s, o)
            : tscRun<TSCGenerator::CGenerator_1Pos_mine>(genPar, N, seed, false, testIsA, counts, s, o);
        if (!ok) return false;
        s = estimateSimple(counts, N, testPar, testIsA, k);
        o = oneStepUpdate(counts, N, testPar, s, testIsA, k);
        return true;
    }

    // ----------------------------------------------------------------------
    // p-value критерия с ОЦЕНЕННЫМИ параметрами при верной H0
    // (при numUnknown == 0 - критерий с известными параметрами)
    // ----------------------------------------------------------------------

    struct EstPValues {
        std::vector<double> simple, onestep;
        double outSimple = 0.0, outOnestep = 0.0;   // доли оценок вне [0,1]
        double badSimple = 0.0, badOnestep = 0.0;   // доли выборок, где pi_ij <= 0
        int failed = 0;                             // испытаний пропущено из-за сбоя TSC
    };

    static bool modelIsValid(const Params& par, bool model1) {
        for (int i = 0; i < par.m; i++)
            for (int j = 0; j < par.m; j++)
                if (!(piModel(model1, i, j, par) * par.p[i] > 0.0)) return false;
        return true;
    }

    static EstPValues collectPValuesEstimated(int N, int numTrials, const Params& par,
        bool modelIsA, int numUnknown, std::mt19937& rng, int mode = 0) {
        EstPValues res;
        res.simple.reserve(numTrials);
        res.onestep.reserve(numTrials);
        int df = chiDf(par.m, mode, numUnknown);
        requireDf(df);
        int os = 0, oo = 0, bs = 0, bo = 0, valid = 0;
        for (int t = 0; t < numTrials; t++) {
            Counts counts;
            Est s, o;
            if (!drawTrial(N, par, modelIsA, par, modelIsA, numUnknown, rng, counts, s, o)) {
                res.failed++;
                continue;
            }
            valid++;
            if (s.outside) os++;
            if (o.outside) oo++;

            Params ps = par, po = par;  // p_i остаются истинными
            ps.p_ch = s.p_ch; ps.p_adv = s.p_adv;
            po.p_ch = o.p_ch; po.p_adv = o.p_adv;
            if (modelIsValid(ps, modelIsA))
                res.simple.push_back(chiSquarePValue(chiSquareStat(counts, N, ps, modelIsA, mode), df));
            else { res.simple.push_back(0.0); bs++; }
            if (modelIsValid(po, modelIsA))
                res.onestep.push_back(chiSquarePValue(chiSquareStat(counts, N, po, modelIsA, mode), df));
            else { res.onestep.push_back(0.0); bo++; }
        }
        double denom = valid > 0 ? valid : 1;
        res.outSimple = os / denom;
        res.outOnestep = oo / denom;
        res.badSimple = bs / denom;
        res.badOnestep = bo / denom;
        return res;
    }

    // ----------------------------------------------------------------------
    // МОЩНОСТЬ ПРОТИВ АЛЬТЕРНАТИВНОЙ МОДЕЛИ
    // Данные генерируются из sampleModelIsA с параметрами (trueCh, trueAdv);
    // проверяется гипотеза "выборка из модели testModelIsA".
    //   numUnknown == 0: параметры проверяемой модели = (trueCh, trueAdv), известны;
    //   numUnknown == 1: p_adv = trueAdv известен, p_ch оценивается;
    //   numUnknown == 2: оба оцениваются (А и Б тогда неразличимы), оценки из TSC.
    // ----------------------------------------------------------------------

    static EstPValues collectPValuesCrossModel(int N, int numTrials, Params par,
        bool sampleModelIsA, double trueCh, double trueAdv,
        bool testModelIsA, int numUnknown, std::mt19937& rng, int mode = 0) {
        EstPValues res;
        res.simple.reserve(numTrials);
        res.onestep.reserve(numTrials);
        int df = chiDf(par.m, mode, numUnknown);
        requireDf(df);
        int os = 0, oo = 0, bs = 0, bo = 0, valid = 0;

        Params genPar = par;
        genPar.p_ch = trueCh;
        genPar.p_adv = trueAdv;

        Params testPar = par;
        testPar.p_adv = trueAdv;
        testPar.p_ch = trueCh;   // используется только при numUnknown == 0

        for (int t = 0; t < numTrials; t++) {
            Counts counts;
            Est s, o;
            if (!drawTrial(N, genPar, sampleModelIsA, testPar, testModelIsA, numUnknown, rng, counts, s, o)) {
                res.failed++;
                continue;
            }
            valid++;
            if (s.outside) os++;
            if (o.outside) oo++;

            Params ps = testPar, po = testPar;
            ps.p_ch = s.p_ch; ps.p_adv = s.p_adv;
            po.p_ch = o.p_ch; po.p_adv = o.p_adv;
            if (modelIsValid(ps, testModelIsA))
                res.simple.push_back(chiSquarePValue(chiSquareStat(counts, N, ps, testModelIsA, mode), df));
            else { res.simple.push_back(0.0); bs++; }
            if (modelIsValid(po, testModelIsA))
                res.onestep.push_back(chiSquarePValue(chiSquareStat(counts, N, po, testModelIsA, mode), df));
            else { res.onestep.push_back(0.0); bo++; }
        }
        double denom = valid > 0 ? valid : 1;
        res.outSimple = os / denom;
        res.outOnestep = oo / denom;
        res.badSimple = bs / denom;
        res.badOnestep = bo / denom;
        return res;
    }

    static void writePValuesCsv(const std::string& path, const std::vector<double>& pvals) {
        std::ofstream out(path.c_str());
        out << "pvalue\n";
        for (size_t i = 0; i < pvals.size(); ++i) out << pvals[i] << "\n";
    }

    static double rejectionRate(const std::vector<double>& pv, double alpha) {
        if (pv.empty()) return 0.0;
        int c = 0;
        for (size_t i = 0; i < pv.size(); ++i) if (pv[i] < alpha) c++;
        return static_cast<double>(c) / pv.size();
    }

    static void printRejectionTable(const std::string& name, const std::vector<double>& pv) {
        static const double alphas[6] = { 0.01, 0.02, 0.03, 0.04, 0.05, 0.10 };
        std::cout << std::setw(10) << name << " | ";
        for (int i = 0; i < 6; ++i) std::cout << std::fixed << std::setprecision(3) << rejectionRate(pv, alphas[i]) << "  ";
        std::cout << "\n";
    }

    static void printPowerTable(double padv, double pch, const std::vector<double>& pv) {
        static const double alphas[6] = { 0.01, 0.02, 0.03, 0.04, 0.05, 0.10 };
        std::cout << "Таблица: p_adv = " << padv << ", p_ch = " << pch
            << ", p_adv*(1-p_ch) = " << padv * (1.0 - pch) << "\n";
        std::cout << "  Мощность              | ";
        for (int i = 0; i < 6; ++i)
            std::cout << std::fixed << std::setprecision(3) << rejectionRate(pv, alphas[i]) << "  ";
        std::cout << "\n  Уровень значимости a  | ";
        for (int i = 0; i < 6; ++i)
            std::cout << std::fixed << std::setprecision(2) << alphas[i] << "   ";
        std::cout << "\n\n";
    }

    // ----------------------------------------------------------------------
    // ИССЛЕДОВАНИЕ МОЩНОСТИ В ЗАВИСИМОСТИ ОТ ВЕКТОРА p_i
    // Для каждого вектора p из pGrid и каждого режима таблицы:
    //   size      - доля отвержений при верной H0 (данные из А, проверяем А);
    //   power     - доля отвержений при альтернативе (данные из А, проверяем Б)
    //               при номинальном критическом значении chi2(df);
    //   powerCal  - мощность при КАЛИБРОВАННОМ пороге (реальный размер = alpha);
    //   minExp    - min N*pi_ij по всем ячейкам под А (условие применимости chi2).
    // Используются one-step p-value (при numUnknown = 0 они равны simple).
    // Результат печатается и пишется в power_vs_p.csv.
    // ----------------------------------------------------------------------

    static double lowerQuantile(std::vector<double> v, double q) {
        std::sort(v.begin(), v.end());
        size_t idx = static_cast<size_t>(std::floor(q * v.size()));
        if (idx >= v.size()) idx = v.size() - 1;
        return v[idx];
    }

    static void powerVsP(const Params& par0, const std::vector<std::vector<double> >& pGrid,
        const std::vector<int>& modes, int N, int trials, double alpha,
        int numUnknownAlt, double pch, double padv, std::mt19937& rng,
        const std::string& csvPath) {
        std::ofstream csv(csvPath.c_str());
        csv << "mode,p,size,power,power_calibrated,min_expected\n";
        std::cout << "\n########## Мощность в зависимости от p_i ##########\n";
        std::cout << "N = " << N << ", повторений = " << trials << ", alpha = " << alpha
            << ", p_ch = " << pch << ", p_adv = " << padv
            << ", неизвестных параметров: " << numUnknownAlt << "\n";
        if (numUnknownAlt == 2)
            std::cout << "ВНИМАНИЕ: при 2 неизвестных А и Б неразличимы, мощность = размер.\n";

        for (size_t mi = 0; mi < modes.size(); ++mi) {
            int mode = modes[mi];
            std::cout << "\n--- Режим: " << modeName(mode)
                << ", df = " << chiDf(par0.m, mode, numUnknownAlt) << " ---\n";
            std::cout << std::setw(24) << "p" << " | size   power  powerCal  minExp\n";
            for (size_t gi = 0; gi < pGrid.size(); ++gi) {
                const std::vector<double>& praw = pGrid[gi];
                Params par = par0;
                par.m = static_cast<int>(praw.size());
                par.p = praw;
                double sp = std::accumulate(par.p.begin(), par.p.end(), 0.0);
                for (size_t k = 0; k < par.p.size(); ++k) par.p[k] /= sp;
                par.p_ch = pch; par.p_adv = padv;

                std::string label;
                for (size_t i = 0; i < par.p.size(); i++) {
                    std::ostringstream os;
                    os << std::fixed << std::setprecision(2) << par.p[i];
                    label += (i ? "/" : "") + os.str();
                }

                if (chiDf(par.m, mode, numUnknownAlt) < 1) {
                    std::cout << std::setw(24) << label << " | df < 1, пропущено\n";
                    continue;
                }
                std::vector<double> pr = jointProbs(par, true);
                double minExp = 1e300;
                for (size_t k = 0; k < pr.size(); ++k) minExp = (std::min)(minExp, N * pr[k]);

                EstPValues h0 = collectPValuesCrossModel(N, trials, par, true, pch, padv,
                    true, numUnknownAlt, rng, mode);
                EstPValues h1 = collectPValuesCrossModel(N, trials, par, true, pch, padv,
                    false, numUnknownAlt, rng, mode);
                if (h0.onestep.empty() || h1.onestep.empty()) {
                    std::cout << std::setw(24) << label << " | нет успешных испытаний TSC\n";
                    continue;
                }
                double size = rejectionRate(h0.onestep, alpha);
                double power = rejectionRate(h1.onestep, alpha);
                double cutoff = lowerQuantile(h0.onestep, alpha);
                int c = 0;
                for (size_t k = 0; k < h1.onestep.size(); ++k) if (h1.onestep[k] < cutoff) c++;
                double powerCal = static_cast<double>(c) / h1.onestep.size();

                std::cout << std::setw(24) << label << " | " << std::fixed << std::setprecision(3)
                    << size << "  " << power << "  " << powerCal << "     "
                    << std::setprecision(1) << minExp << "\n";
                csv << mode << "," << label << "," << size << "," << power << ","
                    << powerCal << "," << minExp << "\n";
            }
        }
        std::cout << "\nТаблица сохранена в " << csvPath << "\n\n";
    }

    // ----------------------------------------------------------------------
    // ПРОВЕРКА ВЫБОРКИ TSC: таблица, суммы по строкам/столбцам (набл. | ожид.)
    // Строки должны соответствовать xi_1 (доли строк ~ p_i). Если это не так -
    // установите g_transposeSample = true.
    // ----------------------------------------------------------------------

    static void sanityCheckTsc(const Params& par, int N, std::mt19937& rng) {
        int m = par.m;
        for (int w = 0; w < 2; ++w) {
            bool isA = (w == 0);
            Counts c = generateSample(N, par, isA, rng);
            std::vector<double> pr = jointProbs(par, isA);
            std::cout << "Проверка выборки TSC, модель " << (isA ? "А" : "Б")
                << " (g_transposeSample = " << (g_transposeSample ? "true" : "false") << "):\n";
            for (int i = 0; i < m; i++) {
                for (int j = 0; j < m; j++) std::cout << std::setw(8) << c[i][j];
                std::cout << "\n";
            }
            std::cout << "  строки  (набл. | ожид.):";
            for (int i = 0; i < m; i++) {
                double o = 0.0, e = 0.0;
                for (int j = 0; j < m; j++) { o += c[i][j]; e += pr[i * m + j]; }
                std::cout << "  " << std::fixed << std::setprecision(3) << o / N << "|" << e;
            }
            std::cout << "\n  столбцы (набл. | ожид.):";
            for (int j = 0; j < m; j++) {
                double o = 0.0, e = 0.0;
                for (int i = 0; i < m; i++) { o += c[i][j]; e += pr[i * m + j]; }
                std::cout << "  " << std::fixed << std::setprecision(3) << o / N << "|" << e;
            }
            std::cout << "\n\n";
        }
    }

    // ----------------------------------------------------------------------
    // ЗАПУСК ЭКСПЕРИМЕНТОВ (бывшая main5)
    // ----------------------------------------------------------------------

    // ----------------------------------------------------------------------
    // Своя выборка: квадратная таблица m x m из файла, частоты - целые числа
    // через пробел (пустые строки игнорируются). Таблица применяется как есть,
    // без генерации TSC: сначала оценки TSC, потом критерий.
    // ----------------------------------------------------------------------

    // exe dir: relative data path is looked up there as well
    static std::string exeDir() {
#if defined(_WIN32)
        char buf[MAX_PATH];
        DWORD n = GetModuleFileNameA(NULL, buf, MAX_PATH);
        if (n > 0 && n < MAX_PATH) {
            std::string s(buf, n);
            size_t k = s.find_last_of("\\/");
            if (k != std::string::npos) return s.substr(0, k + 1);
        }
#endif
        return std::string();
    }
    static bool readCountsFile(const std::string& path, Counts& counts, int& m) {
        std::ifstream in(path.c_str());
        if (!in) {
            std::string alt = exeDir() + path;
            in.clear();
            in.open(alt.c_str());
        }
        if (!in) { std::cerr << "Не удалось открыть файл выборки: " << path << "\n"; return false; }
        std::vector<std::vector<double> > rows;
        std::string line;
        while (std::getline(in, line)) {
            std::istringstream ls(line);
            std::vector<double> r; double v;
            while (ls >> v) r.push_back(v);
            if (!r.empty()) rows.push_back(r);
        }
        int n = static_cast<int>(rows.size());
        if (n < 2) { std::cerr << "В файле нужно хотя бы две строки с числами\n"; return false; }
        for (int i = 0; i < n; ++i)
            if (static_cast<int>(rows[i].size()) != n) {
                std::cerr << "Таблица должна быть квадратной: строк " << n
                    << ", а в строке " << (i + 1) << " чисел " << rows[i].size() << "\n";
                return false;
            }
        m = n;
        counts.assign(m, std::vector<int>(m, 0));
        for (int i = 0; i < m; ++i)
            for (int j = 0; j < m; ++j) {
                if (rows[i][j] < 0.0) {
                    std::cerr << "Отрицательная частота в ячейке (" << i << "," << j << ")\n";
                    return false;
                }
                counts[i][j] = static_cast<int>(std::floor(rows[i][j] + 0.5));
            }
        return true;
    }

    // p_i по таблице: p_i = (сумма строки i) / N - ровно та оценка, что даёт TSC.
    static void pFromRows(const Counts& counts, int m, long N, std::vector<double>& p) {
        p.assign(m, 0.0);
        double sum = 0.0;
        for (int i = 0; i + 1 < m; ++i) {
            long r = 0;
            for (int j = 0; j < m; ++j) r += counts[i][j];
            p[i] = static_cast<double>(r) / static_cast<double>(N);
            sum += p[i];
        }
        p[m - 1] = 1.0 - sum;
    }

    // Векторы оценок TSC для готовой таблицы: [p_0 ... p_{m-2}, p_ch, p_adv].
    static bool tscEstimateAll(const Counts& counts, int m, bool testIsA,
        std::vector<double>& simple, std::vector<double>& onestep) {
        try {
            initDumper();
            MathModels::TLDotCacher ldotCacher;
            MathModels::TInfoCacher infoCacher;
            std::vector<double> flat(static_cast<size_t>(m) * m);
            for (int i = 0; i < m; ++i)
                for (int j = 0; j < m; ++j) flat[i * m + j] = counts[i][j];
            // Таблица должна жить вместе с моделью, поэтому не удаляем её (как в tscRun).
            Tsc::TableManager::CTableManager<double>* sampleManager =
                new Tsc::TableManager::CTableManager<double>(&flat.front(), m, m);
            GroupedTSC::IModelRep* rep;
            if (testIsA)
                rep = new GroupedTSC::CTSCModel_1Pos(m, ETSCModel, ldotCacher, infoCacher);
            else
                rep = new GroupedTSC::CTSCModel_1Pos_mine(m, ETSCModel, ldotCacher, infoCacher);
            std::auto_ptr<GroupedTSC::CModel> model(GroupedTSC::CModel::CreateCModel(
                m, sampleManager, false, rep));
            model->ProcessInitialCalculation(true);
            if (model->GetEstimateCount() != m + 1) {
                std::cerr << "TSC вернул " << model->GetEstimateCount()
                    << " оценок вместо m+1 = " << (m + 1) << "\n";
                return false;
            }
            simple.assign(m + 1, 0.0);
            for (int i = 0; i < m + 1; ++i) simple[i] = model->GetEstimateAt(i).second.Value;
            model->ProcessEstimatesCalculation();
            onestep.assign(m + 1, 0.0);
            for (int i = 0; i < m + 1; ++i) onestep[i] = model->GetEstimateAt(i).second.Value;
            return true;
        }
        catch (std::exception& e) {
            std::cout << "exception! " << e.what() << std::endl;
            return false;
        }
    }

    static void paramsFromTsc(const std::vector<double>& est, int m, Params& par) {
        par.m = m;
        par.p.assign(m, 0.0);
        double sum = 0.0;
        for (int i = 0; i + 1 < m; ++i) { par.p[i] = est[i]; sum += est[i]; }
        par.p[m - 1] = 1.0 - sum;
        par.p_ch = est[m - 1];
        par.p_adv = est[m];
    }

    static void fileSampleCriterion(const std::string& path, const Params& base,
        const std::vector<int>& modes, int numUnknown, bool testIsA, double alpha) {
        Counts counts;
        int m = 0;
        if (!readCountsFile(path, counts, m)) return;

        long total = 0;
        for (int i = 0; i < m; ++i)
            for (int j = 0; j < m; ++j) total += counts[i][j];
        if (total <= 0) { std::cerr << "Сумма частот в таблице равна нулю\n"; return; }
        int N = static_cast<int>(total);

        std::cout << "==================================================\n";
        std::cout << "Своя выборка из файла " << path << ": m = " << m << ", N = " << N
            << ", модель " << (testIsA ? "А" : "Б")
            << ", неизвестных параметров: " << numUnknown << "\n";
        for (int i = 0; i < m; ++i) {
            for (int j = 0; j < m; ++j) std::cout << std::setw(8) << counts[i][j];
            std::cout << "\n";
        }
        std::cout << "  суммы строк:";
        for (int i = 0; i < m; ++i) {
            long r = 0;
            for (int j = 0; j < m; ++j) r += counts[i][j];
            std::cout << std::setw(9) << r;
        }
        std::cout << "\n\n";

        Params ps, po;
        ps.m = m;
        pFromRows(counts, m, total, ps.p);
        po = ps;

        if (numUnknown == 2) {
            std::vector<double> sEst, oEst;
            if (!tscEstimateAll(counts, m, testIsA, sEst, oEst)) return;
            paramsFromTsc(sEst, m, ps);
            paramsFromTsc(oEst, m, po);
            std::cout << "Оценки TSC (простая / одношаговая):\n";
            for (int i = 0; i + 1 < m; ++i)
                std::cout << "  p" << i << " = " << std::fixed << std::setprecision(4)
                    << ps.p[i] << " / " << po.p[i] << "\n";
            std::cout << "  pch = " << ps.p_ch << " / " << po.p_ch << "\n";
            std::cout << "  padv = " << ps.p_adv << " / " << po.p_adv << "\n\n";
        }
        else {
            Params t = ps;
            t.p_ch = base.p_ch;
            t.p_adv = base.p_adv;
            Est es = estimateSimple(counts, N, t, testIsA, numUnknown);
            Params u = t;
            u.p_ch = es.p_ch;
            u.p_adv = es.p_adv;
            Est eo = oneStepUpdate(counts, N, u, es, testIsA, numUnknown);
            ps.p_ch = es.p_ch; ps.p_adv = es.p_adv;
            po.p_ch = eo.p_ch; po.p_adv = eo.p_adv;
            std::cout << "Оценки (простая / одношаговая): pch = " << std::fixed
                << std::setprecision(4) << ps.p_ch << " / " << po.p_ch
                << ", padv = " << ps.p_adv << " / " << po.p_adv << "\n\n";
        }

        const char* names[2] = { "simple  ", "one-step" };
        Params used[2];
        used[0] = ps;
        used[1] = po;

        for (size_t mi = 0; mi < modes.size(); ++mi) {
            int mode = modes[mi];
            int dfKnown = chiDf(m, mode, numUnknown);
            int dfAll = dfKnown - (m - 1);
            std::cout << "==================================================\n";
            std::cout << "Режим: " << modeName(mode) << ", df = " << dfAll
                << " (p_i тоже оценены по таблице)\n";
            std::cout << "==================================================\n";
            if (dfAll < 1) { std::cout << "df < 1: критерий не применим\n\n"; continue; }
            for (int k = 0; k < 2; ++k) {
                if (!modelIsValid(used[k], testIsA)) {
                    std::cout << "  " << names[k]
                        << " оценки дают неположительные вероятности: p-value не определён\n";
                    continue;
                }
                double stat = chiSquareStat(counts, N, used[k], testIsA, mode);
                double pv = chiSquarePValue(stat, dfAll);
                std::cout << "  " << names[k] << " chi2 = " << std::fixed
                    << std::setprecision(4) << stat
                    << ", p-value = " << std::setprecision(6) << pv
                    << ", alpha = " << std::setprecision(3) << alpha
                    << " -> " << (pv < alpha ? "отвергаем H0" : "не отвергаем H0") << "\n";
            }
            if (dfKnown >= 1)
                std::cout << "  (если p_i считать известными, df = " << dfKnown << ": simple = "
                    << chiSquarePValue(chiSquareStat(counts, N, ps, testIsA, mode), dfKnown)
                    << ", one-step = "
                    << chiSquarePValue(chiSquareStat(counts, N, po, testIsA, mode), dfKnown)
                    << ")\n";
            std::cout << "\n";
        }
    }


    static int run() {
        // Формулы TSC генерируются с десятичной точкой, поэтому C-локаль должна
        // оставаться "C": при "Russian" strtod/atof в формульном движке обрывают
        // число на точке (0.503 -> 0, 2.5 -> 2), и все оценки TSC становятся нулевыми.
        setlocale(LC_ALL, "C");
        // Исходники в cp1251 (= ACP), а консоль Windows по умолчанию OEM (866): без
        // SetConsoleOutputCP русские сообщения выводятся как мусор.
#if defined(_WIN32)
    {
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD dwMode = 0;
        if (hOut != INVALID_HANDLE_VALUE && hOut != NULL && GetConsoleMode(hOut, &dwMode))
            SetConsoleOutputCP(1251);
    }
#endif

        // ---------------- Настройки: меняйте здесь ----------------
        Params par;
        par.m = 3;
        par.p_ch = 0.2;
        par.p_adv = 0.05;
        par.p.clear();
        par.p.push_back(0.5); par.p.push_back(0.3); par.p.push_back(0.2);   // p_i ИЗВЕСТНЫ
        int N = 300;                 // объем одной выборки
        int numTrials = 1000;         // число повторений
        double alpha = 0.1;           // для блока "известные параметры"
        bool modelIsA = true;         // true: данные и H0 - модель А; false: модель Б
        int numUnknown = 2;           // 0: все известны; 1: неизвестен p_ch; 2: p_ch и p_adv (оценки TSC)
        int numUnknownAlt = 2;        // то же для блока мощности (0, 1 или 2; 2 бесполезно)
        bool fixedSeed = false;        // true: воспроизводимый результат
        bool sanityCheck = false;      // напечатать пробные выборки TSC для проверки ориентации таблицы
        g_transposeSample = false;    // true, если в таблице TSC строка = xi_2
        std::vector<int> modes;
        modes.push_back(0); modes.push_back(1);// modes.push_back(2);
        // --- проверка своей выборки из файла ---
        bool checkFileSample = true;          // true: проверить таблицу из файла
        bool onlyFileSample = true;          // true: после проверки файла не гонять моделирование
        std::string fileSamplePath = "DES95765Group.txt";   // файл с таблицей m x m (частоты через пробел)
        vector<double> alphas = { 0.01, 0.02, 0.03, 0.04, 0.05, 0.1 };
        // --- Исследование мощности в зависимости от p_i ---
        bool runPowerStudy = false;    // запускать ли исследование
        bool onlyPowerStudy = false;   // true: после него сразу выйти (пропустить остальные блоки)
        int psN = 300;                // объем выборки
        int psTrials = 5000;          // повторений на точку
        double psAlpha = 0.05;        // уровень значимости
        int psUnknown = 0;            // 0 или 1 (2 бессмысленно)
        double psPch = 0.2, psPadv = 0.05;   // параметры данных (истинные)
        std::vector<std::vector<double> > pGrid;
        // Семейство 1: p0 меняется, остальное делится 3:2
        const double p0s[7] = { 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8 };
        for (int k = 0; k < 7; ++k) {
            double p0 = p0s[k];
            std::vector<double> v;
            v.push_back(p0); v.push_back((1.0 - p0) * 0.6); v.push_back((1.0 - p0) * 0.4);
            pGrid.push_back(v);
        }
        // Семейство 2: p0 = 0.5, меняется доля p1 среди оставшихся
        const double ss[5] = { 0.5, 0.6, 0.7, 0.8, 0.9 };
        for (int k = 0; k < 5; ++k) {
            std::vector<double> v;
            v.push_back(0.5); v.push_back(0.5 * ss[k]); v.push_back(0.5 * (1.0 - ss[k]));
            pGrid.push_back(v);
        }
        // ------------------------------------------------------------

        if (numUnknown < 0 || numUnknown > 2 || numUnknownAlt < 0 || numUnknownAlt > 2) {
            std::cerr << "numUnknown и numUnknownAlt должны быть 0, 1 или 2\n";
            return 1;
        }

        double sumP = std::accumulate(par.p.begin(), par.p.end(), 0.0);
        for (size_t k = 0; k < par.p.size(); ++k) par.p[k] /= sumP;

        std::mt19937 rng(fixedSeed ? 12345u : std::random_device()());

        std::cout << "m = " << par.m << ", p_ch = " << par.p_ch << ", p_adv = " << par.p_adv
            << ", N = " << N << ", numTrials = " << numTrials << "\n";
        std::cout << "Данные и H0: модель " << (modelIsA ? "А" : "Б")
            << ", неизвестных параметров: " << numUnknown << "\n\n";
        //
        //
        if (sanityCheck) sanityCheckTsc(par, N, rng);
        if (checkFileSample) {
            fileSampleCriterion(fileSamplePath, par, modes, numUnknown, modelIsA, alpha);
            if (onlyFileSample) return 0;
        }

        // --- Ожидаемые частоты (проверка условия N*p_ij >= 5) ---
        {
            std::vector<double> pr = jointProbs(par, modelIsA);
            int nSmall = 0;
            double restMass = 0.0;
            std::cout << "Ожидаемые частоты N*p_ij:\n";
            for (int i = 0; i < par.m; i++) {
                for (int j = 0; j < par.m; j++) {
                    double e = N * pr[i * par.m + j];
                    if (e < 5.0) nSmall++;
                    if (!isInformative(i, j)) restMass += pr[i * par.m + j];
                    std::cout << std::setw(9) << std::fixed << std::setprecision(1) << e;
                }
                std::cout << "\n";
            }
            std::cout << "Ячеек с N*p_ij < 5: " << nSmall << "\n";
            std::cout << "Суммарная вероятность неинформативных ячеек (pi_rest): "
                << std::setprecision(3) << restMass << "\n\n";
        }

        if (runPowerStudy) {
            if (psUnknown < 0 || psUnknown > 2) { std::cerr << "psUnknown должен быть 0, 1 или 2\n"; return 1; }
            powerVsP(par, pGrid, modes, psN, psTrials, psAlpha, psUnknown, psPch, psPadv, rng,
                "power_vs_p.csv");
            if (onlyPowerStudy) return 0;
        }

        // --- Блоки по режимам таблицы ---
        for (size_t mi = 0; mi < modes.size(); ++mi) {
            int mode = modes[mi];
            std::cout << "==================================================\n";
            std::cout << "Режим: " << modeName(mode)
                << ", df (параметры известны) = " << chiDf(par.m, mode, 0)
                << ", df (оценены) = " << chiDf(par.m, mode, numUnknown) << "\n";
            std::cout << "==================================================\n\n";

            double typeI = empiricalRejectionRate(N, numTrials, par, modelIsA, modelIsA, alpha, rng, mode);
            double power = empiricalRejectionRate(N, numTrials, par, modelIsA, !modelIsA, alpha, rng, mode);
            std::cout << "Параметры известны, alpha = " << alpha << ":\n";
            std::cout << "  уровень значимости (H0 верна): " << typeI << "\n";
            std::cout << "  мощность против другой модели: " << power << "\n\n";

            std::ostringstream modeStr;
            modeStr << mode;
            EstPValues pv = collectPValuesEstimated(N, numTrials, par, modelIsA, numUnknown, rng, mode);
            writePValuesCsv("pvalues_simple_mode" + modeStr.str() + ".csv", pv.simple);
            writePValuesCsv("pvalues_onestep_mode" + modeStr.str() + ".csv", pv.onestep);
            std::cout << "Доли отвержений верной H0 (df = " << chiDf(par.m, mode, numUnknown) << "):\n";
            std::cout << "     alpha | 0.010  0.020  0.030  0.040  0.050  0.100\n";
            printRejectionTable("simple", pv.simple);
            printRejectionTable("one-step", pv.onestep);
            std::cout << "Обрезания НЕТ. Доля выборок с оценкой вне [0,1]: simple = "
                << pv.outSimple << ", one-step = " << pv.outOnestep << "\n";
            std::cout << "Доля выборок, где pi_ij <= 0 (p-value:=0): simple = "
                << pv.badSimple << ", one-step = " << pv.badOnestep << "\n";
            if (pv.failed > 0) std::cout << "Пропущено из-за сбоев TSC: " << pv.failed << "\n";
            std::cout << "p-value сохранены в pvalues_simple_mode" << mode
                << ".csv и pvalues_onestep_mode" << mode << ".csv\n\n";

            // Мощность против альтернативной модели
            {
                std::cout << "--- Мощность против альтернативы, неизвестных параметров: "
                    << numUnknownAlt << " (" << modeName(mode) << ") ---\n";
                int powN = 300;
                int powTrials = 1000;
                if (chiDf(par.m, mode, numUnknownAlt) < 1) {
                    std::cout << "df < 1, пропущено\n\n";
                    continue;
                }
                std::vector<std::pair<double, double> > grid;
                //grid.push_back(std::make_pair(0.4, 0.6)); grid.push_back(std::make_pair(0.2, 0.8));
                //grid.push_back(std::make_pair(0.1, 0.9)); grid.push_back(std::make_pair(0.05, 0.95));
                grid.push_back(std::make_pair(0.05, 0.2));
                for (size_t gi = 0; gi < grid.size(); ++gi) {
                    double padv = grid[gi].first, pch = grid[gi].second;
                    EstPValues cross = collectPValuesCrossModel(powN, powTrials, par,
                        /*sampleModelIsA=*/true, pch, padv,
                        /*testModelIsA=*/false, numUnknownAlt, rng, mode);
                    std::cout << "-- one-step оценка --\n";
                    printPowerTable(padv, pch, cross.onestep);
                    std::cout << "-- простая оценка --\n";
                    printPowerTable(padv, pch, cross.simple);
                    std::cout << "доля оценок вне [0,1]: simple=" << cross.outSimple
                        << ", one-step=" << cross.outOnestep
                        << "; доля pi_ij<=0: simple=" << cross.badSimple
                        << ", one-step=" << cross.badOnestep;
                    if (cross.failed > 0) std::cout << "; сбоев TSC: " << cross.failed;
                    std::cout << "\n\n";
                }
            }
        }
        return 0;
    }

} // namespace chi2test

int main()
{
    return chi2test::run();
    // return mainCompareEstimates();   // ... main
}