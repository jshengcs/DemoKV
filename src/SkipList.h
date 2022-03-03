#ifndef SKIP_LIST_H_
#define SKIP_LIST_H_

template <typename K, typename V>
class SkipList {
public:
    SkipList(){};
    virtual ~SkipList(){};
    virtual bool add(K key, V value) = 0;
    virtual void display() = 0;
    virtual bool search(K key) = 0;
    virtual bool remove(K key) = 0;
    virtual int size() = 0;
    virtual int realSize() = 0;
};

#endif