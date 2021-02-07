#pragma once

template<typename T>
class Optional {
public:
    T value;
    bool success;
    String err;
};