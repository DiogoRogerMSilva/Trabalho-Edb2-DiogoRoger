
// Realizador/Autor : Diogo Roger de Medeiros Silva
// Matricula: 20250041466
// Disciplina: IMD0039 - Estruturas de Dados Basicas II
//
// O usuario escolhe, em tempo de execucao, entre arvore AVL e Rubro-Negra e
// entao usa um menu com insercao, remocao, busca, percurso em ordem,
// impressao da estrutura, estatisticas e validacao das propriedades.
//
// Uso:  ./bin/arvores [avl|rb] [--color]

#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include <random>
#include <cmath>
#include <algorithm>
#include <utility>

#include "avl_tree.hpp"
#include "rb_tree.hpp"

using namespace bt;

namespace {

enum class Kind { AVL, RB };

std::unique_ptr<ITree> makeTree(Kind k) {
    if (k == Kind::AVL) return std::make_unique<AVLTree>();
    return std::make_unique<RBTree>();
}

// Converte texto em numero inteiro de 64 bits, rejeitando lixo ("12abc", "").
bool parseKey(const std::string& s, Key& out) {
    const char* p = s.c_str();
    char* end = nullptr;
    errno = 0;
    long long v = std::strtoll(p, &end, 10);
    if (end == p || errno == ERANGE) return false;
    while (*end && std::isspace(static_cast<unsigned char>(*end))) ++end;
    if (*end != '\0') return false;
    out = v;
    return true;
}

bool askKey(const std::string& prompt, Key& k) {
    std::cout << prompt;
    std::string line;
    if (!std::getline(std::cin, line)) return false;
    if (!parseKey(line, k)) {
        std::cout << "  Entrada invalida: informe um numero inteiro.\n";
        return false;
    }
    return true;
}

// Embaralhamento Fisher-Yates com mt19937 (reprodutivel em qualquer compilador,
// ao contrario de std::shuffle, cujo algoritmo nao e padronizado).
void shuffleKeys(std::vector<Key>& v, unsigned seed) {
    std::mt19937 rng(seed);
    for (std::size_t i = v.size(); i > 1; --i) {
        std::size_t j = rng() % i;
        std::swap(v[i - 1], v[j]);
    }
}

void printMenu(const ITree& t, bool verbose) {
    std::cout << "\n=========== Arvore " << t.name() << "  (n=" << t.size()
              << ", altura=" << t.height() << ") ===========\n"
              << " 1) Inserir elemento\n"
              << " 2) Inserir varios elementos (separados por espaco)\n"
              << " 3) Remover elemento\n"
              << " 4) Buscar elemento\n"
              << " 5) Percurso em ordem\n"
              << " 6) Imprimir estrutura da arvore\n"
              << " 7) Estatisticas (altura, rotacoes, recoloracoes)\n"
              << " 8) Verificar propriedades da arvore\n"
              << " 9) Carga automatica (crescente / decrescente / aleatoria)\n"
              << "10) Modo detalhado (rotacoes e cores): " << (verbose ? "LIGADO" : "desligado") << "\n"
              << "11) Limpar arvore\n"
              << "12) Trocar tipo de arvore (a arvore atual e descartada)\n"
              << " 0) Sair\n"
              << "Opcao: ";
}

void doInsert(ITree& t, Key k) {
    if (t.insert(k)) std::cout << "  " << k << " inserido.\n";
    else std::cout << "  " << k << " ja existe: valor duplicado ignorado (arvore inalterada).\n";
}

void doInsertMany(ITree& t) {
    std::cout << "Valores: ";
    std::string line;
    if (!std::getline(std::cin, line)) return;
    std::istringstream iss(line);
    std::string tok;
    int ok = 0, dup = 0, bad = 0;
    while (iss >> tok) {
        Key k;
        if (!parseKey(tok, k)) { ++bad; continue; }
        if (t.insert(k)) ++ok; else ++dup;
    }
    std::cout << "  inseridos: " << ok << " | duplicados ignorados: " << dup
              << " | entradas invalidas: " << bad << "\n";
}

void doRemove(ITree& t) {
    Key k;
    if (!askKey("Valor a remover: ", k)) return;
    if (t.remove(k)) std::cout << "  " << k << " removido.\n";
    else std::cout << "  " << k << " nao encontrado: nada foi removido.\n";
}

void doSearch(const ITree& t) {
    Key k;
    if (!askKey("Valor a buscar: ", k)) return;
    std::cout << "  " << k << (t.search(k) ? " ENCONTRADO.\n" : " NAO encontrado.\n");
}

void doInOrder(const ITree& t) {
    std::vector<Key> v = t.inOrder();
    if (v.empty()) { std::cout << "  (arvore vazia)\n"; return; }
    std::cout << "  Em ordem (" << v.size() << " elementos):";
    // Limita a saida para nao inundar o terminal em cargas grandes.
    const std::size_t limit = 200;
    for (std::size_t i = 0; i < v.size() && i < limit; ++i) std::cout << ' ' << v[i];
    if (v.size() > limit) std::cout << " ... (+" << (v.size() - limit) << " omitidos)";
    std::cout << "\n";
}

void doPrint(const ITree& t, bool isRB) {
    if (t.size() > 128) {
        std::cout << "  Arvore grande (n=" << t.size() << "): impressao omitida (limite: 128 nos).\n"
                  << "  Use a opcao 7 para ver altura e estatisticas.\n";
        return;
    }
    std::cout << "  (raiz a esquerda; subarvore direita em cima, esquerda embaixo)\n";
    if (isRB) std::cout << "  Legenda: (R) = vermelho, (B) = preto\n";
    else std::cout << "  Legenda: h = altura do no, fb = fator de balanceamento\n";
    t.print(std::cout);
}

void doStats(const ITree& t, bool isRB) {
    std::size_t n = t.size();
    int minH = (n == 0) ? 0 : static_cast<int>(std::floor(std::log2(static_cast<double>(n)))) + 1;
    std::cout << "  Tipo:                 " << t.name() << "\n"
              << "  Elementos (n):        " << n << "\n"
              << "  Altura:               " << t.height() << "\n"
              << "  Altura minima teorica:" << ' ' << minH << "  (arvore completa)\n"
              << "  Rotacoes:             " << t.stats().rotations << "\n";
    if (isRB) std::cout << "  Recoloracoes:         " << t.stats().recolorings << "\n";
    std::cout << "  (contadores acumulados desde que a arvore foi criada/limpa)\n";
}

void doValidate(const ITree& t) {
    std::string msg;
    if (t.validate(msg)) std::cout << "  OK: todas as propriedades da arvore " << t.name() << " estao preservadas.\n";
    else std::cout << "  VIOLACAO: " << msg << "\n";
}

void doAutoLoad(ITree& t, bool verbose) {
    std::cout << "Tipo de carga: 1) crescente  2) decrescente  3) aleatoria (semente 42): ";
    std::string line;
    if (!std::getline(std::cin, line)) return;
    Key type;
    if (!parseKey(line, type) || type < 1 || type > 3) { std::cout << "  Opcao invalida.\n"; return; }
    Key n;
    if (!askKey("Quantidade de elementos (1..1000000): ", n)) return;
    if (n < 1 || n > 1000000) { std::cout << "  Quantidade fora do intervalo.\n"; return; }

    std::vector<Key> keys(static_cast<std::size_t>(n));
    for (Key i = 0; i < n; ++i) keys[static_cast<std::size_t>(i)] = i + 1;
    if (type == 2) std::reverse(keys.begin(), keys.end());
    if (type == 3) shuffleKeys(keys, 42);

    int inserted = 0;
    t.setVerbose(false);  // evita inundar o terminal com o log de cada rotacao
    for (Key k : keys) if (t.insert(k)) ++inserted;
    t.setVerbose(verbose);
    std::cout << "  " << inserted << " elementos inseridos. Altura atual: " << t.height() << "\n";
}

std::unique_ptr<ITree> chooseTree(Kind& kind) {
    while (true) {
        std::cout << "Escolha o tipo de arvore:\n  1) AVL\n  2) Rubro-Negra\nOpcao: ";
        std::string line;
        if (!std::getline(std::cin, line)) return nullptr;
        Key opt;
        if (parseKey(line, opt) && opt == 1) { kind = Kind::AVL; return makeTree(kind); }
        if (parseKey(line, opt) && opt == 2) { kind = Kind::RB; return makeTree(kind); }
        std::cout << "  Opcao invalida.\n";
    }
}

}  // namespace

int main(int argc, char** argv) {
    Kind kind = Kind::AVL;
    std::unique_ptr<ITree> tree;

    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--color") g_useColor = true;
        else if (a == "avl") { kind = Kind::AVL; tree = makeTree(kind); }
        else if (a == "rb") { kind = Kind::RB; tree = makeTree(kind); }
        else {
            std::cout << "Uso: " << argv[0] << " [avl|rb] [--color]\n";
            return 1;
        }
    }

    std::cout << "Amazon — registro de encomendas (Black Friday)\n";
    if (!tree) tree = chooseTree(kind);
    if (!tree) return 0;

    bool verbose = false;
    while (true) {
        printMenu(*tree, verbose);
        std::string line;
        if (!std::getline(std::cin, line)) break;  // EOF (Ctrl+D / Ctrl+Z)
        Key opt;
        if (!parseKey(line, opt)) { std::cout << "  Opcao invalida.\n"; continue; }

        bool isRB = (kind == Kind::RB);
        switch (opt) {
            case 1: { Key k; if (askKey("Valor a inserir: ", k)) doInsert(*tree, k); break; }
            case 2: doInsertMany(*tree); break;
            case 3: doRemove(*tree); break;
            case 4: doSearch(*tree); break;
            case 5: doInOrder(*tree); break;
            case 6: doPrint(*tree, isRB); break;
            case 7: doStats(*tree, isRB); break;
            case 8: doValidate(*tree); break;
            case 9: doAutoLoad(*tree, verbose); break;
            case 10:
                verbose = !verbose;
                tree->setVerbose(verbose);
                std::cout << "  Modo detalhado " << (verbose ? "ligado" : "desligado") << ".\n";
                break;
            case 11:
                tree->clear();
                tree->resetStats();
                std::cout << "  Arvore limpa.\n";
                break;
            case 12:
                kind = (kind == Kind::AVL) ? Kind::RB : Kind::AVL;
                tree = makeTree(kind);
                tree->setVerbose(verbose);
                std::cout << "  Agora usando a arvore " << tree->name() << " (vazia).\n";
                break;
            case 0:
                std::cout << "Ate logo!\n";
                return 0;
            default:
                std::cout << "  Opcao invalida.\n";
        }
    }
    return 0;
}
