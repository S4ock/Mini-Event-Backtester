#pragma once
#include <set>
#include <vector>
#include <unordered_map>
#include "Common.hpp"
using namespace std;

class Book{
    public:
        Book();
        bool addOrder(const Order& order, vector<Fill>* fills = nullptr);
        bool modifyOrder(int orderId,int new_limit_price,int new_quantity, vector<Fill>* fills = nullptr);
        bool cancelOrder(int orderId);
        Price bestBid() const;
        Price bestAsk() const;
        vector<Order> getOpenOrders() const;
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