#pragma once
#include <vector>
using namespace std;
#include "Common.hpp"

class Portfolio{
    public:
        Portfolio();
        void applyFill(const Fill& fill);
        int getPosition(const string& symbol) const;
        void addOrder(const Order& order);
        void modifyOrder(const Order& order);
        void removeOrder(int orderId);
        vector<int> getOpenOrders() const;
        double getCash() const;
    private:
        double cash_;
        int position_;
        set<Order> orders_;
};