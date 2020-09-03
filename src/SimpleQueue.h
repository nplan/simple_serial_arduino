/*
 * Queue.h
 *
 * By Steven de Salas
 *
 * Defines a templated (generic) class for a queue of things.
 * Used for Arduino projects, just #include "Queue.h" and add this file via the IDE.
 *
 * Examples:
 *
 * Queue<char> queue(10); // Max 10 chars in this queue
 * queue.push('H');
 * queue.push('e');
 * queue.count(); // 2
 * queue.push('l');
 * queue.push('l');
 * queue.count(); // 4
 * Serial.print(queue.pop()); // H
 * Serial.print(queue.pop()); // e
 * queue.count(); // 2
 * queue.push('o');
 * queue.count(); // 3
 * Serial.print(queue.pop()); // l
 * Serial.print(queue.pop()); // l
 * Serial.print(queue.pop()); // o
 *
 * struct Point { int x; int y; }
 * Queue<Point> points(5);
 * points.push(Point{2,4});
 * points.push(Point{5,0});
 * points.count(); // 2
 *
 */

#ifndef SIMPLE_QUEUE_H
#define SIMPLE_QUEUE_H

#include <stdint.h>

template<class T>
class SimpleQueue {
private:
    uint16_t front_, back_, count_, maxitems_;
    T *data_;
public:
    explicit SimpleQueue(uint16_t maxitems)
        : front_(0)
        , back_(0)
        , count_(0)
        , maxitems_(maxitems)
        , data_(new T[maxitems_+1])
        {}
    ~SimpleQueue() {
        delete [] data_;
    }
    uint16_t count();
    uint16_t front();
    uint16_t back();
    void push(const T &item);
    T peek();
    T pop();
    void clear();
};

template<class T>
inline uint16_t SimpleQueue<T>::count()
{
    return count_;
}

template<class T>
inline uint16_t SimpleQueue<T>::front()
{
    return front_;
}

template<class T>
inline uint16_t SimpleQueue<T>::back()
{
    return back_;
}

template<class T>
void SimpleQueue<T>::push(const T &item)
{
    if(count_ < maxitems_) { // Drops out when full
        data_[back_++]=item;
        ++count_;
        // Check wrap around
        if (back_ > maxitems_)
            back_ -= (maxitems_ + 1);
    }
}

template<class T>
T SimpleQueue<T>::pop() {
    if(count_ <= 0) return T(); // Returns empty
    else {
        T result = data_[front_];
        front_++;
        --count_;
        // Check wrap around
        if (front_ > maxitems_)
            front_ -= (maxitems_ + 1);
        return result;
    }
}

template<class T>
T SimpleQueue<T>::peek() {
    if(count_ <= 0) return T(); // Returns empty
    else return data_[front_];
}

template<class T>
void SimpleQueue<T>::clear()
{
    front_ = back_;
    count_ = 0;
}

#endif //SIMPLE_QUEUE_H
