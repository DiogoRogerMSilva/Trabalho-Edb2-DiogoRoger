// =============================================================================
// rb_tree.hpp — Arvore Rubro-Negra (implementacao propria, sem bibliotecas)
//
// Propriedades:
//   1) todo no e vermelho ou preto;
//   2) a raiz e preta;
//   3) as folhas (NIL) sao pretas;
//   4) filhos de um no vermelho sao pretos (sem dois vermelhos seguidos);
//   5) todo caminho de um no ate as folhas NIL abaixo dele tem o mesmo
//      numero de nos pretos (altura-negra).
//
// Implementacao no estilo do Cormen (CLRS): nos com ponteiro para o pai e um
// unico no sentinela nil_ (preto) no lugar de nullptr.
// =============================================================================
#pragma once

#include "tree.hpp"

namespace bt {

class RBTree : public ITree {
public:
    RBTree();
    ~RBTree() override;
    RBTree(const RBTree&) = delete;
    RBTree& operator=(const RBTree&) = delete;

    bool insert(Key k) override;
    bool remove(Key k) override;
    bool search(Key k) const override;

    std::vector<Key> inOrder() const override;
    void print(std::ostream& os) const override;

    int height() const override { return heightRec(root_); }
    std::size_t size() const override { return size_; }
    bool validate(std::string& msg) const override;

    const Stats& stats() const override { return stats_; }
    void resetStats() override { stats_ = Stats{}; }
    void setVerbose(bool v) override { verbose_ = v; }

    const char* name() const override { return "Rubro-Negra"; }
    void clear() override;

private:
    enum class Color { Red, Black };

    struct Node {
        Key key;
        Color color;
        Node* left;
        Node* right;
        Node* parent;
    };

    Node* nil_;   // sentinela preta compartilhada
    Node* root_;
    std::size_t size_ = 0;
    Stats stats_;
    bool verbose_ = false;

    void rotateLeft(Node* x);
    void rotateRight(Node* y);
    void setColor(Node* x, Color c);  // conta recoloracoes e (se verbose) loga

    void insertFixup(Node* z);
    void deleteFixup(Node* x);
    void transplant(Node* u, Node* v);

    Node* minimum(Node* x) const;
    Node* findNode(Key k) const;

    void destroy(Node* x);
    void inOrderRec(const Node* x, std::vector<Key>& out) const;
    int heightRec(const Node* x) const;
    void printRec(const Node* x, int depth, std::ostream& os) const;
    bool validateRec(const Node* x, const Key* lo, const Key* hi,
                     std::string& msg, int& blackHeight) const;
};

}  // namespace bt
