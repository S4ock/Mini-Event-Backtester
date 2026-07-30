#include "Portfolio.hpp"
using namespace std;

Portfolio::Portfolio() = default;

int Portfolio::getPosition() const {
    return position_;
}
double Portfolio::getCash() const {
    return cash_;
}
vector<Order> Portfolio::getOpenOrders() const {
    return vector<Order>(orders_.begin(), orders_.end());
}

void Portfolio::applyFill(const Fill& fill) {
    double fee = fill.price * fill.quantity * fee_rate;
    if (fill.side == Side::Buy) {
        cash_ -= fill.price * fill.quantity + fee;
        position_ += fill.quantity;
    } else if (fill.side == Side::Sell) {
        cash_ += fill.price * fill.quantity - fee;
        position_ -= fill.quantity;
    }
    //editing the existing order to take in consideration the filled quantity

    Order key{};
    key.id = fill.order_id;

    auto it = orders_.lower_bound(key);

    if (it != orders_.end() && it->id == fill.order_id) {
        Order modifiedOrder = *it;
        modifiedOrder.quantity = it->quantity - fill.quantity;
        orders_.erase(it);
        if(modifiedOrder.quantity)orders_.insert(modifiedOrder);
    }

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
        modifiedOrder.quantity = new_quantity;

        orders_.erase(it);
        if(modifiedOrder.quantity)orders_.insert(modifiedOrder);
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