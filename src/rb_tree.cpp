
// rb_tree.cpp — Implementacao da arvore Rubro-Negra

#include "rb_tree.hpp"

#include <algorithm>
#include <iostream>

namespace bt {

RBTree::RBTree() {
    nil_ = new Node{0, Color::Black, nullptr, nullptr, nullptr};
    nil_->left = nil_->right = nil_->parent = nil_;
    root_ = nil_;
}

RBTree::~RBTree() {
    clear();
    delete nil_;
}

// Utilitarios

void RBTree::setColor(Node* x, Color c) {
    if (x == nil_ || x->color == c) return;  // sem mudanca: nao conta
    if (verbose_) {
        std::cout << "    [RB] recolorir " << x->key << ": "
                  << (x->color == Color::Red ? "vermelho" : "preto") << " -> "
                  << (c == Color::Red ? "vermelho" : "preto") << "\n";
    }
    x->color = c;
    ++stats_.recolorings;
}

RBTree::Node* RBTree::minimum(Node* x) const {
    while (x->left != nil_) x = x->left;
    return x;
}

RBTree::Node* RBTree::findNode(Key k) const {
    Node* x = root_;
    while (x != nil_) {
        if (k < x->key) x = x->left;
        else if (k > x->key) x = x->right;
        else return x;
    }
    return nil_;
}

// Rotacoes

void RBTree::rotateLeft(Node* x) {
    Node* y = x->right;
    if (verbose_) {
        std::cout << "    [RB] rotacao a esquerda em " << x->key
                  << " (" << y->key << " sobe)\n";
    }
    x->right = y->left;
    if (y->left != nil_) y->left->parent = x;
    y->parent = x->parent;
    if (x->parent == nil_) root_ = y;
    else if (x == x->parent->left) x->parent->left = y;
    else x->parent->right = y;
    y->left = x;
    x->parent = y;
    ++stats_.rotations;
}

void RBTree::rotateRight(Node* y) {
    Node* x = y->left;
    if (verbose_) {
        std::cout << "    [RB] rotacao a direita em " << y->key
                  << " (" << x->key << " sobe)\n";
    }
    y->left = x->right;
    if (x->right != nil_) x->right->parent = y;
    x->parent = y->parent;
    if (y->parent == nil_) root_ = x;
    else if (y == y->parent->left) y->parent->left = x;
    else y->parent->right = x;
    x->right = y;
    y->parent = x;
    ++stats_.rotations;
}


// Insercao

bool RBTree::insert(Key k) {
    Node* parent = nil_;
    Node* x = root_;
    while (x != nil_) {
        parent = x;
        if (k < x->key) x = x->left;
        else if (k > x->key) x = x->right;
        else return false;  // duplicado: nada muda
    }
    Node* z = new Node{k, Color::Red, nil_, nil_, parent};
    if (parent == nil_) root_ = z;
    else if (k < parent->key) parent->left = z;
    else parent->right = z;
    ++size_;
    insertFixup(z);
    return true;
}

void RBTree::insertFixup(Node* z) {
    while (z->parent->color == Color::Red) {
        Node* gp = z->parent->parent;  // existe: pai vermelho nunca e a raiz
        if (z->parent == gp->left) {
            Node* uncle = gp->right;
            if (uncle->color == Color::Red) {  // caso 1
                if (verbose_) std::cout << "    [RB] insercao, caso 1 (tio vermelho) em torno de " << gp->key << "\n";
                setColor(z->parent, Color::Black);
                setColor(uncle, Color::Black);
                setColor(gp, Color::Red);
                z = gp;
            } else {
                if (z == z->parent->right) {  // caso 2
                    if (verbose_) std::cout << "    [RB] insercao, caso 2 (tio preto, zig-zag) em " << z->key << "\n";
                    z = z->parent;
                    rotateLeft(z);
                }
                if (verbose_) std::cout << "    [RB] insercao, caso 3 (tio preto, mesma linha) em torno de " << gp->key << "\n";
                setColor(z->parent, Color::Black);  // caso 3
                setColor(z->parent->parent, Color::Red);
                rotateRight(z->parent->parent);
            }
        } else {  // simetrico
            Node* uncle = gp->left;
            if (uncle->color == Color::Red) {
                if (verbose_) std::cout << "    [RB] insercao, caso 1 (tio vermelho) em torno de " << gp->key << "\n";
                setColor(z->parent, Color::Black);
                setColor(uncle, Color::Black);
                setColor(gp, Color::Red);
                z = gp;
            } else {
                if (z == z->parent->left) {
                    if (verbose_) std::cout << "    [RB] insercao, caso 2 (tio preto, zig-zag) em " << z->key << "\n";
                    z = z->parent;
                    rotateRight(z);
                }
                if (verbose_) std::cout << "    [RB] insercao, caso 3 (tio preto, mesma linha) em torno de " << gp->key << "\n";
                setColor(z->parent, Color::Black);
                setColor(z->parent->parent, Color::Red);
                rotateLeft(z->parent->parent);
            }
        }
    }
    setColor(root_, Color::Black);  // propriedade 2
}


void RBTree::transplant(Node* u, Node* v) {
    if (u->parent == nil_) root_ = v;
    else if (u == u->parent->left) u->parent->left = v;
    else u->parent->right = v;
    v->parent = u->parent; 
}

bool RBTree::remove(Key k) {
    Node* z = findNode(k);
    if (z == nil_) return false;  // inexistente: nada muda

    Node* y = z;
    Color yOriginal = y->color;
    Node* x;
    if (z->left == nil_) {
        x = z->right;
        transplant(z, z->right);
    } else if (z->right == nil_) {
        x = z->left;
        transplant(z, z->left);
    } else {
        y = minimum(z->right);  // sucessor em ordem
        yOriginal = y->color;
        x = y->right;
        if (y->parent == z) {
            x->parent = y;
        } else {
            transplant(y, y->right);
            y->right = z->right;
            y->right->parent = y;
        }
        transplant(z, y);
        y->left = z->left;
        y->left->parent = y;
        setColor(y, z->color);  // o sucessor herda a cor de z
    }
    delete z;
    --size_;
    if (yOriginal == Color::Black) deleteFixup(x);
    return true;
}

void RBTree::deleteFixup(Node* x) {
    while (x != root_ && x->color == Color::Black) {
        if (x == x->parent->left) {
            Node* w = x->parent->right;  // irmao (nunca e nil_ aqui)
            if (w->color == Color::Red) {  // caso 1: irmao vermelho
                if (verbose_) std::cout << "    [RB] remocao, caso 1 (irmao vermelho) em " << w->key << "\n";
                setColor(w, Color::Black);
                setColor(x->parent, Color::Red);
                rotateLeft(x->parent);
                w = x->parent->right;
            }
            if (w->left->color == Color::Black && w->right->color == Color::Black) {
                if (verbose_) std::cout << "    [RB] remocao, caso 2 (irmao preto, sobrinhos pretos) em " << w->key << "\n";
                setColor(w, Color::Red);  // caso 2: empurra o "duplo preto" para cima
                x = x->parent;
            } else {
                if (w->right->color == Color::Black) {  // caso 3: sobrinho distante preto
                    if (verbose_) std::cout << "    [RB] remocao, caso 3 (sobrinho proximo vermelho) em " << w->key << "\n";
                    setColor(w->left, Color::Black);
                    setColor(w, Color::Red);
                    rotateRight(w);
                    w = x->parent->right;
                }
                if (verbose_) std::cout << "    [RB] remocao, caso 4 (sobrinho distante vermelho) em " << w->key << "\n";
                setColor(w, x->parent->color);  // caso 4: termina
                setColor(x->parent, Color::Black);
                setColor(w->right, Color::Black);
                rotateLeft(x->parent);
                x = root_;
            }
        } else {  // simetrico
            Node* w = x->parent->left;
            if (w->color == Color::Red) {
                if (verbose_) std::cout << "    [RB] remocao, caso 1 (irmao vermelho) em " << w->key << "\n";
                setColor(w, Color::Black);
                setColor(x->parent, Color::Red);
                rotateRight(x->parent);
                w = x->parent->left;
            }
            if (w->right->color == Color::Black && w->left->color == Color::Black) {
                if (verbose_) std::cout << "    [RB] remocao, caso 2 (irmao preto, sobrinhos pretos) em " << w->key << "\n";
                setColor(w, Color::Red);
                x = x->parent;
            } else {
                if (w->left->color == Color::Black) {
                    if (verbose_) std::cout << "    [RB] remocao, caso 3 (sobrinho proximo vermelho) em " << w->key << "\n";
                    setColor(w->right, Color::Black);
                    setColor(w, Color::Red);
                    rotateLeft(w);
                    w = x->parent->left;
                }
                if (verbose_) std::cout << "    [RB] remocao, caso 4 (sobrinho distante vermelho) em " << w->key << "\n";
                setColor(w, x->parent->color);
                setColor(x->parent, Color::Black);
                setColor(w->left, Color::Black);
                rotateRight(x->parent);
                x = root_;
            }
        }
    }
    setColor(x, Color::Black);
}

// Busca, percurso, altura e impressao

bool RBTree::search(Key k) const { return findNode(k) != nil_; }

void RBTree::inOrderRec(const Node* x, std::vector<Key>& out) const {
    if (x == nil_) return;
    inOrderRec(x->left, out);
    out.push_back(x->key);
    inOrderRec(x->right, out);
}

std::vector<Key> RBTree::inOrder() const {
    std::vector<Key> out;
    out.reserve(size_);
    inOrderRec(root_, out);
    return out;
}

int RBTree::heightRec(const Node* x) const {
    if (x == nil_) return 0;
    return 1 + std::max(heightRec(x->left), heightRec(x->right));
}

void RBTree::printRec(const Node* x, int depth, std::ostream& os) const {
    if (x == nil_) return;
    printRec(x->right, depth + 1, os);
    bool red = (x->color == Color::Red);
    std::string label = std::to_string(x->key) + (red ? "(R)" : "(B)");
    if (g_useColor && red) label = "\033[31m" + label + "\033[0m";
    os << std::string(static_cast<std::size_t>(depth) * 8, ' ') << label << "\n";
    printRec(x->left, depth + 1, os);
}

void RBTree::print(std::ostream& os) const {
    if (root_ == nil_) {
        os << "(arvore vazia)\n";
        return;
    }
    printRec(root_, 0, os);
}

// Validacao das propriedades da Rubro-Negra 

bool RBTree::validateRec(const Node* x, const Key* lo, const Key* hi,
                         std::string& msg, int& blackHeight) const {
    if (x == nil_) {
        blackHeight = 1;
        return true;
    }
    if ((lo && x->key <= *lo) || (hi && x->key >= *hi)) {
        msg = "ordem BST violada no no " + std::to_string(x->key);
        return false;
    }
    if (x->color == Color::Red &&
        (x->left->color == Color::Red || x->right->color == Color::Red)) {
        msg = "dois nos vermelhos consecutivos em " + std::to_string(x->key);
        return false;
    }
    if ((x->left != nil_ && x->left->parent != x) ||
        (x->right != nil_ && x->right->parent != x)) {
        msg = "ponteiro de pai inconsistente em " + std::to_string(x->key);
        return false;
    }
    int bl = 0, br = 0;
    if (!validateRec(x->left, lo, &x->key, msg, bl)) return false;
    if (!validateRec(x->right, &x->key, hi, msg, br)) return false;
    if (bl != br) {
        msg = "altura-negra diferente entre as subarvores de " + std::to_string(x->key);
        return false;
    }
    blackHeight = bl + (x->color == Color::Black ? 1 : 0);
    return true;
}

bool RBTree::validate(std::string& msg) const {
    if (nil_->color != Color::Black) {
        msg = "sentinela NIL nao e preta";
        return false;
    }
    if (root_ != nil_ && root_->color != Color::Black) {
        msg = "a raiz nao e preta";
        return false;
    }
    if (root_ != nil_ && root_->parent != nil_) {
        msg = "o pai da raiz deveria ser NIL";
        return false;
    }
    int bh = 0;
    if (!validateRec(root_, nullptr, nullptr, msg, bh)) return false;
    msg.clear();
    return true;
}

// Liberacao de memoria

void RBTree::destroy(Node* x) {
    if (x == nil_) return;
    destroy(x->left);
    destroy(x->right);
    delete x;
}

void RBTree::clear() {
    destroy(root_);
    root_ = nil_;
    nil_->left = nil_->right = nil_->parent = nil_;
    size_ = 0;
}

}
