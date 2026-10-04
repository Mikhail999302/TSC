// nw1.cpp - вариант nw.cpp, в котором вся математика считается библиотекой.
//
// Ручные формулы pi_ij, оценки p_ch/p_adv и критерий из nw.cpp удалены.
// Теперь:
//   * p(xi_1=i, xi_2=j) = p_i * pi_ij считает TSCCalc (CTSCModel_1Pos /
//     CTSCModel_1Pos_mine): формулы состояний читаются через
//     DataProcessing::ReadExpressions и вычисляются как CExpression::Eval();
//   * оценки p_ch и p_adv считает ECalc: CalcSimpleEstimates (простые
//     оценки) и CalcEnhancedEstimates (усиленные оценки);
//   * генерация выборки - TSCGenerator (CGenerator_1Pos / CGenerator_1Pos_mine);
//   * X^2 и p-value - TSCPlevel::CPLevel (все три режима группировки).
//
// Локально остаются только разбор файла, печать, сборка параметров
// (известные p_i и p_adv), стратегия группировки (только номера групп)
// и подсчёт df.

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
        double p_ch;
        double p_adv;
        bool outside;  // true, если хотя бы одна оценка вне [0, 1]
    };

    static bool outside01(double v) { return v < 0.0 || v > 1.0; }

    // true, если категории TSC заданы наоборот (xi_2 = xi_1)
    static bool g_transposeSample = false;
    static int g_tscFailed = 0;

    // ----------------------------------------------------------------------
    // Модель TSC из библиотек TSCCalc + ECalc.
    //
    // Параметры в MathCalc адресуются глобально по именам ("pch", "padv",
    // "p0", ...), поэтому каждому экземпляру модели выделяется своя карта
    // параметров (AddMap/SelectMap/RemoveMap). Хранилище карт - std::list,
    // поэтому указатели действительны и при нескольких живых моделях.
    // Перед вычислением состояний или оценок модель выбирает свою карту.
    // ----------------------------------------------------------------------
    static bool g_printTscHeader = false;

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
                // при первом обращении, поэтому начальные значения в карте
                // должны быть корректными.
                for (int i = 0; i + 1 < m; ++i)
                    MathCalc::MathContext::SetParameterValue(paramName(i), 0.25);
                MathCalc::MathContext::SetParameterValue(_T("pch"), 0.2);
                MathCalc::MathContext::SetParameterValue(_T("padv"), 0.05);

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

        int dim() const { return m_m; }

        // p(xi_1 = i, xi_2 = j) = p_i * pi_ij, раскладка [i*m + j].
        // Состояние p[i][j] в m_states соответствует частоте n_i_j, а
        // CDiscrete2DModel::SetFrequenciesValues() читает Sample[i*m + j],
        // поэтому индекс состояния совпадает с индексом ячейки.
        std::vector<double> jointProbs(const std::vector<double>& p, double pCh, double pAdv)
        {
            MathCalc::MathContext::SelectMap(m_mapId);
            for (int i = 0; i + 1 < m_m && i < static_cast<int>(p.size()); ++i)
                MathCalc::MathContext::SetParameterValue(paramName(i), p[i]);
            MathCalc::MathContext::SetParameterValue(_T("pch"), pCh);
            MathCalc::MathContext::SetParameterValue(_T("padv"), pAdv);

            std::vector<double> probs(static_cast<size_t>(m_m) * m_m, 0.0);
            for (int i = 0; i < m_m; ++i)
                for (int j = 0; j < m_m; ++j)
                    probs[i * m_m + j] = m_states[i * m_m + j].Eval();
            return probs;
        }

        // Оценки библиотекой по таблице частот counts (раскладка [i*m + j]).
        // counts - исходные частоты (не нормированные), N - их сумма.
        // Возвращает полные наборы из m+1 параметров:
        // simple = [p_0 ... p_{m-2}, p_ch, p_adv] из CalcSimpleEstimates,
        // onestep - то же из CalcEnhancedEstimates.
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

            // CalcEnhancedEstimates повторяет простой шаг и добавляет
            // усиление Фишера; Gamma = 0 отключает доверительные интервалы.
            if (!m_model->CalcEnhancedEstimates(counts, N, m_se, est, 0.0)) return false;
            if (est.size() != n) return false;
            onestep.assign(n, 0.0);
            for (size_t i = 0; i < n; ++i) onestep[i] = est[i].Value;

            return true;
        }

        // Оценки только p_ch и p_adv (индексы m-1 и m в наборе оценок).
        // Если p_adv известен, используется заданное значение.
        bool estimateChAdv(const std::vector<double>& counts, int N, bool pAdvFixed,
            double knownPAdv, Est& simple, Est& oneStep)
        {
            std::vector<double> sAll, oAll;
            if (!estimates(counts, N, sAll, oAll)) return false;

            const int iCh = m_m - 1, iAdv = m_m;

            simple.p_ch = sAll[iCh];
            simple.p_adv = pAdvFixed ? knownPAdv : sAll[iAdv];
            simple.outside = outside01(simple.p_ch) || outside01(simple.p_adv);

            oneStep.p_ch = oAll[iCh];
            oneStep.p_adv = pAdvFixed ? knownPAdv : oAll[iAdv];
            oneStep.outside = outside01(oneStep.p_ch) || outside01(oneStep.p_adv);

            return true;
        }

    private:
        static MathCalc::MathString paramName(int i)
        {
            typedef MathCalc::MathString::value_type CharT;
            std::basic_stringstream<CharT> os;
            os << static_cast<CharT>('p') << i;
            return os.str();
        }

        int m_m;
        bool m_isA;
        MathCalc::MathContext::MapIDType m_mapId;
        MathModels::CCacherComparingStruct m_comp;

        // Порядок объявления важен: m_rep хранит ссылки на кэшеры,
        // поэтому при разрушении он должен умереть раньше них.
        MathModels::TLDotCacher m_ldot;
        MathModels::TInfoCacher m_info;
        std::auto_ptr<GroupedTSC::IModelRep> m_rep;
        MathModels::CExpressions m_states;
        MathModels::CMathParameterEstimates m_se;
        MathModels::CMathParameterInfos m_paramInfos;
        MathModels::ModelPtr m_model;
    };

    // Две модели (А и Б) создаются один раз и переиспользуются.
    static std::auto_ptr<TscModel> g_modelA;
    static std::auto_ptr<TscModel> g_modelB;

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

    static std::vector<double> jointProbs(const Params& par, bool model1)
    {
        return theModel(par.m, model1).jointProbs(par.p, par.p_ch, par.p_adv);
    }

    // ----------------------------------------------------------------------
    // Генерация TSC: библиотека
    // ----------------------------------------------------------------------

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

    static long nextSeed(std::mt19937& rng)
    {
        return static_cast<long>(rng() & 0x7fffffffu);
    }

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

    // ----------------------------------------------------------------------
    // Оценки: только библиотека
    // ----------------------------------------------------------------------

    //   numUnknown == 0: ничего не оцениваем, параметры известны;
    //   numUnknown == 1: p_adv известен, оценивается только p_ch;
    //   numUnknown == 2: оцениваются оба параметра.
    // testPar - параметры проверяемой модели (p_i известны).
    static bool libraryEstimates(const Counts& counts, int N, const Params& testPar,
        bool testIsA, int numUnknown, Est& s, Est& o)
    {
        if (numUnknown == 0) {
            s.p_ch = testPar.p_ch;
            s.p_adv = testPar.p_adv;
            o = s;
            s.outside = false;
            o.outside = false;
            return true;
        }

        const std::vector<double> flat = flatten(counts, testPar.m);
        const bool pAdvFixed = (numUnknown < 2);
        return theModel(testPar.m, testIsA)
            .estimateChAdv(flat, N, pAdvFixed, testPar.p_adv, s, o);
    }

    // ----------------------------------------------------------------------
    // Критерий: группировка и X^2 / p-value (PLevel::CPLevel)
    // ----------------------------------------------------------------------

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

    static const char* modeName(int mode)
    {
        static const char* names[3] = { "все ячейки", "объединение", "только инф." };
        return names[mode];
    }

    // df = (число групп) - 1 - (число оцениваемых параметров)
    static int chiDf(int m, int mode, int numEstimated)
    {
        int cells = (mode == 0) ? m * m : (mode == 1 ? 2 * (m - 1) + 1 : 2 * (m - 1));
        return cells - 1 - numEstimated;
    }

    static void requireDf(int df)
    {
        if (df < 1) {
            std::cerr << "df < 1 (" << df
                << "): неверно заданы mode/m/numUnknown, расчёт невозможен\n";
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

    // Теоретические вероятности и наблюдаемые частоты для подходящего режима.
    static ChiResult chiSquare(const Counts& counts, int N, const Params& par,
        bool testModel1, int mode, int df)
    {
        ChiResult r;
        r.stat = 0.0; r.plevel = 0.0; r.minNPi = 0.0; r.df = df; r.defined = false;
        if (df < 1) return r;

        const int m = par.m;
        const std::vector<double> allProbs = jointProbs(par, testModel1);

        std::vector<double> theoretic, empiric;
        theoretic.reserve(m * m);
        empiric.reserve(m * m);
        for (int i = 0; i < m; i++) {
            for (int j = 0; j < m; j++) {
                if (mode == 2 && !isInformative(i, j)) continue;   // только инф. ячейки
                theoretic.push_back(allProbs[i * m + j]);
                empiric.push_back(static_cast<double>(counts[i][j]));
            }
        }

        PLevel::CPLevel chi;   // CPLevel::Init выполняет расчёт в CalcChiValue
        try {
            if (mode == 1) {
                MergeNonInformative<PLevel::uint, double> strategy;
                chi.Init(theoretic, N, strategy);
            }
            else {
                Tsc::Grouping::NoneGroup<PLevel::uint, double> strategy;
                chi.Init(theoretic, N, strategy);
            }
            chi.CalcChiValue(empiric, df);
        }
        catch (std::exception&) {
            return r;               // df < 1 или нехватка данных
        }

        const PLevel::PLevelData& data = chi.GetPLevelData();
        r.stat = data.criterion_value;
        r.plevel = data.plevel;
        r.minNPi = data.minNPi;
        r.defined = (data.plevel >= 0.0);
        return r;
    }

    // ----------------------------------------------------------------------
    // Эмпирический размер критерия при известных параметрах
    // ----------------------------------------------------------------------

    static double empiricalRejectionRate(int N, int numTrials, const Params& par,
        bool sampleFromModel1, bool testModel1, double alpha, std::mt19937& rng,
        int mode = 0)
    {
        int rejections = 0;
        int df = chiDf(par.m, mode, 0);
        requireDf(df);
        for (int t = 0; t < numTrials; t++) {
            Counts counts = generateSample(N, par, sampleFromModel1, rng);
            ChiResult cr = chiSquare(counts, N, par, testModel1, mode, df);
            if (cr.defined && cr.plevel < alpha) rejections++;
        }
        return static_cast<double>(rejections) / numTrials;
    }

    // ----------------------------------------------------------------------
    // Один эксперимент: генерация + оценки библиотекой
    // ----------------------------------------------------------------------

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

    // ----------------------------------------------------------------------
    // p-value при оценке параметров по выборке (H0 верна)
    // ----------------------------------------------------------------------

    struct EstPValues {
        std::vector<double> simple, onestep;
        double outSimple = 0.0, outOnestep = 0.0;   // доля оценок вне [0,1]
        double badSimple = 0.0, badOnestep = 0.0;   // доля невырожденных случаев
        int failed = 0;                             // ошибки TSC / оценок
    };

    static bool modelIsValid(const Params& par, bool model1)
    {
        const std::vector<double> probs = jointProbs(par, model1);
        for (size_t k = 0; k < probs.size(); ++k)
            if (!(probs[k] > 0.0)) return false;
        return true;
    }

    static EstPValues collectPValuesEstimated(int N, int numTrials, const Params& par,
        bool modelIsA, int numUnknown, std::mt19937& rng, int mode = 0)
    {
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

            Params ps = par, po = par;  // p_i остаются известными
            ps.p_ch = s.p_ch; ps.p_adv = s.p_adv;
            po.p_ch = o.p_ch; po.p_adv = o.p_adv;
            if (modelIsValid(ps, modelIsA)) {
                ChiResult cr = chiSquare(counts, N, ps, modelIsA, mode, df);
                res.simple.push_back(cr.defined ? cr.plevel : 0.0);
                if (!cr.defined) bs++;
            }
            else { res.simple.push_back(0.0); bs++; }
            if (modelIsValid(po, modelIsA)) {
                ChiResult cr = chiSquare(counts, N, po, modelIsA, mode, df);
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

    // ----------------------------------------------------------------------
    // Выборка от sampleModelIsA с истинными (trueCh, trueAdv),
    // проверка моделью testModelIsA ("оценка от другой модели")
    // ----------------------------------------------------------------------

    static EstPValues collectPValuesCrossModel(int N, int numTrials, Params par,
        bool sampleModelIsA, double trueCh, double trueAdv,
        bool testModelIsA, int numUnknown, std::mt19937& rng, int mode = 0)
    {
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
        testPar.p_ch = trueCh;   // истинные значения для numUnknown == 0

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
            if (modelIsValid(ps, testModelIsA)) {
                ChiResult cr = chiSquare(counts, N, ps, testModelIsA, mode, df);
                res.simple.push_back(cr.defined ? cr.plevel : 0.0);
                if (!cr.defined) bs++;
            }
            else { res.simple.push_back(0.0); bs++; }
            if (modelIsValid(po, testModelIsA)) {
                ChiResult cr = chiSquare(counts, N, po, testModelIsA, mode, df);
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

    static void writePValuesCsv(const std::string& path, const std::vector<double>& pvals)
    {
        std::ofstream out(path.c_str());
        out << "pvalue\n";
        for (size_t i = 0; i < pvals.size(); ++i) out << pvals[i] << "\n";
    }

    static double rejectionRate(const std::vector<double>& pv, double alpha)
    {
        if (pv.empty()) return 0.0;
        int c = 0;
        for (size_t i = 0; i < pv.size(); ++i) if (pv[i] < alpha) c++;
        return static_cast<double>(c) / pv.size();
    }

    static void printRejectionTable(const std::string& name, const std::vector<double>& pv)
    {
        static const double alphas[6] = { 0.01, 0.02, 0.03, 0.04, 0.05, 0.10 };
        std::cout << std::setw(10) << name << " | ";
        for (int i = 0; i < 6; ++i)
            std::cout << std::fixed << std::setprecision(3) << rejectionRate(pv, alphas[i]) << "  ";
        std::cout << "\n";
    }

    static void printPowerTable(double padv, double pch, const std::vector<double>& pv)
    {
        static const double alphas[6] = { 0.01, 0.02, 0.03, 0.04, 0.05, 0.10 };
        std::cout << "модель: p_adv = " << padv << ", p_ch = " << pch
            << ", p_adv*(1-p_ch) = " << padv * (1.0 - pch) << "\n";
        std::cout << "  доля отклонений        | ";
        for (int i = 0; i < 6; ++i)
            std::cout << std::fixed << std::setprecision(3) << rejectionRate(pv, alphas[i]) << "  ";
        std::cout << "\n  уровень значимости a  | ";
        for (int i = 0; i < 6; ++i)
            std::cout << std::fixed << std::setprecision(2) << alphas[i] << "   ";
        std::cout << "\n\n";
    }

    // ----------------------------------------------------------------------
    // Мощность в зависимости от p_i
    // ----------------------------------------------------------------------

    static double lowerQuantile(std::vector<double> v, double q)
    {
        std::sort(v.begin(), v.end());
        size_t idx = static_cast<size_t>(std::floor(q * v.size()));
        if (idx >= v.size()) idx = v.size() - 1;
        return v[idx];
    }

    static void powerVsP(const Params& par0, const std::vector<std::vector<double> >& pGrid,
        const std::vector<int>& modes, int N, int trials, double alpha,
        int numUnknownAlt, double pch, double padv, std::mt19937& rng,
        const std::string& csvPath)
    {
        std::ofstream csv(csvPath.c_str());
        csv << "mode,p,size,power,power_calibrated,min_expected\n";
        std::cout << "\n########## Мощность в зависимости от p_i ##########\n";
        std::cout << "N = " << N << ", повторов = " << trials << ", alpha = " << alpha
            << ", p_ch = " << pch << ", p_adv = " << padv
            << ", оцениваемых параметров: " << numUnknownAlt << "\n";
        if (numUnknownAlt == 2)
            std::cout << "Внимание: при 2 неизвестных оценка одношаговая.\n";

        for (size_t mi = 0; mi < modes.size(); ++mi) {
            int mode = modes[mi];
            std::cout << "\n--- режим: " << modeName(mode)
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
                    std::cout << std::setw(24) << label << " | df < 1, пропуск\n";
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
                    std::cout << std::setw(24) << label << " | нет пригодных выборок TSC\n";
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
        std::cout << "\nРезультаты записаны в " << csvPath << "\n\n";
    }

    // ----------------------------------------------------------------------
    // Проверка согласованности TSC: суммы строк и столбцов
    // ----------------------------------------------------------------------

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

    // ----------------------------------------------------------------------
    // Работа с файлом
    // ----------------------------------------------------------------------

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

    // Известные p_i по строкам файла.
    static void pFromRows(const Counts& counts, int m, long N, std::vector<double>& p)
    {
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

    // Набор оценок TSC -> параметры: [p_0 ... p_{m-2}, p_ch, p_adv].
    static void paramsFromTsc(const std::vector<double>& est, int m, Params& par)
    {
        par.m = m;
        par.p.assign(m, 0.0);
        double sum = 0.0;
        for (int i = 0; i + 1 < m; ++i) { par.p[i] = est[i]; sum += est[i]; }
        par.p[m - 1] = 1.0 - sum;
        par.p_ch = est[m - 1];
        par.p_adv = est[m];
    }

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
        int N = static_cast<int>(total);

        std::cout << "==================================================\n";
        std::cout << "Проверка по файлу " << path << ": m = " << m << ", N = " << N
            << ", модель " << (testIsA ? "А" : "Б")
            << ", оцениваемых параметров: " << numUnknown << "\n";
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

        Params ps, po;
        ps.m = m;
        ps.p_ch = base.p_ch;
        ps.p_adv = base.p_adv;
        pFromRows(counts, m, total, ps.p);
        po = ps;

        if (numUnknown == 2) {
            // Оцениваются p_i, p_ch и p_adv.
            std::vector<double> sAll, oAll;
            if (!theModel(m, testIsA).estimates(flatten(counts, m), N, sAll, oAll)) {
                std::cerr << "Не удалось получить оценки TSC\n";
                return;
            }
            paramsFromTsc(sAll, m, ps);
            paramsFromTsc(oAll, m, po);
            std::cout << "Оценки TSC (простые / усиленные):\n";
            for (int i = 0; i + 1 < m; ++i)
                std::cout << "  p" << i << " = " << std::fixed << std::setprecision(4)
                    << ps.p[i] << " / " << po.p[i] << "\n";
            std::cout << "  pch = " << ps.p_ch << " / " << po.p_ch << "\n";
            std::cout << "  padv = " << ps.p_adv << " / " << po.p_adv << "\n\n";
        }
        else {
            // p_i и p_adv известны, оценивается только p_ch.
            Est es, eo;
            g_printTscHeader = true;
        if (!libraryEstimates(counts, N, ps, testIsA, numUnknown, es, eo)) {
                std::cerr << "Не удалось получить оценки TSC\n";
                return;
            }
            ps.p_ch = es.p_ch; ps.p_adv = es.p_adv;
            po.p_ch = eo.p_ch; po.p_adv = eo.p_adv;
            std::cout << "Оценки (простые / усиленные): pch = " << std::fixed
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
                << " (p_i тоже оцениваются)\n";
            std::cout << "==================================================\n";
            if (dfAll < 1) { std::cout << "df < 1: расчёт невозможен\n\n"; continue; }
            for (int k = 0; k < 2; ++k) {
                if (!modelIsValid(used[k], testIsA)) {
                    std::cout << "  " << names[k]
                        << " модель вырождена: некоторые состояния невозможны, p-value не вычисляется\n";
                    continue;
                }
                ChiResult cr = chiSquare(counts, N, used[k], testIsA, mode, dfAll);
                if (!cr.defined) {
                    std::cout << "  " << names[k]
                        << " недостаточно данных: p-value не определён\n";
                    continue;
                }
                std::cout << "  " << names[k] << " chi2 = " << std::fixed
                    << std::setprecision(4) << cr.stat
                    << ", p-value = " << std::setprecision(6) << cr.plevel
                    << ", alpha = " << std::setprecision(3) << alpha
                    << " -> " << (cr.plevel < alpha ? "отклоняем H0" : "не отклоняем H0") << "\n";
            }
            if (dfKnown >= 1) {
                ChiResult rk = chiSquare(counts, N, ps, testIsA, mode, dfKnown);
                ChiResult ro = chiSquare(counts, N, po, testIsA, mode, dfKnown);
                std::cout << "  (при известных p_i, df = " << dfKnown << ": simple = "
                    << (rk.defined ? rk.plevel : 0.0)
                    << ", one-step = "
                    << (ro.defined ? ro.plevel : 0.0)
                    << ")\n";
            }
            std::cout << "\n";
        }
    }

    // ----------------------------------------------------------------------
    // Основной сценарий
    // ----------------------------------------------------------------------

    static int run()
    {
        setlocale(LC_ALL, "C");
#if defined(_WIN32)
        {
            HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
            DWORD dwMode = 0;
            if (hOut != INVALID_HANDLE_VALUE && hOut != NULL && GetConsoleMode(hOut, &dwMode))
                SetConsoleOutputCP(1251);
        }
#endif
        // PLevel::CPLevel обращается к дамперу, поэтому он должен быть создан.
        initDumper();

        // ---------------- настройки: как в nw.cpp ----------------
        Params par;
        par.m = 3;
        par.p_ch = 0.2;
        par.p_adv = 0.05;
        par.p.clear();
        par.p.push_back(0.5); par.p.push_back(0.3); par.p.push_back(0.2);   // известные p_i
        int N = 300;                 // объём выборки
        int numTrials = 1000;        // число повторов
        double alpha = 0.1;          // уровень значимости
        bool modelIsA = true;        // true: модель А
        int numUnknown = 0;          // 0: ничего; 1: оценивается p_ch; 2: p_ch и p_adv
        int numUnknownAlt = 0;       // для power (0 или 1; 2 не проверяется)
        bool fixedSeed = false;      // true: воспроизводимые генерации
        bool sanityCheck = false;     // проверка согласованности TSC
        g_transposeSample = false;   // true, если категории TSC заданы наоборот
        std::vector<int> modes;
        modes.push_back(0); modes.push_back(1);// modes.push_back(2);
        // --- проверка по файлу частот ---
        bool checkFileSample = false;         // true: проверить файл
        bool onlyFileSample = false;         // true: только файл
        std::string fileSamplePath = "DES95765Group.txt";
        // --- мощность в зависимости от p_i ---
        bool runPowerStudy = false;          // true: выполнить study
        bool onlyPowerStudy = false;         // true: только мощность
        int psN = 300;
        int psTrials = 5000;
        double psAlpha = 0.05;
        int psUnknown = 0;                   // 0 или 1
        double psPch = 0.2, psPadv = 0.05;
        std::vector<std::vector<double> > pGrid;
        const double p0s[7] = { 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8 };
        for (int k = 0; k < 7; ++k) {
            double p0 = p0s[k];
            std::vector<double> v;
            v.push_back(p0); v.push_back((1.0 - p0) * 0.6); v.push_back((1.0 - p0) * 0.4);
            pGrid.push_back(v);
        }
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
        std::cout << "модель H0: модель " << (modelIsA ? "А" : "Б")
            << ", оцениваемых параметров: " << numUnknown << "\n\n";

        if (sanityCheck) sanityCheckTsc(par, N, rng);
        if (checkFileSample) {
            fileSampleCriterion(fileSamplePath, par, modes, numUnknown, modelIsA, alpha);
            if (onlyFileSample) return 0;
        }

        // --- теоретические частоты (проверка условия N*p_ij >= 5) ---
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

        // --- эмпирические размеры критерия ---
        for (size_t mi = 0; mi < modes.size(); ++mi) {
            int mode = modes[mi];
            std::cout << "==================================================\n";
            std::cout << "Режим: " << modeName(mode)
                << ", df (известные параметры) = " << chiDf(par.m, mode, 0)
                << ", df (оценка) = " << chiDf(par.m, mode, numUnknown) << "\n";
            std::cout << "==================================================\n\n";

            double typeI = empiricalRejectionRate(N, numTrials, par, modelIsA, modelIsA, alpha, rng, mode);
            double power = empiricalRejectionRate(N, numTrials, par, modelIsA, !modelIsA, alpha, rng, mode);
            std::cout << "Эмпирические размеры, alpha = " << alpha << ":\n";
            std::cout << "  размер критерия (H0 верна): " << typeI << "\n";
            std::cout << "  мощность при H0 неверна: " << power << "\n\n";

            std::ostringstream modeStr;
            modeStr << mode;
            EstPValues pv = collectPValuesEstimated(N, numTrials, par, modelIsA, numUnknown, rng, mode);
            writePValuesCsv("pvalues_simple_mode" + modeStr.str() + ".csv", pv.simple);
            writePValuesCsv("pvalues_onestep_mode" + modeStr.str() + ".csv", pv.onestep);
            std::cout << "Размер критерия при H0 (df = " << chiDf(par.m, mode, numUnknown) << "):\n";
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

            // Оценка от другой модели (мощность)
            {
                std::cout << "--- оценка от другой модели, оцениваемых параметров: "
                    << numUnknownAlt << " (" << modeName(mode) << ") ---\n";
                int powN = 300;
                int powTrials = 1000;
                if (chiDf(par.m, mode, numUnknownAlt) < 1) {
                    std::cout << "df < 1, пропуск\n\n";
                    continue;
                }
                std::vector<std::pair<double, double> > grid;
                grid.push_back(std::make_pair(0.05, 0.2));
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

} // namespace chi2test

int main()
{
    return chi2test::run();
}