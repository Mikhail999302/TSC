

#include <windows.h>
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
#include "..\ECalc\BaseDiscreteModel.h"
#include "..\ECalc\discretemodel.h"
#include "..\ECalc\DataParameters.h"
#include "..\MathCalc\MathCalcMisc.h"

namespace chi2test {

    typedef std::vector<std::vector<int> > Counts;

    // ----------------------------------------------------------------------
    // Параметры
    // ----------------------------------------------------------------------

    struct Params {
        int m;                  // размер категории
        double p_ch;
        double p_adv;
        std::vector<double> p;  // p_i = P(xi_1 = i), известны
    };

    struct Est {
        std::vector<double> p;  // p_i = P(xi_1 = i), p[m-1] = 1 - sum(p_i)
        double p_ch;
        double p_adv;
        bool outside;  // true, если вне [0, 1]
    };

    // Лежит ли v вне интервала
    static bool outside01(double v) { return v < 0.0 || v > 1.0; }

    // Есть ли среди оценённых параметров значение вне (0, 1)?
    static bool outsideEst(const Est& e)
    {
        bool bad = outside01(e.p_ch) || outside01(e.p_adv);
        for (size_t i = 0; i + 1 < e.p.size(); ++i)
            if (outside01(e.p[i])) bad = true;
        return bad;
    }
    //небольшой чек оценки
    static void estFromTsc(const std::vector<double>& all, int m, Est& e)
    {
        if (static_cast<int>(all.size()) < m + 1) { e.outside = true; return; }
        e.p.assign(m, 0.0);
        double sum = 0.0;
        for (int i = 0; i + 1 < m; ++i) { e.p[i] = all[i]; sum += all[i]; }
        e.p[m - 1] = 1.0 - sum;
        e.p_ch = all[m - 1];
        e.p_adv = all[m];
        e.outside = outsideEst(e);
    }

    // true, если категории TSC заданы наоборот (xi_2 = xi_1)
    static bool g_transposeSample = false;
    static int g_tscFailed = 0;

    static bool g_printTscHeader = false;

    //удобно чтобы брать модель из TSC
    class TscModel
    {
    public:
        TscModel(int m, bool isA)
            : m_m(m)
            , m_isA(isA)
            , m_mapId(MathCalc::MathContext::MapIDType(0))
            , m_comp(0, 0)
            , m_ldot()
            , m_info()
            , m_rep()
            , m_states()
            , m_se()
            , m_paramInfos()
            , m_model()
        {
            initDumper();

            m_mapId = MathCalc::MathContext::AddMap();
            MathCalc::MathContext::SelectMap(m_mapId);

            try {
                if (m_isA)
                    m_rep.reset(new GroupedTSC::CTSCModel_1Pos(m, ETSCModel, m_ldot, m_info));
                else
                    m_rep.reset(new GroupedTSC::CTSCModel_1Pos_mine(m, ETSCModel, m_ldot, m_info));

                // Описание состояний (модель) и формул оценок
                std::auto_ptr<GroupedTSC::TModelManager> modelDef(
                    m_rep->CreateModelManager(m, m));
                const MathCalc::MathString modelStr = m_rep->SetModelStrRep(*modelDef);
                const MathCalc::MathString estStr = m_rep->SetEstimatesFormulasStrRep(m);

                DataProcessing::ReadExpressions(modelStr, m_states, _T("p"));
                DataProcessing::ReadEstimates(estStr, m_se);

                const size_t n = static_cast<size_t>(m) + 1;
                if (m_se.size() != n) {
                    std::cerr << "TSC: " << m_se.size() << " формул оценок, ожидалось m+1 = "
                        << n << "\n";
                    throw std::runtime_error("TSC estimates count");
                }

                // Кэшеры строят символические выражения l_dot и InfoMatrix
                // при первом обращении, поэтому начальные значения
                // должны быть корректными.
                m_paramInfos = MathModels::CMathParameterInfos(m_se);

                static bool printedNames = false;
                if (!printedNames && g_printTscHeader) {
                    printedNames = true;
                    std::cout << "Оценки TSC (библиотечные формулы); "
                        << "p_ch = [" << (m - 1) << "], p_adv = [" << m << "]:\n";
                    for (size_t i = 0; i < n; ++i)
                        std::cout << "  [" << i << "] " << m_se[i]->GetParameterName() << "\n";
                    std::cout << "\n";
                }

                m_model = MathModels::CreateModel(MathModels::CBaseDiscreteModel::mt2D);
                m_model->Init(m_states, m_paramInfos, m_comp,
                    m_ldot, m_info, MathModels::CMathOperationIndicator(),
                    /*_isCalcInfoMatrix=*/true, /*_isBMatrix=*/false);

                {
                    m_dummy.assign(static_cast<size_t>(m) * m, 1.0);
                    m_dummyN = m * m;
                    MathModels::CMathParameterValues est;
                    if (!m_model->CalcSimpleEstimates(m_dummy, m_dummyN, m_se, est))
                        throw std::runtime_error("TSC simple estimates failed");
                    if (est.size() != n)
                        throw std::runtime_error("TSC parameters count");
                    est.SetContextValues();
                }
            }
            catch (...) {
                MathCalc::MathContext::RemoveMap(m_mapId);
                m_mapId = MathCalc::MathContext::MapIDType(0);
                throw;
            }
        }

        ~TscModel()
        {
            m_model.release();
            m_rep.reset();
            MathCalc::MathContext::RemoveMap(m_mapId);
        }

        // Размерность m, для которой собрана модель.
        int dim() const { return m_m; }
        // ... ... ... ....... df, ... TSCCalc (model.cpp:268):
        //   df = GetStatesCount() - GetNANParametersCount() - 1
        int libraryParamCount() const { return m_model->GetNANParametersCount(); }

        //в библиотеке куча try catch поэтому возвращает bool, а так это для подсчета оценок
        bool jointProbs(const std::vector<double>& counts, int n,
            const std::vector<double>& p, double pCh, double pAdv,
            std::vector<double>& probs) const
        {
            const size_t np = static_cast<size_t>(m_m) + 1;
            if (p.size() < static_cast<size_t>(m_m) - 1) return false;
            if (counts.size() != static_cast<size_t>(m_m) * m_m) return false;

            MathCalc::MathContext::SelectMap(m_mapId);

            MathModels::CMathParameterValues est;
            if (!m_model->CalcSimpleEstimates(counts, n, m_se, est)) return false;
            if (est.size() != np) return false;
            for (int i = 0; i + 1 < m_m; ++i) est[i].Value = p[i];
            est[m_m - 1].Value = pCh;
            est[m_m].Value = pAdv;

            probs.clear();
            return m_model->CalcFrequencies(est, 1, probs);
        }

        //перестраховка
        bool jointProbs(const std::vector<double>& p, double pCh, double pAdv,
            std::vector<double>& probs) const
        {
            return jointProbs(m_dummy, m_dummyN, p, pCh, pAdv, probs);
        }

        //тоже для оценок
        bool estimates(const std::vector<double>& counts, int N,
            std::vector<double>& simple, std::vector<double>& onestep) const
        {
            const size_t n = static_cast<size_t>(m_m) + 1;
            if (counts.size() != static_cast<size_t>(m_m) * m_m) return false;

            MathCalc::MathContext::SelectMap(m_mapId);

            MathModels::CMathParameterValues est;

            if (!m_model->CalcSimpleEstimates(counts, N, m_se, est)) return false;
            if (est.size() != n) return false;


            simple.assign(n, 0.0);
            for (size_t i = 0; i < n; ++i) simple[i] = est[i].Value;

           
            if (!m_model->CalcEnhancedEstimates(counts, N, m_se, est, 0.0)) return false;
            if (est.size() != n) return false;
            onestep.assign(n, 0.0);
            for (size_t i = 0; i < n; ++i) onestep[i] = est[i].Value;

            return true;
        }


    private:

        int m_m;
        bool m_isA;
        MathCalc::MathContext::MapIDType m_mapId;
        MathModels::CCacherComparingStruct m_comp;


        MathModels::TLDotCacher m_ldot;
        MathModels::TInfoCacher m_info;
        std::auto_ptr<GroupedTSC::IModelRep> m_rep;
        MathModels::CExpressions m_states;
        std::vector<double> m_dummy;
        int m_dummyN = 0;
        MathModels::CMathParameterEstimates m_se;
        MathModels::CMathParameterInfos m_paramInfos;
        MathModels::ModelPtr m_model;
    };

    // Две модели (А и Б) создаются один раз и переиспользуются.
    static std::auto_ptr<TscModel> g_modelA;
    static std::auto_ptr<TscModel> g_modelB;

    // Обёртка над моделью TSCCalc для m и признака model1 (А = true, Б = false).
    static TscModel& theModel(int m, bool isA)
    {
        std::auto_ptr<TscModel>& slot = isA ? g_modelA : g_modelB;
        const int cached = slot.get() ? slot->dim() : 0;
        if (!slot.get() || cached != m) {
            slot.reset();
            slot.reset(new TscModel(m, isA));
        }
        return *slot;
    }

    // Модель А или Б выбирается флагом; модель кэшируется и пересоздаётся
    // только при смене m. Сами вероятности считает CalcFrequencies; результат - вектор m*m.
    static std::vector<double> jointProbs(const Params& par, bool model1)
    {
        std::vector<double> probs;
        theModel(par.m, model1).jointProbs(par.p, par.p_ch, par.p_adv, probs);
        return probs;
    }


    // Генерация TSC: библиотека

    // Параметры генератора: p_0..p_{m-1}, p_ch, p_adv
    static TSCGenerator::doubles makeDistrib(const Params& par)
    {
        TSCGenerator::doubles d;
        for (size_t i = 0; i + 1 < par.p.size(); ++i) d.push_back(par.p[i]);
        d.push_back(par.p_ch);
        d.push_back(par.p_adv);
        return d;
    }

    // Из плоского sample m*m -> counts[xi_1][xi_2]
    static void tableFromSample(const doubles& sample, int m, int N, Counts& counts)
    {
        if (static_cast<int>(sample.size()) != m * m) {
            std::cerr << "Размер выборки TSC = " << sample.size()
                << ", ожидалось m*m = " << m * m << "\n";
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
            std::cerr << "Сумма частот выборки TSC = " << total << " != N = " << N
                << ": генератор TSC не согласован?\n";
            std::exit(1);
        }
    }

    // Генерация выборки TSC заданной модели (Gen = CGenerator_1Pos / _mine).
    template <class Gen>
    static bool tscGenerate(const Params& genPar, int N, long seed, Counts& counts)
    {
        try {
            TSCGenerator::doubles distrib = makeDistrib(genPar);
            std::auto_ptr<Gen> generator(new Gen(distrib, seed));
            generator->SetSize(N);
            generator->Generate();
            doubles sample = generator->GetSample();
            tableFromSample(sample, genPar.m, N, counts);
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

    // Следующий seed для генератора
    static long nextSeed(std::mt19937& rng)
    {
        return static_cast<long>(rng() & 0x7fffffffu);
    }

    // Генерирует таблицу частот размера N для заданной модели.
    static Counts generateSample(int N, const Params& par, bool fromModel1, std::mt19937& rng)
    {
        Counts counts;
        long seed = nextSeed(rng);
        bool ok = fromModel1
            ? tscGenerate<TSCGenerator::CGenerator_1Pos>(par, N, seed, counts)
            : tscGenerate<TSCGenerator::CGenerator_1Pos_mine>(par, N, seed, counts);
        if (!ok) { std::cerr << "Не удалось сгенерировать выборку TSC\n"; std::exit(1); }
        return counts;
    }

    // Таблица частот counts -> плоский sample для библиотечных оценок.
    static std::vector<double> flatten(const Counts& counts, int m)
    {
        std::vector<double> flat(static_cast<size_t>(m) * m, 0.0);
        for (int i = 0; i < m; ++i)
            for (int j = 0; j < m; ++j)
                flat[i * m + j] = static_cast<double>(counts[i][j]);
        return flat;
    }

    //это то самое место где пришлось реализовать оценки p_i
    struct KernelABC { double a, b, d; };

    // Параметры ядра p(xi_2 = j | xi_1 = i): a, b = p_ch, d = p_adv.
    static KernelABC kernelABC(bool model1, double p_ch, double p_adv)
    {
        KernelABC k;
        if (model1) {
            k.a = (1.0 - p_adv) * p_ch;
            k.b = p_adv;
            k.d = (1.0 - p_adv) * (1.0 - p_ch);
        }
        else {
            k.a = p_ch * (1.0 - p_adv);
            k.b = p_ch * p_adv;
            k.d = 1.0 - p_ch;
        }
        return k;
    }

    // Логарифм правдоподобия 
    static double logLik(const Counts& counts, const std::vector<double>& r, int m,
        const KernelABC& k, const std::vector<double>& p)
    {
        double L = 0.0;
        for (int i = 0; i < m; ++i) {
            if (p[i] <= 0.0) return -1e300;
            if (r[i] != 0) L += r[i] * std::log(p[i]);
        }
        for (int i = 0; i < m; ++i)
            for (int l = 0; l < m; ++l) {
                const double c = k.a * p[l] + (l == 0 ? k.b : 0.0) + (l == i ? k.d : 0.0);
                if (c <= 0.0) return -1e300;
                if (counts[i][l] != 0) L += counts[i][l] * std::log(c);
            }
        return L;
    }

    // Шаг Ньютона
    static bool newtonP(const Counts& counts, int N, int m, bool model1,
        double p_ch, double p_adv, std::vector<double>& p, int maxIter)
    {
        if (static_cast<int>(p.size()) != m || m < 2) return false;
        const KernelABC k = kernelABC(model1, p_ch, p_adv);
        const int f = m - 1;
        std::vector<double> r(m, 0.0), c(m * m), gp(m, 0.0), D(m, 0.0);
        std::vector<double> g(f), w(f), dir(f), trial(m);
        for (int i = 0; i < m; ++i)
            for (int j = 0; j < m; ++j) r[i] += counts[i][j];
        for (int it = 0; it < maxIter; ++it) {
            for (int i = 0; i < m; ++i)
                for (int j = 0; j < m; ++j)
                    c[i * m + j] = k.a * p[j] + (j == 0 ? k.b : 0.0) + (j == i ? k.d : 0.0);
            bool bad = false;
            for (int l = 0; l < m; ++l) {
                const double cll = c[l * m + l];
                if (p[l] <= 0.0 || cll <= 0.0) { bad = true; break; }
                double t = 0.0;
                for (int i = 0; i < m; ++i) t += counts[i][l] / c[i * m + l];
                gp[l] = r[l] / p[l] + k.a * t;
                D[l] = r[l] / (p[l] * p[l]) + k.a * k.a * counts[l][l] / (cll * cll);
            }
            if (bad) break;
            double S = 0.0, wg = 0.0;
            for (int t = 0; t < f; ++t) {
                if (D[t] <= 0.0) { bad = true; break; }
                g[t] = gp[t] - gp[m - 1];
                w[t] = 1.0 / D[t];
                S += w[t];
                wg += w[t] * g[t];
            }
            if (bad) break;
            const double den = 1.0 + D[m - 1] * S;
            if (den <= 0.0) break;
            const double c0 = D[m - 1] / den;
            for (int t = 0; t < f; ++t) dir[t] = w[t] * g[t] - c0 * w[t] * wg;
            const double L0 = logLik(counts, r, m, k, p);
            bool moved = false;
            double alpha = 1.0;
            for (int bt = 0; bt < 24; ++bt) {
                double sum = 0.0;
                bool ok = true;
                for (int t = 0; t < f; ++t) {
                    trial[t] = p[t] + alpha * dir[t];
                    if (trial[t] <= 0.0 || trial[t] >= 1.0) { ok = false; break; }
                    sum += trial[t];
                }
                if (ok && sum < 1.0) {
                    trial[f] = 1.0 - sum;
                    if (logLik(counts, r, m, k, trial) > L0) { moved = true; break; }
                }
                alpha *= 0.5;
            }
            if (!moved) break;
            double shift = 0.0;
            for (int t = 0; t < f; ++t) {
                const double dt = trial[t] - p[t];
                const double ad = dt < 0.0 ? -dt : dt;
                if (ad > shift) shift = ad;
            }
            p = trial;
            if (shift < 1e-13) break;
        }
        (void)N;
        for (int i = 0; i < m; ++i) if (p[i] <= 0.0 || p[i] >= 1.0) return false;
        return true;
    }

    // Оценки TSC из библиотеки (simple и one-step); известные p_ch/p_adv подставляются по numUnknown.
    static bool libraryEstimates(const Counts& counts, int N, const Params& testPar,
        bool testIsA, int numUnknown, Est& s, Est& o)
    {
        const int m = testPar.m;
        s.p.assign(m, 0.0);
        o.p.assign(m, 0.0);
        std::vector<double> sAll, oAll;
        if (!theModel(m, testIsA).estimates(flatten(counts, m), N, sAll, oAll)) return false;
        estFromTsc(sAll, m, s);
        estFromTsc(oAll, m, o);
        if (numUnknown < 1) { s.p_ch = testPar.p_ch; o.p_ch = testPar.p_ch; }
        if (numUnknown < 2) { s.p_adv = testPar.p_adv; o.p_adv = testPar.p_adv; }
        if (numUnknown == 0) {
            // p_ch, p_adv are known: p_0..p_{m-2} must be maximised of the
            // log-likelihood with p_ch, p_adv held fixed (library's
            // CalcEnhancedEstimates steps all four parameters jointly instead).
            o.p = s.p;
            if (!newtonP(counts, N, m, testIsA, s.p_ch, s.p_adv, o.p, 50)) o.p = s.p;
        }
        s.outside = outsideEst(s);
        o.outside = outsideEst(o);
        return true;
    }
    //количество оцениваемых параметров
    static int numEstimatedFor(int m, bool isA, int numUnknown)
    {
        const int nAll = theModel(m, isA).libraryParamCount();
        return (nAll - 2) + numUnknown;
    }

    // Критерий: группировка и X^2 / p-value (PLevel::CPLevel)

    // Информативная ячейка: i != 0 и (j == 0 или i == j).
    static bool isInformative(int i, int j)
    {
        return i != 0 && (j == 0 || i == j);
    }

    // Все неинформативные ячейки в группу 0, информативные - отдельно.
    template <class ReturnType = unsigned int, class ValueType = double>
    class MergeNonInformative : public Tsc::Grouping::IMergeGrouping<ReturnType, ValueType>
    {
        typedef std::vector<ReturnType> ReturnTypes;
    public:
        MergeNonInformative(int identificator = 0)
            : Tsc::Grouping::IMergeGrouping<ReturnType, ValueType>(identificator) {}
        virtual ReturnTypes operator()()
        {
            const int cells = static_cast<int>(this->states.size());
            const int m = static_cast<int>(std::floor(std::sqrt(static_cast<double>(cells)) + 0.5));
            ReturnTypes res(cells, 0);
            ReturnType next = 1;
            for (int i = 0; i < m; ++i)
                for (int j = 0; j < m; ++j)
                    if (isInformative(i, j))
                        res[i * m + j] = next++;
            return res;
        }
    };

    // Режимы группировки:
    //   0: без группировки, все ячейки   - Tsc::Grouping::NoneGroup
    //   1: объединение неинформативных ячеек - MergeNonInformative (одна категория) это моя реализация, но сказали без самодеятельности
    //   2: StandardGroup                  - Tsc::Grouping::StandardGroup
    //   3: AutoGroup                      - Tsc::Grouping::AutoGroup (число групп зависит от данных)
    //   4: ShrinkGroup                    - Tsc::Grouping::ShrinkGroup
    static const int kModeCount = 5;
    static const int kDfDynamic = -1;   // AutoGroup: число групп вычисляется по выборке

    // Название режима группировки по его номеру.
    static const char* modeName(int mode)
    {
        static const char* names[kModeCount] = {
            "без группировки", "объединение неинформативных",
            "StandardGroup", "AutoGroup", "ShrinkGroup" };
        return (mode >= 0 && mode < kModeCount) ? names[mode] : "?";
    }

    // df = (число групп) - 1 - (число оцениваемых параметров).
    // Для AutoGroup число групп вычисляется по данным = kDfDynamic.
    // с первого взгляда есть в TSC, но это было сделано из за проблем со shrinkgroup
    static int chiDf(int m, int mode, int numEstimated)
    {
        int cells = 0;
        switch (mode) {
        case 0: cells = m * m; break;            // NoneGroup
        case 1: cells = 2 * (m - 1) + 1; break;  // MergeNonInformative
        case 2: cells = m + 3; break;            // StandardGroup
        case 4: cells = 3; break;                // ShrinkGroup
        default: return kDfDynamic;              // AutoGroup
        }
        return cells - 1 - numEstimated;
    }

    // Строка описания группировки и df для вывода.
    static std::string dfStr(int m, int mode, int numEstimated)
    {
        const int df = chiDf(m, mode, numEstimated);
        if (df == kDfDynamic) return "dynamic";
        std::ostringstream os;
        os << df;
        return os.str();
        return os.str();
    }


    // Если df < 1 (кроме динамического) — сообщение об ошибке и выход.
    static void requireDf(int df)
    {
        if (df != kDfDynamic && df < 1) {
            std::cerr << "df < 1 (" << df
                << "): проверьте mode/m/numUnknown, параметры некорректны\n";
            std::exit(1);
        }
    }

    struct ChiResult {
        double stat;      // X^2
        double plevel;    // p-value
        double minNPi;    // минимальное N*pi_ij
        int df;
        bool defined;     // false, если p-value не определено
    };

    // Критерий: группировка состояний и вычисление критерия на полученных группах.
    // Значение X^2 и p-value считает TSCPlevel::CPLevel.
    //
    // df вычисляет библиотека: CPLevel::CalcChiValue(empiric) без df даёт
    // df = (число групп) - 1; из неё вычитается число оцениваемых параметров,
    // и получается df для расчёта p-value.
    static ChiResult chiSquare(const Counts& counts, int N, const Params& par,
        bool testModel1, int mode, int numEstimated)
    {
        ChiResult r;
        r.stat = 0.0; r.plevel = 0.0; r.minNPi = 0.0; r.df = 0; r.defined = false;

        const int m = par.m;
        const std::vector<double> allProbs = jointProbs(par, testModel1);
        if (allProbs.size() != static_cast<size_t>(m) * m) return r;

        std::vector<double> theoretic, empiric;
        theoretic.reserve(m * m);
        empiric.reserve(m * m);
        for (int i = 0; i < m; i++) {
            for (int j = 0; j < m; j++) {
                theoretic.push_back(allProbs[i * m + j]);
                empiric.push_back(static_cast<double>(counts[i][j]));
            }
        }

        PLevel::CPLevel chi;   // CPLevel::Init задаёт стратегию, CalcChiValue считает X^2
        try {
            if (mode == 1) {
                MergeNonInformative<PLevel::uint, double> strategy;
                chi.Init(theoretic, N, strategy);
            }
            else if (mode == 2) {
                Tsc::Grouping::StandardGroup<PLevel::uint, double> strategy;
                chi.Init(theoretic, N, strategy);
            }
            else if (mode == 3) {
                Tsc::Grouping::AutoGroup<PLevel::uint, double> strategy;
                chi.Init(theoretic, N, strategy);
            }
            else if (mode == 4) {
                Tsc::Grouping::ShrinkGroup<PLevel::uint, double> strategy;
                // это для shrink
                strategy.setSize(N);
                strategy.setStates(theoretic);
                strategy.setLevel(5.0);
                strategy.setMinSize(1);
                std::vector<PLevel::uint> shrink = strategy();
                if (static_cast<int>(shrink.size()) != m * m) {
                    static bool warned = false;
                    if (!warned) {
                        warned = true;
                        std::cerr << "!!!!!!!!!";
                    }
                    return r;
                }
                chi.Init(theoretic, N, strategy);
            }
            else {
                Tsc::Grouping::NoneGroup<PLevel::uint, double> strategy;
                chi.Init(theoretic, N, strategy);
            }

            // Первое вычисление: библиотека выдаёт df = (число групп) - 1.
            chi.CalcChiValue(empiric, /*isCheckSize=*/false);
            const int dfAll = chi.GetPLevelData().df;
            const int df = dfAll - numEstimated;
            r.df = df;
            if (df < 1) return r;              // df < 1: критерий не определён

            // Второе вычисление: df уже окончательный, считается p-value.
            chi.CalcChiValue(empiric, df, /*isCheckSize=*/false);
        }
        catch (std::exception&) {
            return r;               // df < 1 или нехватка данных
        }

        const PLevel::PLevelData& data = chi.GetPLevelData();
        r.stat = data.criterion_value;
        r.plevel = data.plevel;
        r.minNPi = data.minNPi;
        r.df = data.df;
        r.defined = (data.plevel >= 0.0);
        return r;
    }

    // Все ли совместные вероятности модели строго положительны?
    static bool modelIsValid(const Params& par, bool model1)
    {
        const std::vector<double> probs = jointProbs(par, model1);
        if (probs.size() != static_cast<size_t>(par.m) * par.m) return false;
        for (size_t k = 0; k < probs.size(); ++k)
            if (!(probs[k] > 0.0)) return false;
        return true;
    }

    struct RejRates { double simple; double onestep; };

    // название говорит за себя
    static RejRates empiricalRejectionRate(int N, int numTrials, const Params& par,
        bool sampleFromModel1, bool testModel1, double alpha, std::mt19937& rng,
        int numUnknown = 0, int mode = 0)
    {
        RejRates r; r.simple = 0.0; r.onestep = 0.0;
        int rejS = 0, rejO = 0, total = 0;
        const int numEstAll = numEstimatedFor(par.m, testModel1, numUnknown);
        requireDf(chiDf(par.m, mode, numEstAll));
        for (int t = 0; t < numTrials; t++) {
            Counts counts = generateSample(N, par, sampleFromModel1, rng);
            Est s, o;
            if (!libraryEstimates(counts, N, par, testModel1, numUnknown, s, o)) continue;
            total++;
            Params ps = par, po = par;
            ps.p = s.p; ps.p_ch = s.p_ch; ps.p_adv = s.p_adv;
            po.p = o.p; po.p_ch = o.p_ch; po.p_adv = o.p_adv;
            if (modelIsValid(ps, testModel1)) {
                ChiResult cr = chiSquare(counts, N, ps, testModel1, mode, numEstAll);
                if (cr.defined && cr.plevel < alpha) rejS++;
            }
            if (modelIsValid(po, testModel1)) {
                ChiResult cr = chiSquare(counts, N, po, testModel1, mode, numEstAll);
                if (cr.defined && cr.plevel < alpha) rejO++;
            }
        }
        if (total > 0) {
            r.simple = static_cast<double>(rejS) / total;
            r.onestep = static_cast<double>(rejO) / total;
        }
        return r;
    }

    // Одна выборка: генерация, оценки библиотекой и p-value для обеих оценок.
    static bool drawTrial(int N, const Params& genPar, bool genIsA,
        const Params& testPar, bool testIsA, int k, std::mt19937& rng,
        Counts& counts, Est& s, Est& o)
    {
        long seed = nextSeed(rng);
        bool ok = genIsA
            ? tscGenerate<TSCGenerator::CGenerator_1Pos>(genPar, N, seed, counts)
            : tscGenerate<TSCGenerator::CGenerator_1Pos_mine>(genPar, N, seed, counts);
        if (!ok) return false;
        return libraryEstimates(counts, N, testPar, testIsA, k, s, o);
    }

    // p-value при оценке параметров по выборке (H0 верна)

    struct EstPValues {
        std::vector<double> simple, onestep;
        double outSimple = 0.0, outOnestep = 0.0;   // доля оценок вне [0,1]
        double badSimple = 0.0, badOnestep = 0.0;   // доля невырожденных случаев
        int failed = 0;                             // ошибки TSC / оценок
    };


    // Собирает p-value по numTrials выборкам при оцениваемых из данных параметрах.
    static EstPValues collectPValuesEstimated(int N, int numTrials, const Params& par,
        bool modelIsA, int numUnknown, std::mt19937& rng, int mode = 0)
    {
        EstPValues res;
        res.simple.reserve(numTrials);
        res.onestep.reserve(numTrials);
        const int numEstAll = numEstimatedFor(par.m, modelIsA, numUnknown);
        requireDf(chiDf(par.m, mode, numEstAll));
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
            Params ps = par, po = par;  
            ps.p = s.p; ps.p_ch = s.p_ch; ps.p_adv = s.p_adv;
            po.p = o.p; po.p_ch = o.p_ch; po.p_adv = o.p_adv;
            if (modelIsValid(ps, modelIsA)) {
                ChiResult cr = chiSquare(counts, N, ps, modelIsA, mode, numEstAll);
                res.simple.push_back(cr.defined ? cr.plevel : 0.0);
                if (!cr.defined) bs++;
            }
            else { res.simple.push_back(0.0); bs++; }
            if (modelIsValid(po, modelIsA)) {
                ChiResult cr = chiSquare(counts, N, po, modelIsA, mode, numEstAll);
                res.onestep.push_back(cr.defined ? cr.plevel : 0.0);
                if (!cr.defined) bo++;
            }
            else { res.onestep.push_back(0.0); bo++; }
        }
        double denom = valid > 0 ? valid : 1;
        res.outSimple = os / denom;
        res.outOnestep = oo / denom;
        res.badSimple = bs / denom;
        res.badOnestep = bo / denom;
        return res;
    }

    // Собирает p-value для выборок модели sampleModelIsA, проверяемых моделью testModelIsA.
    static EstPValues collectPValuesCrossModel(int N, int numTrials, Params par,
        bool sampleModelIsA, double trueCh, double trueAdv,
        bool testModelIsA, int numUnknown, std::mt19937& rng, int mode = 0)
    {
        EstPValues res;
        res.simple.reserve(numTrials);
        res.onestep.reserve(numTrials);
        const int numEstAll = numEstimatedFor(par.m, testModelIsA, numUnknown);
        requireDf(chiDf(par.m, mode, numEstAll));
        int os = 0, oo = 0, bs = 0, bo = 0, valid = 0;
        Params genPar = par;
        genPar.p_ch = trueCh;
        genPar.p_adv = trueAdv;
        Params testPar = par;
        testPar.p_adv = trueAdv;
        testPar.p_ch = trueCh;   
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
            ps.p = s.p; ps.p_ch = s.p_ch; ps.p_adv = s.p_adv;
            po.p = o.p; po.p_ch = o.p_ch; po.p_adv = o.p_adv;
            if (modelIsValid(ps, testModelIsA)) {
                ChiResult cr = chiSquare(counts, N, ps, testModelIsA, mode, numEstAll);
                res.simple.push_back(cr.defined ? cr.plevel : 0.0);
                if (!cr.defined) bs++;
            }
            else { res.simple.push_back(0.0); bs++; }
            if (modelIsValid(po, testModelIsA)) {
                ChiResult cr = chiSquare(counts, N, po, testModelIsA, mode, numEstAll);
                res.onestep.push_back(cr.defined ? cr.plevel : 0.0);
                if (!cr.defined) bo++;
            }
            else { res.onestep.push_back(0.0); bo++; }
        }
        double denom = valid > 0 ? valid : 1;
        res.outSimple = os / denom;
        res.outOnestep = oo / denom;
        res.badSimple = bs / denom;
        res.badOnestep = bo / denom;
        return res;
    }

    // Записывает массив p-value в CSV-файл.
    static void writePValuesCsv(const std::string& path, const std::vector<double>& pvals)
    {
        std::ofstream out(path.c_str());
        out << "pvalue\n";
        for (size_t i = 0; i < pvals.size(); ++i) out << pvals[i] << "\n";
    }

    // Доля p-value меньше alpha
    static double rejectionRate(const std::vector<double>& pv, double alpha)
    {
        if (pv.empty()) return 0.0;
        int c = 0;
        for (size_t i = 0; i < pv.size(); ++i) if (pv[i] < alpha) c++;
        return static_cast<double>(c) / pv.size();
    }

    // Печатает таблицу по набору уровней alpha.
    static void printRejectionTable(const std::string& name, const std::vector<double>& pv)
    {
        static const double alphas[6] = { 0.01, 0.02, 0.03, 0.04, 0.05, 0.10 };
        std::cout << std::setw(10) << name << " | ";
        for (int i = 0; i < 6; ++i)
            std::cout << std::fixed << std::setprecision(3) << rejectionRate(pv, alphas[i]) << "  ";
        std::cout << "\n";
    }

    // Печатает таблицу мощности по набору уровней alpha.
    static void printPowerTable(double padv, double pch, const std::vector<double>& pv)
    {
        static const double alphas[6] = { 0.01, 0.02, 0.03, 0.04, 0.05, 0.10 };
        std::cout << "модель: p_adv = " << padv << ", p_ch = " << pch
            << ", p_adv*(1-p_ch) = " << padv * (1.0 - pch) << "\n";
        std::cout << "  мощность               | ";
        for (int i = 0; i < 6; ++i)
            std::cout << std::fixed << std::setprecision(3) << rejectionRate(pv, alphas[i]) << "  ";
        std::cout << "\n  уровень значимости a  | ";
        for (int i = 0; i < 6; ++i)
            std::cout << std::fixed << std::setprecision(2) << alphas[i] << "   ";
        std::cout << "\n\n";
    }

    // Мощность в зависимости от p_i

    // Мощность в зависимости от p_i; результат пишется в csvPath.
    static void powerVsP(const Params& par0, const std::vector<std::vector<double> >& pGrid,
        const std::vector<int>& modes, int N, int trials, double alpha,
        int numUnknownAlt, double pch, double padv, std::mt19937& rng,
        const std::string& csvPath)
    {
        std::ofstream csv(csvPath.c_str());
        csv << "mode,p,power\n";
        std::cout << "\n########## Мощность в зависимости от p_i ##########\n";
        std::cout << "N = " << N << ", повторов = " << trials << ", alpha = " << alpha
            << ", p_ch = " << pch << ", p_adv = " << padv
            << ", оцениваемых параметров: " << numUnknownAlt << "\n";

        for (size_t mi = 0; mi < modes.size(); ++mi) {
            int mode = modes[mi];
            std::cout << "\n--- режим: " << modeName(mode)
                << ", df = " << dfStr(par0.m, mode, numEstimatedFor(par0.m, true, numUnknownAlt)) << " ---\n";
            std::cout << std::setw(24) << "p" << " |  power\n";
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

                const int dfHere = chiDf(par.m, mode, numEstimatedFor(par.m, true, numUnknownAlt));
                if (dfHere != kDfDynamic && dfHere < 1) {
                    std::cout << std::setw(24) << label << " | df < 1, пропуск\n";
                    continue;
                }

                EstPValues h1 = collectPValuesCrossModel(N, trials, par, true, pch, padv,
                    false, numUnknownAlt, rng, mode);
                if (h1.onestep.empty()) {
                    std::cout << std::setw(24) << label << " | нет пригодных выборок TSC\n";
                    continue;
                }
                double power = rejectionRate(h1.onestep, alpha);

                std::cout << std::setw(24) << label << " | " << std::fixed << std::setprecision(3)
                    << power << "\n";
                csv << mode << "," << label << "," << power << "\n";
            }
        }
        std::cout << "\nРезультаты записаны в " << csvPath << "\n\n";
    }

    // Проверка согласованности TSC: суммы строк и столбцов

    // Проверка согласованности TSC: суммы строк и столбцов.
    static void sanityCheckTsc(const Params& par, int N, std::mt19937& rng)
    {
        int m = par.m;
        g_printTscHeader = true;
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
            std::cout << "  строки  (набл. | теор.):";
            for (int i = 0; i < m; i++) {
                double o = 0.0, e = 0.0;
                for (int j = 0; j < m; j++) { o += c[i][j]; e += pr[i * m + j]; }
                std::cout << "  " << std::fixed << std::setprecision(3) << o / N << "|" << e;
            }
            std::cout << "\n  столбцы (набл. | теор.):";
            for (int j = 0; j < m; j++) {
                double o = 0.0, e = 0.0;
                for (int i = 0; i < m; i++) { o += c[i][j]; e += pr[i * m + j]; }
                std::cout << "  " << std::fixed << std::setprecision(3) << o / N << "|" << e;
            }
            std::cout << "\n\n";
        }
    }

    // Работа с файлом

    // Каталог, где лежит исполняемый файл. Это с работы взял
    static std::string exeDir()
    {
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

    // Читает квадратную таблицу частот из файла; m берётся по числу строк.
    static bool readCountsFile(const std::string& path, Counts& counts, int& m)
    {
        std::ifstream in(path.c_str());
        if (!in) {
            std::string alt = exeDir() + path;
            in.clear();
            in.open(alt.c_str());
        }
        if (!in) { std::cerr << "Не удалось открыть файл частот: " << path << "\n"; return false; }
        std::vector<std::vector<double> > rows;
        std::string line;
        while (std::getline(in, line)) {
            std::istringstream ls(line);
            std::vector<double> r; double v;
            while (ls >> v) r.push_back(v);
            if (!r.empty()) rows.push_back(r);
        }
        int n = static_cast<int>(rows.size());
        if (n < 2) { std::cerr << "В файле меньше двух непустых строк\n"; return false; }
        for (int i = 0; i < n; ++i)
            if (static_cast<int>(rows[i].size()) != n) {
                std::cerr << "Число строк не совпадает: строк " << n
                    << ", в строке " << (i + 1) << " чисел " << rows[i].size() << "\n";
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


    // Проверка выборки из файла на принадлежность модели А или Б для всех группировок.
    static void fileSampleCriterion(const std::string& path, const Params& base,
        const std::vector<int>& modes, int numUnknown, bool testIsA, double alpha)
    {
        Counts counts;
        int m = 0;
        if (!readCountsFile(path, counts, m)) return;

        long total = 0;
        for (int i = 0; i < m; ++i)
            for (int j = 0; j < m; ++j) total += counts[i][j];
        if (total <= 0) { std::cerr << "Нет данных: сумма частот равна нулю\n"; return; }
        int N = static_cast<int>(total);   // N = сумма частот из файла

        std::cout << "==================================================\n";
        std::cout << "Проверка по файлу " << path << ": m = " << m << ", N = " << N
            << ", модель " << (testIsA ? "А" : "Б")
            << ", оцениваемых параметров: " << numEstimatedFor(m, testIsA, numUnknown) << "\n";
        for (int i = 0; i < m; ++i) {
            for (int j = 0; j < m; ++j) std::cout << std::setw(8) << counts[i][j];
            std::cout << "\n";
        }
        std::cout << "  суммы по строкам:";
        for (int i = 0; i < m; ++i) {
            long r = 0;
            for (int j = 0; j < m; ++j) r += counts[i][j];
            std::cout << std::setw(9) << r;
        }
        std::cout << "\n\n";

        Params testPar = base;
        testPar.m = m;
        Est es, eo;
        g_printTscHeader = true;
        if (!libraryEstimates(counts, N, testPar, testIsA, numUnknown, es, eo)) {
            std::cerr << "не удалось получить оценки TSC\n";
            return;
        }
        Params ps = testPar, po = testPar;
        ps.p = es.p; ps.p_ch = es.p_ch; ps.p_adv = es.p_adv;
        po.p = eo.p; po.p_ch = eo.p_ch; po.p_adv = eo.p_adv;
        std::cout << "Оценки TSC (simple / one-step):\n";
        for (int i = 0; i + 1 < m; ++i)
            std::cout << "  p" << i << " = " << std::fixed << std::setprecision(4)
                << ps.p[i] << " / " << po.p[i] << "\n";
        std::cout << "  pch = " << ps.p_ch << " / " << po.p_ch << "\n";
        std::cout << "  padv = " << ps.p_adv << " / " << po.p_adv << "\n\n";
        const char* names[2] = { "simple  ", "one-step" };
        Params used[2];
        used[0] = ps;
        used[1] = po;
        for (size_t mi = 0; mi < modes.size(); ++mi) {
            int mode = modes[mi];
            //noteMode(mode);
            const int numEstAll = numEstimatedFor(m, testIsA, numUnknown);
            std::cout << "==================================================\n";
            std::cout << "Группировка: " << modeName(mode) << ", df = "
                << dfStr(m, mode, numEstAll) << " (оценено параметров: " << numEstAll << ")\n";
            std::cout << "==================================================\n";
            {
                const int dfCheck = chiDf(m, mode, numEstAll);
                if (dfCheck != kDfDynamic && dfCheck < 1) {
                    std::cout << "df < 1: критерий не применим\n\n";
                    continue;
                }
            }
            for (int k = 0; k < 2; ++k) {
                if (!modelIsValid(used[k], testIsA)) {
                    std::cout << "  " << names[k]
                        << " модель невалидна: оценки вне (0,1), p-value не вычислен\n";
                    continue;
                }
                ChiResult cr = chiSquare(counts, N, used[k], testIsA, mode, numEstAll);
                if (!cr.defined) {
                    std::cout << "  " << names[k]
                        << " критерий не применён: p-value не вычислен\n";
                    continue;
                }
                std::cout << "  " << names[k] << " chi2 = " << std::fixed
                    << std::setprecision(4) << cr.stat
                    << ", p-value = " << std::setprecision(6) << cr.plevel
                    << ", alpha = " << std::setprecision(3) << alpha
                    << " -> " << (cr.plevel < alpha ? "отклоняем H0" : "не отклоняем H0") << "\n";
            }
            std::cout << "\n";
        }
    }


    // Настройки, проверка файла, эмпирические размеры и мощность.
    static int run()
    {
        setlocale(LC_ALL, "C");
#if defined(_WIN32) //тож научили
        {
            HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
            DWORD dwMode = 0;
            if (hOut != INVALID_HANDLE_VALUE && hOut != NULL && GetConsoleMode(hOut, &dwMode))
                SetConsoleOutputCP(1251);
        }
#endif
        // PLevel::CPLevel обращается к дамперу, поэтому он должен быть создан.
        initDumper();

        Params par;   // Параметры модели H0: m, известные p_i, p_ch, p_adv.
        par.m = 3;
        par.p_ch = 0.2;
        par.p_adv = 0.05;
        par.p.clear();
        par.p.push_back(0.5); par.p.push_back(0.3); par.p.push_back(0.2);   // p_i
        /*for (int i = 0; i < 6; ++i)
            par.p.push_back(1);*/
        int N = 300;                // количество людей
        int numTrials = 1000;        // количество p-values
        double alpha = 0.05;          // уровень значимости
        bool modelIsA = false;        // true: модель А
        int numUnknown = 2;          // количество неизвестных (0 или 2)
        int numUnknownAlt = 2;       // для power (0 или 2)
        bool fixedSeed = false;      // true: воспроизводимые генерации
        bool sanityCheck = false;      //   true: напечатать сгенерированные таблицы и суммы строк/столбцов
        g_transposeSample = false;   // true, если категории TSC заданы наоборот
        std::vector<int> modes;   // Номера режимов группировки для проверки.
        modes.push_back(0);
        //modes.push_back(1);  моя реализация (объединение неинформативных ячеек)
        modes.push_back(2); // Standardgroup: не работает корректно в общем случае
        modes.push_back(3); // Autogroup: не работает корректно в общем случае
        // modes.push_back(4);   // ShrinkGroup: не работает в принципе, есть проблема с реализацией в TSC
        // --- ........ .. ..... ...... ---
        bool checkFileSample = true;         // true: проверить файл
        bool onlyFileSample = true;         // true: только файл
        std::string fileSamplePath = "DES95765Group.txt";   // Файл с таблицей частот для проверки.
        // --- мощность в зависимости от p_i ---
        bool runPowerStudy = false;          // true: выполнить проверку p_i
        bool onlyPowerStudy = false;         // true: только проверку p_i
        int psN = 300;   // Размер выборки в исследовании 
        int psTrials = 1000;   // Число p_val
        double psAlpha = 0.05;   // Уровень значимости для таблицы мощности.
        int psUnknown = 2;   // numUnknown для powerVsP.
        double psPch = 0.2, psPadv = 0.05;   // p_ch и p_adv для powerVsP.
        std::vector<std::vector<double> > pGrid;
        const double p0s[7] = { 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8 };
        for (int k = 0; k < 7; ++k) {
            double p0 = p0s[k];   // Первое значение p_0 в сетке.
            std::vector<double> v;   // Вектор p_i для оставшихся p_i.
            v.push_back(p0); v.push_back((1.0 - p0) * 0.6); v.push_back((1.0 - p0) * 0.4);
            pGrid.push_back(v);
        }
        const double ss[5] = { 0.5, 0.6, 0.7, 0.8, 0.9 };
        for (int k = 0; k < 5; ++k) {
            std::vector<double> v;   // Вектор p_i для оставшихся p_i.
            v.push_back(0.5); v.push_back(0.5 * ss[k]); v.push_back(0.5 * (1.0 - ss[k]));
            pGrid.push_back(v);
        }
        // ------------------------------------------------------------
        if (numUnknown < 0 || numUnknown > 2 || numUnknownAlt < 0 || numUnknownAlt > 2) {
            std::cerr << "numUnknown и numUnknownAlt должны быть 0, 1 или 2\n";
            return 1;
        }

        double sumP = std::accumulate(par.p.begin(), par.p.end(), 0.0);   // Сумма p_i перед нормировкой.
        for (size_t k = 0; k < par.p.size(); ++k) par.p[k] /= sumP;

        std::mt19937 rng(fixedSeed ? 12345u : std::random_device()());

        std::cout << "m = " << par.m << ", p_ch = " << par.p_ch << ", p_adv = " << par.p_adv
            << ", N = " << N << ", numTrials = " << numTrials << "\n";
        std::cout << "модель H0: модель " << (modelIsA ? "А" : "Б")
            << ", оцениваемых параметров: " << numUnknown << "\n\n";
        const int numEstAll = numEstimatedFor(par.m, modelIsA, numUnknown);   // Общее число оцениваемых параметров при текущем numUnknown.


        // Дополнительная отладочная печать
        if (sanityCheck) sanityCheckTsc(par, N, rng);
        if (checkFileSample) {
            fileSampleCriterion(fileSamplePath, par, modes, numUnknown, modelIsA, alpha);
            if (onlyFileSample) return 0;
        }

        // теоретические частоты (проверка условия N*p_ij >= 5) 
        {
            std::vector<double> pr = jointProbs(par, modelIsA);   
            int nSmall = 0;   // Число ячеек с N*p_ij < 5.
            double restMass = 0.0;   // Суммарная вероятность неинформативной части при H0.
            std::cout << "Ожидаемые частоты N*p_ij:\n";
            for (int i = 0; i < par.m; i++) {
                for (int j = 0; j < par.m; j++) {
                    double e = N * pr[i * par.m + j];   // Ожидаемая частота N*p_ij в текущей ячейке.
                    if (e < 5.0) nSmall++;
                    if (!isInformative(i, j)) restMass += pr[i * par.m + j];
                    std::cout << std::setw(9) << std::fixed << std::setprecision(1) << e;
                }
                std::cout << "\n";
            }
            std::cout << "ячеек с N*p_ij < 5: " << nSmall << "\n";
            std::cout << "неинформативная часть при H0 (pi_rest): "
                << std::setprecision(3) << restMass << "\n\n";
        }
        
        if (runPowerStudy) {
            if (psUnknown < 0 || psUnknown > 2) {
                std::cerr << "psUnknown должен быть 0, 1 или 2\n"; return 1;
            }
            powerVsP(par, pGrid, modes, psN, psTrials, psAlpha, psUnknown, psPch, psPadv, rng,
                "power_vs_p.csv");
            if (onlyPowerStudy) return 0;
        }

        // удобная таблица 
        for (size_t mi = 0; mi < modes.size(); ++mi) {
            int mode = modes[mi];   // Текущий режим группировки.
            {
                const int dfCheck = chiDf(par.m, mode, numEstAll);   // df для проверки применимости критерия.
                if (dfCheck != kDfDynamic && dfCheck < 1) {
                    std::cout << "==================================================\n";
                    std::cout << "Режим: " << modeName(mode) << ", df = " << dfCheck
                        << " (< 1): при " << numEstAll
                        << " оцениваемых параметрах критерий неприменим\n";
                    std::cout << "==================================================\n\n";
                    continue;
                }
            }
            std::cout << "==================================================\n";
            std::cout << "Режим: " << modeName(mode)
                << ", df (расчётный) = " << dfStr(par.m, mode, numEstAll) << "\n";
            std::cout << "==================================================\n\n";

            RejRates typeI = empiricalRejectionRate(N, numTrials, par, modelIsA, modelIsA, alpha, rng, numUnknown, mode);   // ошибка 1 рода
            RejRates power = empiricalRejectionRate(N, numTrials, par, modelIsA, !modelIsA, alpha, rng, numUnknown, mode);   // мощность при верной альтернативе
            std::cout << "Эмпирическая часть, alpha = " << alpha << ":\n";
            std::cout << "  Ошибка первого рода (H0 верен): simple = " << typeI.simple
                << ", one-step = " << typeI.onestep << "\n";
            std::cout << "  Мощность при H0 неверна: simple = " << power.simple
                << ", one-step = " << power.onestep << "\n\n";

            std::ostringstream modeStr;   // Номер режима строкой для имени CSV-файла.
            modeStr << mode;
            EstPValues pv = collectPValuesEstimated(N, numTrials, par, modelIsA, numUnknown, rng, mode);   // Собранные p-value при оцениваемых параметрах.
            writePValuesCsv("pvalues_simple_mode" + modeStr.str() + ".csv", pv.simple);
            writePValuesCsv("pvalues_onestep_mode" + modeStr.str() + ".csv", pv.onestep);
            std::cout << "Ошибка первого рода при верной H0 (df = " << dfStr(par.m, mode, numEstAll) << "):\n";
            std::cout << "     alpha | 0.010  0.020  0.030  0.040  0.050  0.100\n";
            printRejectionTable("simple", pv.simple);
            printRejectionTable("one-step", pv.onestep);
            std::cout << "оценок вне [0,1]: simple = "
                << pv.outSimple << ", one-step = " << pv.outOnestep << "\n";
            std::cout << "вырожденных случаев (pi_ij <= 0, p-value:=0): simple = "
                << pv.badSimple << ", one-step = " << pv.badOnestep << "\n";
            if (pv.failed > 0) std::cout << "не удалось получить оценки TSC: " << pv.failed << "\n";
            std::cout << "p-value записаны в pvalues_simple_mode" << mode
                << ".csv и pvalues_onestep_mode" << mode << ".csv\n\n";

            // мощность
            {
                std::cout << "--- мощность, оцениваемых параметров: "
                    << numUnknownAlt << " (" << modeName(mode) << ") ---\n";
                int powN = 500;   // Размер выборки 
                int powTrials = 1000;   // количество p_value
                const int dfHere = chiDf(par.m, mode, numEstimatedFor(par.m, modelIsA, numUnknownAlt));   // df при numUnknownAlt для проверки применимости.
                if (dfHere != kDfDynamic && dfHere < 1) {
                    std::cout << "df < 1, пропуск\n\n";
                    continue;
                }
                std::vector<std::pair<double, double> > grid;
                //grid.push_back(std::make_pair(0.4, 0.6));
                grid.push_back(std::make_pair(0.05, 0.2));
                //grid.push_back(std::make_pair(0.2, 0.8));
                //grid.push_back(std::make_pair(0.1, 0.9));
                for (size_t gi = 0; gi < grid.size(); ++gi) {
                    double padv = grid[gi].first, pch = grid[gi].second; 
 
                    EstPValues cross = collectPValuesCrossModel(powN, powTrials, par,
                        /*sampleModelIsA=*/true, pch, padv,
                        /*testModelIsA=*/false, numUnknownAlt, rng, mode);
                    std::cout << "-- оценка one-step --\n";
                    printPowerTable(padv, pch, cross.onestep);
                    std::cout << "-- оценка простая --\n";
                    printPowerTable(padv, pch, cross.simple);
                    std::cout << "оценок вне [0,1]: simple=" << cross.outSimple
                        << ", one-step=" << cross.outOnestep
                        << "; вырожденных: simple=" << cross.badSimple
                        << ", one-step=" << cross.badOnestep;
                    if (cross.failed > 0) std::cout << "; ошибок TSC: " << cross.failed;
                    std::cout << "\n\n";
                }
            }
        }
        return 0;
    }

}

int main()
{
    return chi2test::run();
}
