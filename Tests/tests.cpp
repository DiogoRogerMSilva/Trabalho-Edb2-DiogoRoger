
// tests.cpp — Testes automatizados e analise comparativa (AVL x Rubro-Negra)
//
// Executa a MESMA bateria de testes nas duas arvores e, ao final, imprime a
// tabela comparativa (altura, rotacoes, recoloracoes) com os MESMOS conjuntos
// de dados e as MESMAS sequencias de operacoes.
//
// Uso:  ./bin/testes        (retorna 0 se todos os testes passarem)

#include <algorithm>
#include <cmath>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <numeric>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "avl_tree.hpp"
#include "rb_tree.hpp"

using namespace bt;

// Mini framework de testes

static int g_checks = 0;
static int g_fail = 0;

#define CHECK(cond, msg)                                                       \
    do {                                                                       \
        ++g_checks;                                                            \
        if (!(cond)) {                                                         \
            ++g_fail;                                                          \
            std::cout << "      [FALHOU] " << (msg) << "  (linha " << __LINE__ \
                      << ")\n";                                                \
        }                                                                      \
    } while (0)

struct Impl {
    std::string name;
    std::function<std::unique_ptr<ITree>()> make;
    double (*maxHeight)(std::size_t);  // limite teorico superior da altura
};

static double avlMaxHeight(std::size_t n) { return 1.4405 * std::log2(double(n) + 2.0) - 0.3277; }
static double rbMaxHeight(std::size_t n) { return 2.0 * std::log2(double(n) + 1.0); }

static bool valid(const ITree& t, const std::string& ctx) {
    std::string msg;
    if (t.validate(msg)) return true;
    std::cout << "      invariante violado (" << ctx << "): " << msg << "\n";
    return false;
}

enum class Order { Asc, Desc, Random };

static void shuffleKeys(std::vector<Key>& v, unsigned seed) {
    std::mt19937 rng(seed);
    for (std::size_t i = v.size(); i > 1; --i) std::swap(v[i - 1], v[rng() % i]);
}

static std::vector<Key> makeKeys(Order o, std::size_t n, unsigned seed = 42) {
    std::vector<Key> v(n);
    std::iota(v.begin(), v.end(), 1);
    if (o == Order::Desc) std::reverse(v.begin(), v.end());
    if (o == Order::Random) shuffleKeys(v, seed);
    return v;
}

static const char* orderName(Order o) {
    switch (o) {
        case Order::Asc: return "crescente";
        case Order::Desc: return "decrescente";
        default: return "aleatoria";
    }
}

// Testes

static void testEmpty(const Impl& impl) {
    auto t = impl.make();
    CHECK(!t->search(5), "busca em arvore vazia deve falhar");
    CHECK(!t->remove(5), "remocao em arvore vazia deve retornar false");
    CHECK(t->inOrder().empty(), "percurso em ordem vazio");
    CHECK(t->height() == 0, "altura da arvore vazia = 0");
    CHECK(t->size() == 0, "tamanho 0");
    CHECK(valid(*t, "vazia"), "propriedades da arvore vazia");
    std::ostringstream os;
    t->print(os);
    CHECK(!os.str().empty(), "impressao da arvore vazia nao deve quebrar");
}

static void testSingle(const Impl& impl) {
    auto t = impl.make();
    CHECK(t->insert(42), "inserir 42");
    CHECK(t->search(42), "buscar 42");
    CHECK(!t->search(41), "buscar 41 (inexistente)");
    CHECK(t->height() == 1, "altura com 1 elemento = 1");
    CHECK(t->size() == 1, "tamanho 1");
    CHECK((t->inOrder() == std::vector<Key>{42}), "em ordem = {42}");
    CHECK(valid(*t, "1 elemento"), "propriedades com 1 elemento");
    CHECK(t->remove(42), "remover 42");
    CHECK(t->size() == 0 && t->height() == 0, "arvore volta a ficar vazia");
    CHECK(!t->remove(42), "remover 42 de novo deve falhar");
    CHECK(valid(*t, "apos esvaziar"), "propriedades apos esvaziar");
}

static void testDuplicates(const Impl& impl) {
    auto t = impl.make();
    for (Key k : {10, 20, 30}) t->insert(k);
    Stats before = t->stats();
    CHECK(!t->insert(20), "inserir duplicado deve retornar false");
    CHECK(t->size() == 3, "tamanho inalterado apos duplicado");
    CHECK((t->inOrder() == std::vector<Key>{10, 20, 30}), "conteudo inalterado apos duplicado");
    CHECK(t->stats().rotations == before.rotations, "duplicado nao causa rotacoes");
    CHECK(valid(*t, "duplicado"), "propriedades apos duplicado");
}

static void testNonexistent(const Impl& impl) {
    auto t = impl.make();
    for (Key k = 1; k <= 10; ++k) t->insert(k);
    CHECK(!t->remove(99), "remover 99 (inexistente)");
    CHECK(!t->remove(0), "remover 0 (inexistente)");
    CHECK(!t->search(99), "buscar 99 (inexistente)");
    CHECK(!t->search(-5), "buscar -5 (inexistente)");
    CHECK(t->size() == 10, "tamanho inalterado");
    CHECK(valid(*t, "inexistentes"), "propriedades apos operacoes inexistentes");
}

static void testNegativeAndLarge(const Impl& impl) {
    auto t = impl.make();
    std::vector<Key> keys = {0, -1, 9000000000000LL, -9000000000000LL, 7, -7};
    for (Key k : keys) CHECK(t->insert(k), "inserir chave negativa/grande");
    std::vector<Key> sorted = keys;
    std::sort(sorted.begin(), sorted.end());
    CHECK(t->inOrder() == sorted, "ordem correta com negativos e valores de 64 bits");
    CHECK(valid(*t, "negativos"), "propriedades");
}

static void testInsertionOrders(const Impl& impl) {
    const std::size_t N = 500;
    for (Order o : {Order::Asc, Order::Desc, Order::Random}) {
        auto t = impl.make();
        std::vector<Key> keys = makeKeys(o, N);
        bool ok = true;
        for (Key k : keys) {
            CHECK(t->insert(k), "insercao deve ter sucesso");
            ok = valid(*t, std::string("insercao ") + orderName(o) + " chave " + std::to_string(k));
            CHECK(ok, "propriedades apos insercao");
            if (!ok) break;
        }
        std::vector<Key> expected = makeKeys(Order::Asc, N);
        CHECK(t->inOrder() == expected, std::string("em ordem crescente apos carga ") + orderName(o));
        bool allFound = true;
        for (Key k : expected) allFound = allFound && t->search(k);
        CHECK(allFound, "todos os elementos inseridos sao encontrados");
        CHECK(!t->search(0) && !t->search(static_cast<Key>(N) + 1), "inexistentes nao sao encontrados");
        CHECK(t->height() <= impl.maxHeight(N) + 1e-9,
              std::string("altura dentro do limite teorico (") + orderName(o) + ")");
    }
}

// Remove CADA chave, uma por vez, de uma arvore recem-construida: como a arvore
// tem 31 nos, isso exercita a remocao de folhas, nos com um filho (esq. ou
// dir.), nos com dois filhos e da raiz.
static void testRemovalEveryNode(const Impl& impl) {
    const Key N = 31;
    for (Order o : {Order::Asc, Order::Desc, Order::Random}) {
        std::vector<Key> keys = makeKeys(o, static_cast<std::size_t>(N));
        for (Key victim = 1; victim <= N; ++victim) {
            auto t = impl.make();
            for (Key k : keys) t->insert(k);
            CHECK(t->remove(victim), "remover chave existente");
            std::vector<Key> expected;
            for (Key k = 1; k <= N; ++k) if (k != victim) expected.push_back(k);
            CHECK(t->inOrder() == expected, "conteudo correto apos remover " + std::to_string(victim));
            bool ok = valid(*t, "remover " + std::to_string(victim));
            CHECK(ok, "propriedades apos remocao de um no");
            CHECK(!t->search(victim), "chave removida nao e mais encontrada");
            if (!ok) return;
        }
    }
}

// Remove todos os elementos em varias ordens validando a cada passo.

static void testRemovalOrders(const Impl& impl) {
    const std::size_t N = 400;
    for (Order o : {Order::Asc, Order::Desc, Order::Random}) {
        auto t = impl.make();
        for (Key k : makeKeys(Order::Random, N, 7)) t->insert(k);
        std::set<Key> ref;
        for (Key k = 1; k <= static_cast<Key>(N); ++k) ref.insert(k);
        for (Key k : makeKeys(o, N, 99)) {
            CHECK(t->remove(k), "remocao deve ter sucesso");
            ref.erase(k);
            bool ok = valid(*t, std::string("remocao ") + orderName(o) + " chave " + std::to_string(k));
            CHECK(ok, "propriedades apos remocao");
            if (!ok) return;
            CHECK(t->size() == ref.size(), "tamanho coerente");
        }
        CHECK(t->size() == 0 && t->height() == 0, "arvore vazia ao final");
    }
}

// Compara com std::set (apenas como "oraculo" nos testes) em operacoes mistas.
static void testMixedOracle(const Impl& impl) {
    std::mt19937 rng(123);
    auto t = impl.make();
    std::set<Key> ref;
    for (int i = 0; i < 20000; ++i) {
        Key k = static_cast<Key>(rng() % 300);
        unsigned op = rng() % 5;
        if (op <= 1) CHECK(t->insert(k) == ref.insert(k).second, "inserir == oraculo");
        else if (op <= 3) CHECK(t->remove(k) == (ref.erase(k) > 0), "remover == oraculo");
        else CHECK(t->search(k) == (ref.count(k) > 0), "buscar == oraculo");
        CHECK(t->size() == ref.size(), "tamanho == oraculo");
        if (i % 100 == 0) {
            bool ok = valid(*t, "operacoes mistas, passo " + std::to_string(i));
            CHECK(ok, "propriedades durante operacoes mistas");
            CHECK(t->inOrder() == std::vector<Key>(ref.begin(), ref.end()), "conteudo == oraculo");
            if (!ok) return;
        }
    }
    CHECK(valid(*t, "fim das operacoes mistas"), "propriedades no final");
}

// Analise comparativa

struct Row {
    std::size_t nIns, nFinal;
    int hIns, hFinal;
    long long rotIns, rotTotal, recIns, recTotal;
};

static Row runScenario(const Impl& impl, const std::vector<Key>& ins, const std::vector<Key>& rem) {
    auto t = impl.make();
    for (Key k : ins) t->insert(k);
    Row r{};
    r.nIns = t->size();
    r.hIns = t->height();
    r.rotIns = t->stats().rotations;
    r.recIns = t->stats().recolorings;
    for (Key k : rem) t->remove(k);
    r.nFinal = t->size();
    r.hFinal = t->height();
    r.rotTotal = t->stats().rotations;
    r.recTotal = t->stats().recolorings;
    std::string msg;
    if (!t->validate(msg)) {
        ++g_fail;
        std::cout << "      [FALHOU] comparacao: " << msg << "\n";
    }
    return r;
}

static void comparison(const std::vector<Impl>& impls) {
    std::cout << "\n================ ANALISE COMPARATIVA ================\n"
              << "Mesmos dados e mesma sequencia de operacoes nas duas arvores.\n"
              << "Cenario: inserir n chaves; depois remover metade delas (ordem aleatoria, semente 7).\n"
              << "Altura contada em nos. 'min.' = ceil(log2(n+1)), a menor altura possivel.\n\n";

    std::cout << std::left << std::setw(13) << "Insercao" << std::right
              << std::setw(8) << "n" << std::setw(7) << "min."
              << "  " << std::left << std::setw(12) << "Arvore" << std::right
              << std::setw(8) << "alt.ins" << std::setw(9) << "rot.ins" << std::setw(9) << "rec.ins"
              << std::setw(9) << "alt.fin" << std::setw(10) << "rot.tot" << std::setw(10) << "rec.tot" << "\n"
              << std::string(100, '-') << "\n";

    for (std::size_t n : {1000u, 10000u, 100000u}) {
        for (Order o : {Order::Asc, Order::Desc, Order::Random}) {
            std::vector<Key> ins = makeKeys(o, n, 42);
            std::vector<Key> rem = makeKeys(Order::Random, n, 7);
            rem.resize(n / 2);
            int minH = static_cast<int>(std::ceil(std::log2(double(n) + 1.0)));
            for (const Impl& impl : impls) {
                Row r = runScenario(impl, ins, rem);
                std::cout << std::left << std::setw(13) << orderName(o) << std::right
                          << std::setw(8) << n << std::setw(7) << minH
                          << "  " << std::left << std::setw(12) << impl.name << std::right
                          << std::setw(8) << r.hIns << std::setw(9) << r.rotIns << std::setw(9) << r.recIns
                          << std::setw(9) << r.hFinal << std::setw(10) << r.rotTotal << std::setw(10) << r.recTotal << "\n";
            }
        }
        std::cout << std::string(100, '-') << "\n";
    }
    std::cout << "Legenda: alt.ins/rot.ins/rec.ins = apos as insercoes; alt.fin = apos remover metade;\n"
              << "         rot.tot/rec.tot = acumulado (insercoes + remocoes).\n"
              << "         Recoloracoes so se aplicam a Rubro-Negra.\n";
}

// ----------------------------------------------------------------------------
int main() {
    std::vector<Impl> impls = {
        {"AVL", [] { return std::unique_ptr<ITree>(new AVLTree()); }, avlMaxHeight},
        {"Rubro-Negra", [] { return std::unique_ptr<ITree>(new RBTree()); }, rbMaxHeight},
    };

    struct T { const char* title; void (*fn)(const Impl&); };
    const T tests[] = {
        {"arvore vazia", testEmpty},
        {"arvore com um unico elemento", testSingle},
        {"insercao de valores repetidos", testDuplicates},
        {"busca/remocao de elementos inexistentes", testNonexistent},
        {"chaves negativas e de 64 bits", testNegativeAndLarge},
        {"insercao crescente, decrescente e aleatoria (+ propriedades a cada passo)", testInsertionOrders},
        {"remocao de cada no (folha, 1 filho, 2 filhos, raiz)", testRemovalEveryNode},
        {"remocao total em ordem crescente, decrescente e aleatoria", testRemovalOrders},
        {"20 000 operacoes mistas comparadas com std::set", testMixedOracle},
    };

    for (const Impl& impl : impls) {
        std::cout << "\n=== Testes: " << impl.name << " ===\n";
        for (const T& t : tests) {
            int before = g_fail;
            t.fn(impl);
            std::cout << "  [" << (g_fail == before ? " OK " : "FAIL") << "] " << t.title << "\n";
        }
    }

    comparison(impls);

    std::cout << "\nVerificacoes executadas: " << g_checks << " | falhas: " << g_fail << "\n"
              << (g_fail == 0 ? "RESULTADO: TODOS OS TESTES PASSARAM\n" : "RESULTADO: HA FALHAS\n");
    return g_fail == 0 ? 0 : 1;
}
