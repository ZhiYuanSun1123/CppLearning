#include<iostream>
class A{
    public:
        A(const A& a){
            this->path_ = a.path_;
        }
        std::string path_;
};