#include "Portfolio.hpp"
using namespace std;

Portfolio::Portfolio() = default;

int Portfolio::getPosition() const {
    return position_;
}

int Portfolio::getEffectivePosition() const {
    int effective = position_;
    for (const auto& order : orders_) {
        if (order.side == Side::Buy) {
            effective += order.quantity;
        } else {
            effective -= order.quantity;
        }
    }
    return effective;
}
int Portfolio::getTotalHistoricalPosition() const {
    return total_historical_position_;
}
double Portfolio::getCash() const {
    return cash_;
}
vector<Order> Portfolio::getOpenOrders() const {
    return vector<Order>(orders_.begin(), orders_.end());
}

vector<Fill> Portfolio::getOrderHistory() const {
    return order_history_;
}

void Portfolio::applyFill(const Fill& fill) {
    double fee = fill.price * fill.quantity * fee_rate;
    if (fill.side == Side::Buy) {
        cash_ -= fill.price * fill.quantity + fee;
        total_historical_position_ += fill.quantity;
        position_ += fill.quantity;
    } else if (fill.side == Side::Sell) {
        cash_ += fill.price * fill.quantity - fee;
        total_historical_position_ -= fill.quantity;
        position_ -= fill.quantity;
    }
    //editing the existing order to take in consideration the filled quantity

    Order key{};
    key.id = fill.order_id;

    auto it = orders_.lower_bound(key);

    if (it != orders_.end() && it->id == fill.order_id) {
        Order modifiedOrder = *it;
        modifiedOrder.quantity = max(0, it->quantity - fill.quantity);
        orders_.erase(it);
        if (modifiedOrder.quantity) {
            orders_.insert(modifiedOrder);
        }
    }
    order_history_.push_back(fill);

}

void Portfolio::addOrder(const Order& order) {
    orders_.insert(order);
}

void Portfolio::modifyOrder(int orderId, Price new_limit_price, int new_quantity) {
    Order key{};
    key.id = orderId;

    auto it = orders_.lower_bound(key);

    if (it != orders_.end() && it->id == orderId) {
        Order modifiedOrder = *it;
        modifiedOrder.limit_price = new_limit_price;
        modifiedOrder.quantity = max(0, new_quantity);

        orders_.erase(it);
        if (modifiedOrder.quantity) {
            orders_.insert(modifiedOrder);
        }
    }
}

void Portfolio::removeOrder(int orderId) {
    Order key{};
    key.id = orderId;

    auto it = orders_.lower_bound(key);

    if (it != orders_.end() && it->id == orderId) {
        orders_.erase(it);
    }
}