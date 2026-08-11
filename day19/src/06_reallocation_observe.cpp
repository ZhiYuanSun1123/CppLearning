#include<iostream>
#include<vector>
int main(){
    std::vector<int> values;
    values.push_back(10);

    const int* old_address = values.data();
    const auto old_capacity = values.capacity();

    while (values.capacity() == old_capacity) {
        values.push_back(20);
    }

    std::cout
        << "old: "
        << static_cast<const void*>(old_address)
        << '\n';

    std::cout
        << "new: "
        << static_cast<const void*>(values.data())
        << '\n';
    std::cout << *old_address << std::endl;
}