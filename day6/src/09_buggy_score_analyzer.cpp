#include <iostream>

long long calculate_sum(const int* scores, int size) {
    long long sum = 0;

    for (int index = 0; index < size; ++index) {
        sum += scores[index];
    }

    return sum;
}

int main() {
    const int size = 5;
    int* scores = new int[size]{80, 91, 76, 88, 95};

    std::cout << "Sum: " << calculate_sum(scores, size) << '\n';

    int* first_score = scores;
    std::cout << "First score: " << *first_score << '\n';
    delete[] scores;
    scores = nullptr;
    first_score = nullptr;
    return 0;
}