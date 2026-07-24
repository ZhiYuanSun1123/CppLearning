#include <iomanip>
#include <iostream>

bool is_safe(const int* data, int size);
long long array_sum(const int* data, int size);
double array_average(const int* data, int size);

int main() {
    const int values[] = {80, 91, 76, 88, 95};
    const int size = 5;

    std::cout << "Sum: "
              << array_sum(values, size)
              << '\n';

    std::cout << "Average: "
              << std::fixed
              << std::setprecision(1)
              << array_average(values, size)
              << '\n';
    std::cout << "Sum: "
              << array_sum(nullptr, size)
              << '\n';

    std::cout << "Average: "
              << std::fixed
              << std::setprecision(1)
              << array_average(nullptr, size)
              << '\n';
    std::cout << "Sum: "
              << array_sum(values, 0)
              << '\n';

    std::cout << "Average: "
              << std::fixed
              << std::setprecision(1)
              << array_average(values, 0)
              << '\n';

    return 0;
}
bool is_safe(const int* data,int size){
    if(data==nullptr || size <= 0){
        return false;
    }
    return true;
}
long long array_sum(const int* data, int size){
    if(!is_safe(data,size)){
        std::cout << "Invalid Input" << std::endl;
        return -1;
    }
    long long sum = 0;
    for(int i = 0; i < size; i++)
        sum += data[i];
    return sum;
}
double array_average(const int* data,int size){
    if(!is_safe(data,size)){
        std::cout << "Invalid Input" << std::endl;
        return -1;
    }
    long long sum = array_sum(data,size);
    double average = static_cast<double>(sum)/size;
    return average;
}