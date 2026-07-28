#pragma once
#include "src\main.cpp"
#include <set>
#include <map>
using namespace std;
enum class Side{
    Buy,
    Sell
};
struct Order {
    OrderId id;
    int owner_id;
    string symbol;
    Time ts;
    Side side;
    int quantity;
    double limit_price;

    bool operator<(const Order& other) const {
        if (ts != other.ts)
            return ts < other.ts;

        if (quantity != other.quantity)
            return quantity < other.quantity;

        return id < other.id;
    }
};

struct BookLevel {
    set<Order> orders;
};

struct OrderLocation {
    bool isBid;
    map<double, BookLevel>::iterator levelIt;
    set<Order>::iterator orderIt;
};

struct Fill{
    string symbol;
    Time ts;
    OrderId order_id;
    OwnerId owner_id;
    Side side;
    int quantity;
    double price;
};

struct AddOrder{
    Time sent_ts;
    Order order;
};

struct ModifyOrder{
    Time sent_ts;
    OrderId order_id;
    int new_quantity;
    double new_limit_price;
};

struct CancelOrder{
    Time sent_ts;
    OrderId order_id;
};