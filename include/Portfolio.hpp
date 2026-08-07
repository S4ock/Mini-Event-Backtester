#pragma once
#include <vector>
using namespace std;
#include "Common.hpp"

class Portfolio{
    public:
        Portfolio();
        void applyFill(const Fill& fill);
        int getPosition() const;
        int getEffectivePosition() const;
        void addOrder(const Order& order);
        void modifyOrder(int orderId, Price new_limit_price, int new_quantity);
        void removeOrder(int orderId);
        int getTotalHistoricalPosition() const;
        vector<Order> getOpenOrders() const;
        vector<Fill> getOrderHistory() const;
        double getCash() const;
    private:
        double cash_{100000.0};
        int position_{0};
        set<Order, CompareById> orders_;
        vector<Fill> order_history_;
        int total_historical_position_{0};
};