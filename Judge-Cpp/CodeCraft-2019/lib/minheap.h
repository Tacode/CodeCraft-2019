#ifndef MINHEAP_H
#define MINHEAP_H
#include <iostream>
#include <vector>
#include <map>
using namespace std;

class Tuple{
public:
    Tuple(){;}
    Tuple(int v, double d):vertex(v), distance(d){;}
public:
    int vertex;
    double distance;

};


class MinHeap
{
public:
    MinHeap()
    {
        minHeapArray_ = new Tuple[300];
        capacity_ = 300;
        size_ = 0;
    }
    MinHeap(int capacity)
    {
        minHeapArray_ = new Tuple[capacity];
        size_ = 0;
        capacity_ = capacity;
    }
    ~MinHeap()
    {
        delete[] minHeapArray_;
        size_ = 0;
        capacity_ = 0;
    }

public:
    int insert(Tuple data);
    int remove(Tuple data);
    int find(int vertex);
    int replace(Tuple data);
    Tuple top();
    void pop();
    void print();
    bool empty();
    int length();
private:
    void moveDown(int start, int end);
    void moveUp(int start);
    Tuple* minHeapArray_;
    int capacity_;
    int size_;
    map<int,int> location_;
};


#endif // MINHEAP_H
