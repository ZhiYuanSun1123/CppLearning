template<typename T>
T larger_of(const T& t1,const T& t2){
    return t1 > t2 ? t1 : t2;
}
template<typename T>
T smaller_of(const T& t1,const T& t2){
    return t1 > t2 ? t2 : t1;
}
template<typename T>
T absolute_value(const T& t){
    return t>0?t:-t;
}
template<typename T>
T clamp_value(const T& t,const T& min,const T& max){
    if(t<min)
        return min;
    else if(t>max)
        return max;
    else
        return t;
}