#include<vector>
#include<cstddef>
#include<utility>
#include<stdexcept>
template<typename T>
class SimpleStack{
public:
    void push(const T& t);
    void push(T&& t);
    std::vector<T>::iterator top();
    void pop();
    bool empty();
    std::size_t size();
    void clear();
private:
    std::vector<T> stack;
};
template<typename T>
void SimpleStack<T>::push(const T& t){
    stack.push_back(t);
}
template<typename T>
void SimpleStack<T>::push(T&& t){
    stack.push_back(std::move(t));
}
template<typename T>
std::vector<T>::iterator SimpleStack<T>::top(){
    if(stack.empty())
        throw std::runtime_error("不能获取为空的top");
    return stack.front();
}
template<typename T>
void SimpleStack<T>::pop(){
    if(stack.empty())
        throw std::runtime_error("不能获取为空的pop");
    stack.pop_back();
}
template<typename T>
bool SimpleStack<T>::empty(){
    return stack.empty();
}
template<typename T>
std::size_t SimpleStack<T>::size(){
    return stack.size();
}
template<typename T>
void SimpleStack<T>::clear(){
    stack.clear();
}