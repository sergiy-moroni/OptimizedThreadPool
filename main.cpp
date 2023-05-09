#include <future>
#include <chrono>
#include <iostream>
#include <random>
#include "OptimizedThreadPool.h"

using namespace std;
bool make_thread = false;

RequestHandler pool;

//проверка работы сортировки
void PrintArr(int* arr, unsigned int arrsize){
    cout << endl;
    for (int i = 0; i < arrsize; i+=1000000) {
        cout << arr[i] << " ";
}
    cout << endl;
}

void quicksort(int* array, int left, int right) {
    if (left >= right) return;

    int left_bound = left;
    int right_bound = right;

    int middle = array[(left_bound + right_bound) / 2];

    do {
        while (array[left_bound] < middle) {
            left_bound++;
        }
        while (array[right_bound] > middle) {
            right_bound--;
        }

        //Меняем элементы местами
        if (left_bound <= right_bound) {
            std::swap(array[left_bound], array[right_bound]);
            left_bound++;
            right_bound--;
        }
    } while (left_bound <= right_bound);

    if (make_thread && (right_bound - left > 100000))
    {
        pool.pushRequest(quicksort, array, left, right_bound);
        quicksort(array, left_bound, right);
     
      
    }
    else {
        // запускаем обе части синхронно
        quicksort(array, left, right_bound);
        quicksort(array, left_bound, right);
    }
}

int main() {
    cout << "Number of cores: " << thread::hardware_concurrency() << endl;

    time_t start, end;
    //сортировка однопоточная
    time(&start);
   unsigned long arrsize = 30'000'000;
    int* arr = new int[arrsize];
    for (int i = 0; i < arrsize; i++) {
        arr[i] = rand() / 100;
    }
    cout << "array constructed" << endl;
    cout << endl;
   PrintArr(arr, arrsize);

    cout << "array solo sorting..." << endl;
    quicksort(arr, 0, arrsize - 1);

    cout << "array sorted by solo threads" << endl;
    PrintArr(arr, arrsize);
    time(&end);
    double seconds = difftime(end, start);
    printf("The time: %f seconds\n", seconds);

    //Сортировка многопоточная
    time(&start);
    make_thread = true;
    for (unsigned int i = 0; i < arrsize; i++) {
        arr[i] = rand() / 100;
    }
    cout << "array constructed" << endl;
   PrintArr(arr, arrsize);
    cout << endl;

    cout << "array pool thread sorting..." << endl;
   quicksort(arr, 0, arrsize - 1);
    cout << "array sorted by pool thread" << endl;
    
   PrintArr(arr, arrsize);
    time(&end);
    seconds = difftime(end, start);
    printf("The time: %f seconds\n", seconds);

    delete[] arr;
    return 0;
}
