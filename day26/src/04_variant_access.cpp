#include<iostream>
#include<type_traits>
int main(){
    std::variant<
        int,
        double,
        std::string
    > test;
    test = 1;
    if(auto* num = std::get_if<int>(&test)){
        std::cout << *num << std::endl;
    }
    auto* num = std::get_if<double>(&test);
    if(num == nullptr){
        std::cout << "nullptr "<< std::endl;
    }
    test = "string";
    std::visit(
        [](auto& value){
            if constexpr(std::is_same_v<std::decay_t<decltype(value)>,std::string>)
                std::cout << value << std::endl;
        },
        test
    );
}