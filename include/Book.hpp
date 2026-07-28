#pragma once
#include <set>
#include <unordered_map>
#include "src\main.cpp"
using namespace std;

class Book{
    public:
        Book();
        void addOrder(const Order& order);
        bool modifyOrder(int orderId,int new_limit_price,int new_quantity);
        bool cancelOrder(int orderId);
        double bestBid() const;
        double bestAsk() const;
        vector<Fill> checkCross(Time now, Order order);
    private:
        map<double, BookLevel> bidLevels_;
        map<double, BookLevel> askLevels_;
        struct OrderLocation {
            bool isBid;
            map<double, BookLevel>::iterator levelIt;
            set<Order>::iterator orderIt;
        };

        unordered_map<OrderId, OrderLocation> orderMap_;
};