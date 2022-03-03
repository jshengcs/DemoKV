#ifndef OPTIMISTIC_SKIP_LIST_H_
#define OPTIMISTIC_SKIP_LIST_H_

#include <limits.h>
#include <pthread.h>
#include <stdlib.h>

#include <atomic>
#include <ctime>
#include <mutex>

#include "Log.h"

template <typename K, typename V>
class Node {
public:
    // sentinel node constructor
    // Node( int key);
    // Node(K key, V value);
    // Node( int x, int height);
    Node();
    Node(K key, int level);
    Node(K key, V value, int level);
    ~Node();
    K getKey();
    V getValue();

    Node **nexts_;  // next节点数组
    K key_;
    V value_;
    int node_level_;
    bool marked_;        //逻辑删除标记
    bool fully_linked_;  // 抽象集标记

    std::mutex mutex_;
};

template <typename K, typename V>
class OptimisticSkipList : public SkipList<K, V> {
public:
    OptimisticSkipList(int max_level);
    virtual ~OptimisticSkipList();

    virtual int size();
    Node<K, V> *getKey(K key);
    V *getValue(K key);
    virtual bool search(K key);
    virtual bool remove(K key);
    virtual bool add(K key, V value);
    virtual void display();
    virtual int realSize();

private:
    Node<K, V> *head_;
    // Node<K, V> *tail_;
    //  std::mutex mutex_;
    std::atomic<int> count_;
    int max_level_;
    int cur_level_;
    int getRandomLevel();
    int find(K, Node<K, V> **pres, Node<K, V> **succs);
};

template <typename K, typename V>
OptimisticSkipList<K, V>::OptimisticSkipList(int max_level) {
    max_level_ = max_level;
    cur_level_ = 0;
    count_ = 0;
    //构造head节点
    K k;
    V v;
    head_ = new Node<K, V>(k, v, max_level_ + 1);
    std::cout << "OptimisticSkipList" << '\n';
};

template <typename K, typename V>
OptimisticSkipList<K, V>::~OptimisticSkipList() {
    Node<K, V> *curr = head_;
    Node<K, V> *next = head_;
    //循环删除
    while (curr->nexts_[0] != nullptr) {
        next = curr->nexts_[0];
        delete curr;
        curr = next;
    }
    delete curr;
};

template <typename K, typename V>
int OptimisticSkipList<K, V>::size() {
    return count_;
};

template <typename K, typename V>
bool OptimisticSkipList<K, V>::search(K key) {
    Node<K, V> *pres[max_level_ + 1];
    Node<K, V> *succs[max_level_ + 1];
    memset(pres, 0, sizeof(Node<K, V> *) * (max_level_ + 1));
    memset(succs, 0, sizeof(Node<K, V> *) * (max_level_ + 1));
    int lFound = find(key, pres, succs);
    return (lFound != -1 && succs[lFound]->fully_linked_ &&
            !succs[lFound]->marked_);
};

template <typename K, typename V>
int OptimisticSkipList<K, V>::find(K key, Node<K, V> **pres,
                                   Node<K, V> **succs) {
    K l_key = key;  // hash will change value to key ******
    int lFound = -1;
    Node<K, V> *pred = head_;
    for (int i = max_level_; i >= 0; i--) {
        Node<K, V> *curr = (pred->nexts_)[i];
        while (curr && l_key > curr->key_) {
            pred = curr;
            curr = (pred->nexts_)[i];
        }
        if (curr && lFound == -1 && l_key == curr->key_) {
            lFound = i;
        }
        pres[i] = pred;
        succs[i] = curr;
    }
    return lFound;
};

template <typename K, typename V>
bool OptimisticSkipList<K, V>::add(K key, V value) {
    int node_level = getRandomLevel();
    Node<K, V> *pres[max_level_ + 1];
    Node<K, V> *succs[max_level_ + 1];
    memset(pres, 0, sizeof(Node<K, V> *) * (max_level_ + 1));
    memset(succs, 0, sizeof(Node<K, V> *) * (max_level_ + 1));

    while (true) {
        int lFound = find(key, pres, succs);
        // LOG << "Find key:" << key << " " << lFound << "\n";
        //存在该节点，等待完全链接后返回
        if (lFound != -1) {
            Node<K, V> *nodeFound = succs[lFound];
            if (!nodeFound->marked_) {
                while (!nodeFound->fully_linked_) {
                }
                // std::cout << "--------not fully linked or exists----------"
                //           << "\n";
                return false;
            }
            continue;
        }
        int highestLocked = -1;
        Node<K, V> *pred, *succ;
        bool valid = true;
        Node<K, V> *previous = nullptr;
        for (int i = 0; valid && (i <= node_level); i++) {
            pred = pres[i];
            succ = succs[i];
            if (previous != pres[i]) {
                pred->mutex_.lock();
            }
            previous = pres[i];
            highestLocked = i;
            valid = !pred->marked_ && valid && pred->nexts_[i] == succ;
        }
        if (!valid) {
            Node<K, V> *previous = nullptr;
            for (int i = 0; i <= highestLocked; i++) {
                if (previous == nullptr || previous != pres[i]) {
                    pres[i]->mutex_.unlock();
                }
                previous = pres[i];
            }
            continue;
        }
        count_++;
        Node<K, V> *newNode = new Node<K, V>(key, value, node_level);
        if (cur_level_ < node_level) cur_level_ = node_level;
        for (int i = 0; i <= node_level; i++) newNode->nexts_[i] = succs[i];
        for (int i = 0; i <= node_level; i++) pres[i]->nexts_[i] = newNode;
        newNode->fully_linked_ = true;
        previous = nullptr;
        for (int i = 0; i <= highestLocked; i++) {
            if (previous == nullptr || previous != pres[i])
                pres[i]->mutex_.unlock();
            previous = pres[i];
        }
        return true;
    }
};

template <typename K, typename V>
V *OptimisticSkipList<K, V>::getValue(K key) {
    Node<K, V> *pres[max_level_ + 1];
    Node<K, V> *succs[max_level_ + 1];
    memset(pres, 0, sizeof(Node<K, V> *) * (max_level_ + 1));
    memset(succs, 0, sizeof(Node<K, V> *) * (max_level_ + 1));
    int lFound = find(key, pres, succs);
    if (lFound != -1) {
        Node<K, V> *target = succs[lFound];
        if (target != nullptr) {
            target->mutex_.lock();
            if ((lFound != -1 &&
                 (target->fully_linked_ && target->node_level_ == lFound &&
                  !target->marked_))) {
                target->mutex_.unlock();
                return &(target->value_);
            }
            target->mutex_.unlock();
            return nullptr;
        }
        return nullptr;
    }
    return nullptr;
}

template <typename K, typename V>
Node<K, V> *OptimisticSkipList<K, V>::getKey(K key) {
    Node<K, V> *pres[max_level_ + 1];
    Node<K, V> *succs[max_level_ + 1];
    memset(pres, 0, sizeof(Node<K, V> *) * (max_level_ + 1));
    memset(succs, 0, sizeof(Node<K, V> *) * (max_level_ + 1));
    int lFound = find(key, pres, succs);
    if (lFound != -1) {
        Node<K, V> *target = succs[lFound];
        if (target != nullptr) {
            target->mutex_.lock();
            if ((lFound != -1 &&
                 (target->fully_linked_ && target->node_level_ == lFound &&
                  !target->marked_))) {
                target->mutex_.unlock();
                return target;
            }
            target->mutex_.unlock();
            return nullptr;
        }
        return nullptr;
    }
    return nullptr;
};

template <typename K, typename V>
bool OptimisticSkipList<K, V>::remove(K key) {
    Node<K, V> *victim;
    bool is_marked = false;
    int top_level = -1;
    Node<K, V> *pres[max_level_ + 1];
    Node<K, V> *succs[max_level_ + 1];
    memset(pres, 0, sizeof(Node<K, V> *) * (max_level_ + 1));
    memset(succs, 0, sizeof(Node<K, V> *) * (max_level_ + 1));

    while (true) {
        int lFound = find(key, pres, succs);
        if (lFound != -1) {
            victim = succs[lFound];
        }
        if (is_marked | (lFound != -1 &&
                         (victim->fully_linked_ &&
                          victim->node_level_ == lFound && !victim->marked_))) {
            if (!is_marked) {
                top_level = victim->node_level_;
                victim->mutex_.lock();
                if (victim->marked_) {
                    victim->mutex_.unlock();
                    return false;
                }
                victim->marked_ = true;
                is_marked = true;
            }
            int highestLocked = -1;
            Node<K, V> *pred;  //, *succ;
            bool valid = true;
            Node<K, V> *previous = nullptr;
            for (int i = 0; valid && (i <= top_level); i++) {
                pred = pres[i];
                if (previous != pres[i]) pred->mutex_.lock();
                previous = pres[i];
                highestLocked = i;
                valid = !pred->marked_ && pred->nexts_[i] == victim;
            }
            if (!valid) {
                Node<K, V> *previous = nullptr;
                for (int i = 0; i <= highestLocked; i++) {
                    if (previous == nullptr || previous != pres[i])
                        pres[i]->mutex_.unlock();
                    previous = pres[i];
                }
                continue;
            }
            count_--;
            for (int i = top_level; i >= 0; i--) {
                pres[i]->nexts_[i] = victim->nexts_[i];
            }
            victim->mutex_.unlock();
            previous = nullptr;
            for (int i = 0; i <= highestLocked; i++) {
                if (previous == nullptr || previous != pres[i])
                    pres[i]->mutex_.unlock();
                previous = pres[i];
            }
            return true;
        } else {
            return false;
        }
    }
};

template <typename K, typename V>
int OptimisticSkipList<K, V>::getRandomLevel() {
    int k = 0;
    while (rand() % 2) {
        k++;
    }
    k = (k < max_level_) ? k : max_level_;
    // cout << "k: " << k << '\n';
    return k;
};

// sentinel node constructor
template <typename K, typename V>
Node<K, V>::Node(K key, int level) {
    key_ = key;
    value_ = nullptr;
    nexts_ = new Node *[level + 1];
    node_level_ = level;
    marked_ = false;
    fully_linked_ = false;
};

template <typename K, typename V>
Node<K, V>::Node(K key, V value, int level) {
    value_ = value;
    key_ = key;
    nexts_ = new Node *[level + 1];
    node_level_ = level;
    marked_ = false;
    fully_linked_ = false;
};

template <typename K, typename V>
K Node<K, V>::getKey() {
    return key_;
};

template <typename K, typename V>
V Node<K, V>::getValue() {
    return value_;
}

template <typename K, typename V>
Node<K, V>::~Node() {
    delete[] nexts_;
};

template <typename K, typename V>
int OptimisticSkipList<K, V>::realSize() {
    int length = 0;
    Node<K, V> *curr = head_;
    if (curr->nexts_[0] == NULL) return 0;
    while (curr->nexts_[0] != NULL) {
        curr = curr->nexts_[0];
        length++;
    }
    return length;
}

template <typename K, typename V>
void OptimisticSkipList<K, V>::display() {
    std::string str;
    std::cout << "\n*****Skip List*****"
              << "\n";
    std::cout << "cur_level: " << cur_level_ << "\n";
    for (int i = 0; i <= cur_level_; i++) {
        Node<K, V> *node = this->head_->nexts_[i];
        std::cout << "Level " << i << ": ";
        while (node != NULL) {
            str += node->getKey() + node->getValue();
            // std::cout << node->getKey() << ":" << node->getValue() << ";";
            node = node->nexts_[i];
        }
        std::cout << '\n';
    }
    std::cout << "-----------str.size(): " << str.size() << '\n';
};

#endif