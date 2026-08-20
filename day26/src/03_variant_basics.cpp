#include<iostream>
int main(){
    std::variant<int,double,std::string> test;
    std::cout << test.index() << std::endl;
    test = 1.0;
    std::cout << test.index() << std::endl;
    test = "string";
    std::cout << test.index() << std::endl;
    if(std::holds_alternative<std::string>(test)){
        std::cout << "String "<< std::endl;
    }
    std::cout << std::get<std::string>(test) << std::endl;
    try{
        std::get<int>(test);
    }catch(const std::bad_variant_access error){
        std::cout << "bad_variant_access" << std::endl;
    }
}