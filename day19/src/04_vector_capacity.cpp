#include<iostream>
#include<vector>
int main(){
    std::vector<int> values;
    std::size_t count = 0;
    values.reserve(100);
    for(int i = 0;i<100; i++){
        std::size_t old_size = values.size();
        std::size_t old_capacity = values.capacity();
        int* old_data_address = values.data();
        values.push_back(i);
        if(old_capacity != values.capacity()){
            count ++;
            std::cout << "Old Size: " << old_size << std::endl;
            std::cout << "New Size: " << values.size() << std::endl;
            std::cout << "Old Capacity: " << old_capacity << std::endl;
            std::cout << "New Capacity: " << values.capacity() << std::endl;
            std::cout << "Old Data Address: " <<  old_data_address << std::endl;
            std::cout << "New Data Address: " << values.data() << std::endl;
        }
    }
    std::cout << count << std::endl;
}