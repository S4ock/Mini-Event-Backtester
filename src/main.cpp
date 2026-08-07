#include <iostream>
#include <string>
#include "Backtest.hpp"
#include "RollingMid.hpp"

int main() {
    std::string path = "data/AMZN_2012-06-21_34200000_57600000_message_10.csv";
    Book book;
    RollingMid strategy;
    Backtest backtest(path, book, strategy, 34200017459617LL, 57599959359650LL, 1000000LL);

    backtest.run();
    backtest.printResults();
    return 0;
}
