#include <iostream>

int* create_sequence(int size) {
    if (size <= 0) {
        return nullptr;
    }

    int* data = new int[size]{};

    for (int index = 0; index < size; ++index) {
        std::cin >> data[index];
        if(std::cin.fail()){
            delete[] data;
            return nullptr;
        }
    }

    return data;
}

void destroy_array(int*& data) {
    delete[] data;
    data = nullptr;
}

int main() {
    const int size = 5;
    int* values = create_sequence(size);
    if(values == nullptr)
        return 0;
    for(int i = 0; i < size; i++){
        std::cout << values[i] << " ";
    }
    destroy_array(values);
    if(values==nullptr){
        std::cout << "values is nullptr" << std::endl;
    }
    return 0;
}