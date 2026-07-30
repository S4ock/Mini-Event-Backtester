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
    string historicalDataPath = "data/AMZN_2012-06-21_34200000_57600000_message_10.csv";
    Time startTime = 34200000000; 
    Time endTime = 36200000000;
    Time strategyLatency = 1000000;
    Backtest backtest(historicalDataPath, book, strategy, startTime, endTime, strategyLatency);
    backtest.run();
    backtest.printResults();
}