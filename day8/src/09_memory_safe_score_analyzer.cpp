#include <iomanip>
#include <iostream>
#include <iomanip>

bool is_safe(const int* scores, int size){
    if(scores == nullptr || size <= 0)
        return false;
    return true;
}

long long calculate_sum(
    const int* scores,
    int size
) {
    long long sum = 0;
//缺少空指针检测
    if(!is_safe(scores,size))
        return 0;
    for (int index = 0;
         index < size; // 数组越界
         ++index) {
        sum += scores[index];
    }

    return sum;
}

double calculate_average(
    const int* scores,
    int size
) {
    // 检测空指针检测
    // 没有对size进行检测
    if(!is_safe(scores,size))
        return 0;
    return static_cast<double>(
        calculate_sum(scores, size)
    ) / size;
}

int main() {
    const int size = 5;

    int* scores = new int[size]{
        80, 91, 76, 88, 95
    };

    int* first_score = scores;

    std::cout
        << "Sum: "
        << calculate_sum(scores, size)
        << '\n';
    std::cout
        << "Average: "
        << std::fixed
        << std::setprecision(2)
        << calculate_average(scores,size)
        << std::endl;
    std::cout
        << "First: "
        << *first_score //释放后使用
        << '\n';

    delete[] scores;
    scores = nullptr;

    return 0;
}