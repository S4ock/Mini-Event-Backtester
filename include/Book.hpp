#pragma once
#include <set>
#include <unordered_map>
#include "src\main.cpp"
#include "Common.hpp"
using namespace std;

class Book{
    public:
        Book();
        void addOrder(const Order& order);
        bool modifyOrder(int orderId,int new_limit_price,int new_quantity);
        bool cancelOrder(int orderId);
        Price bestBid() const;
        Price bestAsk() const;
        vector<Fill> checkCross(Time now, Order order);
    private:
        map<Price, BookLevel> bidLevels_;
        map<Price, BookLevel> askLevels_;
        struct OrderLocation {
            bool isBid;
            map<Price, BookLevel>::iterator levelIt;
            set<Order>::iterator orderIt;
        };

        unordered_map<OrderId, OrderLocation> orderMap_;
};