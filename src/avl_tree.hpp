
// Invariante: para todo no, altura(esq) - altura(dir) = {-1, 0, +1}.
// O rebalanceamento e feito na volta da recursao (insercao e remocao) usando
// as 4 situacoes classicas: LL, RR, LR e RL.

#pragma once

#include "tree.hpp"

namespace bt {

class AVLTree : public ITree {
public:
    AVLTree() = default;
    ~AVLTree() override { clear(); }
    AVLTree(const AVLTree&) = delete;
    AVLTree& operator=(const AVLTree&) = delete;

    bool insert(Key k) override;
    bool remove(Key k) override;
    bool search(Key k) const override;

    std::vector<Key> inOrder() const override;
    void print(std::ostream& os) const override;

    int height() const override { return h(root_); }
    std::size_t size() const override { return size_; }
    bool validate(std::string& msg) const override;

    const Stats& stats() const override { return stats_; }
    void resetStats() override { stats_ = Stats{}; }
    void setVerbose(bool v) override { verbose_ = v; }

    const char* name() const override { return "AVL"; }
    void clear() override;

private:
    struct Node {
        Key key;
        int height;  // altura da subarvore com raiz neste no (folha = 1)
        Node* left;
        Node* right;
        explicit Node(Key k) : key(k), height(1), left(nullptr), right(nullptr) {}
    };

    Node* root_ = nullptr;
    std::size_t size_ = 0;
    Stats stats_;
    bool verbose_ = false;

    static int h(const Node* x) { return x ? x->height : 0; }
    static int balanceFactor(const Node* x) { return x ? h(x->left) - h(x->right) : 0; }
    static void updateHeight(Node* x);
    static Node* minNode(Node* x);

    Node* rotateRight(Node* y);
    Node* rotateLeft(Node* x);
    Node* rebalance(Node* x);

    Node* insertRec(Node* x, Key k, bool& inserted);
    Node* removeRec(Node* x, Key k, bool& removed);

    static void destroy(Node* x);
    static void inOrderRec(const Node* x, std::vector<Key>& out);
    static void printRec(const Node* x, int depth, std::ostream& os);
    static bool validateRec(const Node* x, const Key* lo, const Key* hi,
                            std::string& msg, int& heightOut);
};

}
