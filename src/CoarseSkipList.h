#ifndef COARSE_SKIP_LIST_H_
#define COARSE_SKIP_LIST_H_

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <mutex>
#include <string>

#include "Log.h"
#include "SkipList.h"

template <typename K, typename V>
class ListNode {
public:
    ListNode();
    ListNode(K key, V value, int level);
    ~ListNode();
    K getKey();
    V getValue();
    void setValue(V value);
    // next节点指针的数组
    ListNode<K, V> **nexts_;
    int node_level_;

private:
    K key_;
    V value_;
};

template <typename K, typename V>
class CoarseSkipList : public SkipList<K, V> {
public:
    CoarseSkipList(int level);
    virtual ~CoarseSkipList();
    virtual bool add(K key, V value);
    virtual void display();
    virtual bool search(K key);
    virtual bool remove(K key);
    virtual int size();
    virtual int realSize();

private:
    std::mutex mutex_;
    int count_;
    int max_level_;
    int cur_level_;
    ListNode<K, V> *head_;
    int getRandomLevel();
    ListNode<K, V> *createNode(K key, V value, int);
};

template <typename K, typename V>
ListNode<K, V>::ListNode(K key, V value, int level) {
    key_ = key;
    value_ = value;
    node_level_ = level;
    // 0~level
    nexts_ = new ListNode<K, V> *[level + 1];
    memset(nexts_, 0, sizeof(ListNode<K, V> *) * (level + 1));
};

template <typename K, typename V>
ListNode<K, V>::~ListNode() {
    delete[] nexts_;
}

template <typename K, typename V>
K ListNode<K, V>::getKey() {
    return key_;
};

template <typename K, typename V>
V ListNode<K, V>::getValue() {
    return value_;
};

template <typename K, typename V>
void ListNode<K, V>::setValue(V value) {
    value_ = value;
};

template <typename K, typename V>
CoarseSkipList<K, V>::CoarseSkipList(int max_level) {
    max_level_ = max_level;
    cur_level_ = 0;
    count_ = 0;
    //构造head节点
    K k;
    V v;
    head_ = new ListNode<K, V>(k, v, max_level_ + 1);
    std::cout << "CoarseSkipList" << '\n';
};

template <typename K, typename V>
CoarseSkipList<K, V>::~CoarseSkipList() {
    ListNode<K, V> *curr = head_;
    ListNode<K, V> *next = head_;
    //循环删除
    while (curr->nexts_[0] != NULL) {
        next = curr->nexts_[0];
        delete curr;
        curr = next;
    }
    delete curr;
};

// 搜索节点
/*
                           +------------+
                           |  select 60 |
                           +------------+
level 4     +-->1+                                                      100
                 |
                 |
level 3         1+-------->10+-------------->50+        70   100
                                               |
                                               |
level 2         1          10     30         50|        70   100
                                               |
                                               |
level 1         1    4     10     30         50|        70   100
                                               |
                                               |
level 0         1    4   9 10     30   40    50+-->60   70   100
*/
template <typename K, typename V>
bool CoarseSkipList<K, V>::search(K key) {
    ListNode<K, V> *curr = head_;
    for (int i = cur_level_; i >= 0; i--) {
        while (curr->nexts_[i] && curr->nexts_[i]->getKey() < key) {
            curr = curr->nexts_[i];
        }
    }

    curr = curr->nexts_[0];
    if (curr && curr->getKey() == key) {
        return true;
    }

    return false;
};

template <typename K, typename V>
bool CoarseSkipList<K, V>::add(K key, V value) {
    std::lock_guard<std::mutex> guard(mutex_);
    ListNode<K, V> *curr = head_;
    // pre节点指针数组
    ListNode<K, V> *pres[max_level_ + 1];
    memset(pres, 0, sizeof(ListNode<K, V> *) * (max_level_ + 1));

    for (int i = cur_level_; i >= 0; i--) {
        while (curr->nexts_[i] && curr->nexts_[i]->getKey() < key) {
            curr = curr->nexts_[i];
        }
        pres[i] = curr;
    }

    curr = curr->nexts_[0];
    if (curr && curr->getKey() == key) {
        // std::cout << "--------exists----------"
        //<< "\n";
        return false;
    }

    if (curr == nullptr || curr->getKey() != key) {
        int level = getRandomLevel();
        if (level > cur_level_) {
            for (int i = cur_level_ + 1; i <= level; i++) {
                pres[i] = head_;
            }
            cur_level_ = level;
        }

        ListNode<K, V> *newNode = createNode(key, value, level);
        for (int i = 0; i <= level; i++) {
            newNode->nexts_[i] = pres[i]->nexts_[i];
            pres[i]->nexts_[i] = newNode;
        }
        count_++;
    }
    return true;
};

template <typename K, typename V>
bool CoarseSkipList<K, V>::remove(K key) {
    std::lock_guard<std::mutex> guard(mutex_);
    ListNode<K, V> *curr = head_;
    ListNode<K, V> *pres[max_level_ + 1];
    memset(pres, 0, sizeof(ListNode<K, V> *) * (max_level_ + 1));

    for (int i = cur_level_; i >= 0; i--) {
        while (curr->nexts_[i] && curr->nexts_[i]->getKey() < key) {
            curr = curr->nexts_[i];
        }
        pres[i] = curr;
    }

    curr = curr->nexts_[0];

    if (curr && curr->getKey() == key) {
        //自底向上
        for (int i = 0; i <= cur_level_; i++) {
            if (pres[i]->nexts_[i] != curr) {
                break;
            }

            pres[i]->nexts_[i] = curr->nexts_[i];
        }
        delete curr;
    } else {
        return false;
    }

    while (cur_level_ > 0 && head_->nexts_[cur_level_] == nullptr) {
        cur_level_--;
    }

    return true;
}

template <typename K, typename V>
void CoarseSkipList<K, V>::display() {
    std::string str;
    std::cout << "\n*****Skip List*****"
              << "\n";
    std::cout << "cur_level: " << cur_level_ << "\n";
    for (int i = 0; i <= cur_level_; i++) {
        ListNode<K, V> *node = this->head_->nexts_[i];
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

template <typename K, typename V>
int CoarseSkipList<K, V>::size() {
    return count_;
};

template <typename K, typename V>
int CoarseSkipList<K, V>::getRandomLevel() {
    int k = 0;
    while (rand() % 2) {
        k++;
    }
    k = (k < max_level_) ? k : max_level_;
    return k;
};

template <typename K, typename V>
ListNode<K, V> *CoarseSkipList<K, V>::createNode(K key, V value, int level) {
    return new ListNode<K, V>(key, value, level);
};

template <typename K, typename V>
int CoarseSkipList<K, V>::realSize() {
    int length = 0;
    ListNode<K, V> *curr = head_;
    if (curr->nexts_[0] == NULL) return 0;
    while (curr->nexts_[0] != NULL) {
        curr = curr->nexts_[0];
        length++;
    }
    return length;
}

#endif