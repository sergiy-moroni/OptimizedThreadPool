#pragma once
#include <queue>
#include <future>
#include <vector>
#include <iostream>
#include <mutex>
#include <functional>
#include <thread>

using namespace std;

typedef function<void()> task_type;
typedef future<void> res_type;

template<class T>
class BlockedQueue {
public:
    void push(T& item) {
        lock_guard<mutex> l(m_locker);
        // обычный потокобезопасный push
        m_task_queue.push(item);
        // делаем оповещение, чтобы поток, вызвавший
        // pop проснулся и забрал элемент из очереди
        m_notifier.notify_one();
    }
    // блокирующий метод получения элемента из очереди
    void pop(T& item) {
        unique_lock<mutex> l(m_locker);
        if (m_task_queue.empty())
            // ждем, пока вызовут push
            m_notifier.wait(l, [this] {return !m_task_queue.empty(); });
        item = m_task_queue.front();
        m_task_queue.pop();
    }
    // неблокирующий метод получения элемента из очереди
    // возвращает false, если очередь пуста
    bool fast_pop(T& item) {
        lock_guard<mutex> l(m_locker);
        if (m_task_queue.empty())
            // просто выходим
            return false;
        // забираем элемент
        item = m_task_queue.front();
        m_task_queue.pop();
        return true;
    }
private:
    mutex m_locker;
    // очередь задач
    queue<T> m_task_queue;
    // уведомитель
    condition_variable m_notifier;
};

// тип указатель на функцию, которая является эталоном для функций задач
typedef void (*FuncType) (int*, int, int);
// пул потоков
class OptimizedThreadPool {
public:
    OptimizedThreadPool();
    // запуск:
    void start();
    // остановка:
    void stop();
    // проброс задач
    void push_task(FuncType f, int* array, int left, int right);
    // функция входа для потока
    void threadFunc(int qindex);
private:
    int m_thread_count;
    // потоки
    vector<thread> m_threads;
    // очереди задач для потоков
    vector<BlockedQueue<task_type>> m_thread_queues;
    // для равномерного распределения задач
    int m_index;
};

class RequestHandler {
public:
    RequestHandler() {
        m_tpool.start();
    }
    ~RequestHandler() {
        m_tpool.stop();
    }
    void pushRequest(FuncType f, int* array, int left, int right) {
        m_tpool.push_task(f, array, left,right);
    }
private:
    // пул потоков
    OptimizedThreadPool m_tpool;
};
