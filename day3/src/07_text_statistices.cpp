#include<iostream>
using namespace std;
int count_character(const string& text,char target);
int count_spaces(const string& text);
void print_text(const string& text);
int main(){
    string text;
    char find_chr;
    getline(cin,text);
    cin >> find_chr;
    print_text(text);
    cout << "Target count: " << count_character(text,find_chr) << endl;
    cout << "Spaces: " << count_spaces(text) << endl;
}
int count_character(const string& text, char target){
    int count = 0;
    for(int i = 0; i <= text.length(); i++){
        if(text[i] == target)
            count += 1;
    }
    return count;
}
int count_spaces(const string& text){
    int count = 0;
    for(int i = 0; i <= text.length(); i++){
        if(text[i] == ' ')
            count += 1;
    }
    return count;
}
void print_text(const string& text){
    cout << "Text: " << text << endl;
}