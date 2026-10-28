
// demo.cpp — Exemplos "antes e depois" de rotações e ajustes de cor
//
// Gera as evidências para os slides: cada cenário mostra a árvore ANTES da
// operação, o log detalhado do que o algoritmo fez e a árvore DEPOIS.
//
// Uso:  ./bin/demo

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "avl_tree.hpp"
#include "rb_tree.hpp"

using namespace bt;

namespace {

void build(ITree& t, const std::vector<Key>& keys) {
    for (Key k : keys) t.insert(k);
}

void opShow(ITree& t, bool insertOp, Key k) {
    std::cout << "\n  ANTES:\n";
    t.print(std::cout);
    std::cout << "\n  >> " << (insertOp ? "inserir " : "remover ") << k << "\n";
    t.setVerbose(true);
    if (insertOp) t.insert(k); else t.remove(k);
    t.setVerbose(false);
    std::cout << "\n  DEPOIS:\n";
    t.print(std::cout);
}

void scenario(const std::string& title, std::unique_ptr<ITree> t,
              const std::vector<Key>& prefix, bool insertOp, Key k) {
    std::cout << "\n------------------------------------------------------------\n"
              << title << "\n"
              << "------------------------------------------------------------";
    build(*t, prefix);
    opShow(*t, insertOp, k);
    std::string msg;
    std::cout << "\n  Validacao: " << (t->validate(msg) ? "OK" : msg) << "\n";
}

}  // namespace

int main() {
    std::cout << "=== AVL: os quatro casos de rotacao ===\n";
    scenario("AVL - caso LL (rotacao simples a direita): 30,20 + inserir 10",
             std::make_unique<AVLTree>(), {30, 20}, true, 10);
    scenario("AVL - caso RR (rotacao simples a esquerda): 10,20 + inserir 30",
             std::make_unique<AVLTree>(), {10, 20}, true, 30);
    scenario("AVL - caso LR (rotacao dupla esquerda-direita): 30,10 + inserir 20",
             std::make_unique<AVLTree>(), {30, 10}, true, 20);
    scenario("AVL - caso RL (rotacao dupla direita-esquerda): 10,30 + inserir 20",
             std::make_unique<AVLTree>(), {10, 30}, true, 20);
    scenario("AVL - rotacao causada por REMOCAO: 20,10,30,5 + remover 30",
             std::make_unique<AVLTree>(), {20, 10, 30, 5}, false, 30);

    std::cout << "\n\n=== Rubro-Negra: recoloracao e rotacoes ===\n";
    scenario("RB - caso 1 (tio vermelho => so recoloracao): 10,5,15 + inserir 1",
             std::make_unique<RBTree>(), {10, 5, 15}, true, 1);
    scenario("RB - caso 3 (tio preto, mesma linha => rotacao simples + cores): 10,20 + inserir 30",
             std::make_unique<RBTree>(), {10, 20}, true, 30);
    scenario("RB - caso 2+3 (tio preto, zig-zag => rotacao dupla): 10,30 + inserir 20",
             std::make_unique<RBTree>(), {10, 30}, true, 20);
    scenario("RB - remocao de no PRETO (ajuste com rotacao e cores): 10,20,30,40 + remover 10",
             std::make_unique<RBTree>(), {10, 20, 30, 40}, false, 10);

    std::cout << "\n";
    return 0;
}
