#pragma once
#include <vector>
#include <queue>
using namespace std;
#include "Book.hpp"
#include "Portfolio.hpp"
#include "Strategy.hpp"

class Simulator{
    public:
        Simulator(string& historicalDataPath,Book& book,Strategy& strategy,Time endTime,Time strategyLatency);
        void run();
        void printResults();
    private:
        void load_historical_data(string historicalDataPath);
        void scheduleEvent(const OrderCommand& event);

        void scheduleOrderCommand(Time now, const OrderCommand& command);
        void AddOrder(Time now,const Order& order);
        void ModifyOrder(Time now,const Order& order);
        void CancelOrder(Time now,const Order& order);
        void applyFill(Time now,const Fill& fill);

        string historicalDataPath_;
        Time now_{0};
        Time endTime_{0};
        Time strategyLatency_{1};
        Book book_;
        Strategy& strategy_;
        Portfolio portfolio_;
        OrderId nextOrderId_{1};
        priority_queue<OrderCommand,std::vector<OrderCommand>,OrderCommand> eventPool_;
        vector<OrderEvent> recent_order_events_; 
};  