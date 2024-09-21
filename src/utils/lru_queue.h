#include <iostream>
#include <unordered_map>

// 双向链表节点结构体
template <typename T1, typename T2>
struct DListNode {
    T1 key;
    T2 value;
    DListNode<T1, T2>* prev;
    DListNode<T1, T2>* next;
    DListNode() : prev(nullptr), next(nullptr) {}
    DListNode(T1 k, T2 val) : key(k), value(val), prev(nullptr), next(nullptr) {}
};

// LRU 缓存类
template <typename T1, typename T2>
class LRUQueue {
public:
    LRUQueue(int cap) : capacity(cap), size(0) {
        head = new DListNode<T1, T2>();
        tail = new DListNode<T1, T2>();
        head->next = tail;
        tail->prev = head;
    }

    bool contains(T1 key)
    {
        if (cache.find(key)!= cache.end())
            return true;
        return false;
    }

    // 获取元素，如果存在则移到链表头部并返回值，否则返回默认值
    T2 operator[](T1 key) {
        if (cache.find(key)!= cache.end()) {
            DListNode<T1, T2>* node = cache[key];
            moveToHead(node);
            return node->value;
        }
        return T2();
    }

    // 放入元素，如果元素已存在则移到链表头部，如果不存在且队列已满，则删除尾部元素后插入新元素到头部
    void insert(T1 key, T2 value) {
        if (cache.find(key)!= cache.end()) {
            DListNode<T1, T2>* node = cache[key];
            moveToHead(node);
        } else {
            if (size == capacity) {
                removeTail();
            }
            DListNode<T1, T2>* newNode = new DListNode<T1, T2>(key, value);
            newNode->next = head;
            head->prev = newNode;
            head = newNode;
            cache[key] = newNode;
            size++;
        }
    }

private:
    // 将节点移到链表头部
    void moveToHead(DListNode<T1, T2>* node) {
        if (node == head) return;
        if (node == tail) {
            tail = tail->prev;
            tail->next = nullptr;
        } else {
            node->prev->next = node->next;
            node->next->prev = node->prev;
        }
        node->next = head;
        head->prev = node;
        node->prev = nullptr;
        head = node;
    }

    // 删除链表尾部节点
    void removeTail() {
        DListNode<T1, T2>* removed = tail;
        tail = tail->prev;
        if (tail) {
            tail->next = nullptr;
        }
        cache.erase(removed->key);
        delete removed;
        size--;
    }

private:
    int capacity;
    int size;
    DListNode<T1, T2>* head;
    DListNode<T1, T2>* tail;
    std::unordered_map<T1, DListNode<T1, T2>*> cache;

};
