#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <cstddef>
#include "Common.hpp"
#include "Backtest.hpp"
#include "Strategy.hpp"
#include "Book.hpp"
#include "Portfolio.hpp"
using namespace std;
int main(){
    Book book;
    Portfolio portfolio;
    Strategy strategy;
    string historicalDataPath = "data/SPY_2012-06-21_34200000_37800000_message_50.csv";
    Time startTime = 34200017459617;
    Time endTime = 38599959359650;
    Time strategyLatency = 1000000;
    Backtest backtest(historicalDataPath, book, strategy, startTime, endTime, strategyLatency);
    backtest.run();
    backtest.printResults();
}