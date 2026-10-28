#include "avl_tree.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>

namespace bt {

// ----------------------------------------------------------------------------
// Utilitarios
// ----------------------------------------------------------------------------
void AVLTree::updateHeight(Node* x) {
    x->height = 1 + std::max(h(x->left), h(x->right));
}

AVLTree::Node* AVLTree::minNode(Node* x) {
    while (x->left) x = x->left;
    return x;
}


// Rotacoes
//   rotateRight(y): x = filho esquerdo de y passa a ser a raiz da subarvore;
//   y vira filho direito de x; a subarvore B (direita de x) vira filho
//   esquerdo de y. Antes: y(x(A,B),C). Depois: x(A,y(B,C)).
//   rotateLeft e o espelho dessa operacao. A ordem BST e preservada.

AVLTree::Node* AVLTree::rotateRight(Node* y) {
    Node* x = y->left;
    Node* b = x->right;
    if (verbose_) {
        std::cout << "    [AVL] rotacao a direita em " << y->key
                  << " (" << x->key << " sobe)\n";
    }
    x->right = y;
    y->left = b;
    updateHeight(y);  // y agora e filho: atualiza ele primeiro
    updateHeight(x);
    ++stats_.rotations;
    return x;
}

AVLTree::Node* AVLTree::rotateLeft(Node* x) {
    Node* y = x->right;
    Node* b = y->left;
    if (verbose_) {
        std::cout << "    [AVL] rotacao a esquerda em " << x->key
                  << " (" << y->key << " sobe)\n";
    }
    y->left = x;
    x->right = b;
    updateHeight(x);
    updateHeight(y);
    ++stats_.rotations;
    return y;
}

// Restaura o invariante no no x (cujas subarvores ja sao AVL) e devolve a nova
// raiz da subarvore. Serve tanto para insercao quanto para remocao.

AVLTree::Node* AVLTree::rebalance(Node* x) {
    updateHeight(x);
    int bf = balanceFactor(x);

    if (bf > 1) {  // pesada a esquerda
        if (balanceFactor(x->left) < 0) {  // caso LR: dupla rotacao
            if (verbose_) std::cout << "    [AVL] desbalanceamento em " << x->key << ": caso Esquerda-Direita (LR)\n";
            x->left = rotateLeft(x->left);
        } else {  // caso LL (ou fator 0 apos remocao): rotacao simples
            if (verbose_) std::cout << "    [AVL] desbalanceamento em " << x->key << ": caso Esquerda-Esquerda (LL)\n";
        }
        return rotateRight(x);
    }
    if (bf < -1) {  // pesada a direita
        if (balanceFactor(x->right) > 0) {  // caso RL: dupla rotacao
            if (verbose_) std::cout << "    [AVL] desbalanceamento em " << x->key << ": caso Direita-Esquerda (RL)\n";
            x->right = rotateRight(x->right);
        } else {  // caso RR (ou fator 0 apos remocao): rotacao simples
            if (verbose_) std::cout << "    [AVL] desbalanceamento em " << x->key << ": caso Direita-Direita (RR)\n";
        }
        return rotateLeft(x);
    }
    return x;
}

// Insercao

AVLTree::Node* AVLTree::insertRec(Node* x, Key k, bool& inserted) {
    if (!x) {
        inserted = true;
        return new Node(k);
    }
    if (k < x->key) {
        x->left = insertRec(x->left, k, inserted);
    } else if (k > x->key) {
        x->right = insertRec(x->right, k, inserted);
    } else {
        inserted = false;  // duplicado: nada muda
        return x;
    }
    return rebalance(x);
}

bool AVLTree::insert(Key k) {
    bool inserted = false;
    root_ = insertRec(root_, k, inserted);
    if (inserted) ++size_;
    return inserted;
}

// Remocao
//   - folha ou um filho: o no e substituido pelo filho (ou por nulo)
//   - dois filhos: copia a chave do sucessor em ordem (menor da subarvore
//     direita) e remove o sucessor da subarvore direita

AVLTree::Node* AVLTree::removeRec(Node* x, Key k, bool& removed) {
    if (!x) {
        removed = false;  // inexistente: nada muda
        return nullptr;
    }
    if (k < x->key) {
        x->left = removeRec(x->left, k, removed);
    } else if (k > x->key) {
        x->right = removeRec(x->right, k, removed);
    } else {
        removed = true;
        if (!x->left || !x->right) {
            Node* child = x->left ? x->left : x->right;
            delete x;
            return child;  // o pai fara o rebalanceamento
        }
        Node* succ = minNode(x->right);
        x->key = succ->key;
        bool dummy = false;
        x->right = removeRec(x->right, succ->key, dummy);
    }
    return rebalance(x);
}

bool AVLTree::remove(Key k) {
    bool removed = false;
    root_ = removeRec(root_, k, removed);
    if (removed) --size_;
    return removed;
}

// Busca

bool AVLTree::search(Key k) const {
    const Node* x = root_;
    while (x) {
        if (k < x->key) x = x->left;
        else if (k > x->key) x = x->right;
        else return true;
    }
    return false;
}

// Percurso em ordem e impressao

void AVLTree::inOrderRec(const Node* x, std::vector<Key>& out) {
    if (!x) return;
    inOrderRec(x->left, out);
    out.push_back(x->key);
    inOrderRec(x->right, out);
}

std::vector<Key> AVLTree::inOrder() const {
    std::vector<Key> out;
    out.reserve(size_);
    inOrderRec(root_, out);
    return out;
}

void AVLTree::printRec(const Node* x, int depth, std::ostream& os) {
    if (!x) return;
    printRec(x->right, depth + 1, os);
    int fb = balanceFactor(x);
    os << std::string(static_cast<std::size_t>(depth) * 8, ' ')
       << x->key << " (h" << x->height << ",fb" << (fb > 0 ? "+" : "") << fb << ")\n";
    printRec(x->left, depth + 1, os);
}

void AVLTree::print(std::ostream& os) const {
    if (!root_) {
        os << "(arvore vazia)\n";
        return;
    }
    printRec(root_, 0, os);
}

// Validacao das propriedades da AVL:
//   1) ordem de busca binaria (esq < no < dir, sem repetidos)
//   2) campo height de cada no consistente com as subarvores
//   3) fator de balanceamento em {-1, 0, +1} em todos os nos

bool AVLTree::validateRec(const Node* x, const Key* lo, const Key* hi,
                          std::string& msg, int& heightOut) {
    if (!x) {
        heightOut = 0;
        return true;
    }
    if ((lo && x->key <= *lo) || (hi && x->key >= *hi)) {
        msg = "ordem BST violada no no " + std::to_string(x->key);
        return false;
    }
    int hl = 0, hr = 0;
    if (!validateRec(x->left, lo, &x->key, msg, hl)) return false;
    if (!validateRec(x->right, &x->key, hi, msg, hr)) return false;
    if (std::abs(hl - hr) > 1) {
        msg = "fator de balanceamento fora de [-1,+1] no no " + std::to_string(x->key);
        return false;
    }
    heightOut = 1 + std::max(hl, hr);
    if (x->height != heightOut) {
        msg = "altura armazenada incorreta no no " + std::to_string(x->key);
        return false;
    }
    return true;
}

bool AVLTree::validate(std::string& msg) const {
    int hOut = 0;
    if (!validateRec(root_, nullptr, nullptr, msg, hOut)) return false;
    msg.clear();
    return true;
}

// Liberacao de memoria

void AVLTree::destroy(Node* x) {
    if (!x) return;
    destroy(x->left);
    destroy(x->right);
    delete x;
}

void AVLTree::clear() {
    destroy(root_);
    root_ = nullptr;
    size_ = 0;
}

} 