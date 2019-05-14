#include "minheap.h"


void MinHeap::moveDown(int start, int end)
{
    int cPos = start;
    int lPos = 2 * cPos + 1;
    Tuple tmp = minHeapArray_[cPos];
    while(lPos <= end)
    {
        if (lPos < end && minHeapArray_[lPos].distance > minHeapArray_[lPos+1].distance)
            lPos++;
        if (minHeapArray_[lPos].distance >= minHeapArray_[cPos].distance)
            break;
        else
        {
            minHeapArray_[cPos] = minHeapArray_[lPos];
            location_[minHeapArray_[lPos].vertex] = cPos;
            cPos = lPos;
            lPos = lPos *2 + 1;
        }
    }
    location_[tmp.vertex] = cPos;
    minHeapArray_[cPos] = tmp;
}


void MinHeap::moveUp(int start)
{

    int cPos = start;
    int pPos = (cPos-1) / 2;
    Tuple tmp = minHeapArray_[cPos];
    while(cPos > 0)
    {
        if (minHeapArray_[pPos].distance <= tmp.distance)
            break;
        else
        {
            minHeapArray_[cPos] = minHeapArray_[pPos];
            location_[minHeapArray_[pPos].vertex] = cPos;
            cPos = pPos;
            pPos = (pPos-1) / 2;
        }
    }
    location_[tmp.vertex] = cPos;
    minHeapArray_[cPos] = tmp;

}


int MinHeap::insert(Tuple data)
{

    if (size_ == capacity_)
        return -1;
    minHeapArray_[size_] = data;
    moveUp(size_);
    size_ += 1;
    return 1;
}


int MinHeap::remove(Tuple data)
{
    int index;
    if (size_ == 0)
        return -1;
    index = this->find(data.vertex);
    location_.erase(minHeapArray_[index].vertex);
    minHeapArray_[index] = minHeapArray_[--size_];
    this->moveDown(index,size_-1);
    return 1;
}


int MinHeap::find(int vetex)
{
    if (location_.count(vetex) == 0 )
        return -1;
    return location_[vetex];
}

int MinHeap::replace(Tuple data)
{
    if (this->find(data.vertex)==-1)
        return -1;
    else
    {
        int index = location_[data.vertex];
        location_.erase(minHeapArray_[index].vertex);
        minHeapArray_[index] = minHeapArray_[--size_];
        this->moveDown(index,size_-1);
        this->insert(data);
    }

    return 1;
}


void MinHeap::pop()
{
    location_.erase(minHeapArray_[0].vertex);
    if (size_ == 1)
    {
        delete[] minHeapArray_;
        minHeapArray_ = new Tuple[this->capacity_];
        size_ = 0;
    }
    else
    {
        minHeapArray_[0] = minHeapArray_[--size_];
        this->moveDown(0,size_-1);
    }
}

bool MinHeap::empty()
{
    return size_ == 0 ? true:false;
}

int MinHeap::length()
{
    return size_;
}

void MinHeap::print()
{
    cout << "Queue Value:" << endl;
    for (int i=0; i<size_; i++)
    {
        cout << minHeapArray_[i].vertex << " " << minHeapArray_[i].distance << " " << endl;
    }
    cout << "Dict Value:" << endl;
    for (auto kv:this->location_)
    {
        cout << kv.first << " " << kv.second <<endl;
    }
}


Tuple MinHeap::top()
{
    return minHeapArray_[0];
}
