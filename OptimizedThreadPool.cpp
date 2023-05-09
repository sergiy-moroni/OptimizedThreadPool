#include "OptimizedThreadPool.h"

OptimizedThreadPool::OptimizedThreadPool() :
   m_thread_count(thread::hardware_concurrency() != 0 ? thread::hardware_concurrency() : 4)
    ,
    m_thread_queues(m_thread_count) {}

void OptimizedThreadPool::start() {
    for (int i = 0; i < m_thread_count; i++) {
        m_threads.emplace_back(&OptimizedThreadPool::threadFunc, this, i);
    }
}

void OptimizedThreadPool::stop() {
    for (int i = 0; i < m_thread_count; i++) {
        // кладем задачу-пустышку в каждую очередь
        // для завершения потока
        task_type empty_task;
        m_thread_queues[i].push(empty_task);
    }
    for (auto& t : m_threads) {
        t.join();
    }

}

void OptimizedThreadPool::push_task(FuncType f, int* array, int left, int right) {

    // вычисляем индекс очереди, куда положим задачу
    int queue_to_push = m_index++ % m_thread_count;
    // формируем функтор
    task_type task = [=] {f(array, left, right);  };
    // кладем в очередь
    m_thread_queues[queue_to_push].push(task);
}

void OptimizedThreadPool::threadFunc(int qindex) {
    while (true) {
        // обработка очередной задачи
        task_type task_to_do;
        bool res;
        int i = 0;
        for (; i < m_thread_count; i++) {
            // попытка быстро забрать задачу из любой очереди, начиная со своей
            if (res = m_thread_queues[(qindex + i) % m_thread_count].fast_pop(task_to_do))
                break;
        }

        if (!res) {
            // вызываем блокирующее получение очереди
            m_thread_queues[qindex].pop(task_to_do);
        }
        else if (!task_to_do) {
            // чтобы не допустить зависания потока
            // кладем обратно задачу-пустышку
            m_thread_queues[(qindex + i) % m_thread_count].push(task_to_do);
        }
        if (!task_to_do) {
            return;
        }
        // выполняем задачу
        task_to_do();
    }
}
