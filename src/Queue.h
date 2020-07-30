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

#ifndef QUEUE_H
#define QUEUE_H

#ifndef QUEUE_MAXITEMS
#define QUEUE_MAXITEMS 8
#endif

#include <stdint.h>

template<class T>
class Queue {
private:
    uint16_t front_, back_, count_;
    T data_[QUEUE_MAXITEMS + 1];
    uint16_t maxitems_;
public:
    explicit Queue(uint16_t maxitems = 8) {
        front_ = 0;
        back_ = 0;
        count_ = 0;
        maxitems_ = maxitems;
        // data_ = T[QUEUE_MAXITEMS + 1];
    }
    ~Queue() {
        delete[] data_;
    }
    inline uint16_t count();
    inline uint16_t front();
    inline uint16_t back();
    void push(const T &item);
    T peek();
    T pop();
    void clear();
};

template<class T>
inline uint16_t Queue<T>::count()
{
    return count_;
}

template<class T>
inline uint16_t Queue<T>::front()
{
    return front_;
}

template<class T>
inline uint16_t Queue<T>::back()
{
    return back_;
}

template<class T>
void Queue<T>::push(const T &item)
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
T Queue<T>::pop() {
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
T Queue<T>::peek() {
    if(count_ <= 0) return T(); // Returns empty
    else return data_[front_];
}

template<class T>
void Queue<T>::clear()
{
    front_ = back_;
    count_ = 0;
}

#endif //QUEUE_H
