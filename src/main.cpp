#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <cstddef>
#include "include\Common.hpp"
using namespace std;
using Time=long long;
using OrderId=int;
using OwnerId=int;
const Time latency=3;

int signed_quantity(const Fill& fill){
    if (fill.side == Side::Buy) {
        return fill.quantity;
    }
    return -fill.quantity;
}
Time arrival_time(Time sent_ts,Time latency){
    return sent_ts+latency;
}
int main(){
    //testing
}