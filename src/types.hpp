#pragma once

template<typename T>
class Optional {
public:
    T value;
    bool success;
    String err;
};


struct dataItem {
    String name;
    String value;
    String suffix;
};
