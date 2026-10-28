
// tree.hpp — Interface comum as arvores balanceadas (AVL e Rubro-Negra)
//
// Autor: Diogo Roger de Medeiros Silva
// Matricula: 20250041466
// Disciplina: IMD0039 - Estruturas de Dados Basicas II
//
// Toda a interacao com as arvores (menu, testes, demo) e feita atraves de
// ITree, o que permite escolher o tipo de arvore em tempo de execucao e
// executar exatamente as mesmas sequencias de operacoes nas duas estruturas.
//
// Convencao de altura (a mesma nas duas arvores):
//   - arvore vazia            -> altura 0
//   - arvore com um so no     -> altura 1
//   (a altura e contada em NOS, nao em arestas)
// =============================================================================
#pragma once

#include <cstddef>
#include <ostream>
#include <string>
#include <vector>

namespace bt {

// Identificador de encomenda (ex.: codigo de rastreio numerico).
using Key = long long;

// Contadores de operacoes de balanceamento, usados na analise comparativa.
struct Stats {
    long long rotations = 0;    // rotacoes simples (uma dupla = 2 rotacoes)
    long long recolorings = 0;  // mudancas de cor (apenas Rubro-Negra)
};

// Se verdadeiro, a impressao da arvore Rubro-Negra usa cores ANSI no terminal.
// Mesmo com cores desligadas, cada no mostra "(R)" ou "(B)".
#if __cplusplus >= 201703L
inline bool g_useColor = false;
#else
static bool g_useColor = false;
#endif

class ITree {
public:
    virtual ~ITree() = default;

    // Retorna true se inseriu; false se o valor ja existia (arvore inalterada).
    virtual bool insert(Key k) = 0;
    // Retorna true se removeu; false se o valor nao existia (arvore inalterada).
    virtual bool remove(Key k) = 0;
    virtual bool search(Key k) const = 0;

    // Elementos em ordem crescente.
    virtual std::vector<Key> inOrder() const = 0;
    // Desenha a arvore "deitada": subarvore direita no topo, raiz a esquerda.
    virtual void print(std::ostream& os) const = 0;

    virtual int height() const = 0;
    virtual std::size_t size() const = 0;

    // Verifica TODAS as propriedades da estrutura (ordem BST + balanceamento).
    // Em caso de violacao, devolve false e descreve o problema em msg.
    virtual bool validate(std::string& msg) const = 0;

    virtual const Stats& stats() const = 0;
    virtual void resetStats() = 0;
    // Modo detalhado: imprime em std::cout cada rotacao / recoloracao.
    virtual void setVerbose(bool v) = 0;

    virtual const char* name() const = 0;
    virtual void clear() = 0;
};

}