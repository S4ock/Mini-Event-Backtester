#include "Book.hpp"
using namespace std;

Book::Book() = default;

double Book::bestBid() const {
    if (bidLevels_.empty()) {
        return 0.0;
    }
    return bidLevels_.rbegin()->first;
}

double Book::bestAsk() const {
    if (askLevels_.empty()) {
        return 0.0;
    }
    return askLevels_.begin()->first;
}

bool Book::addOrder(const Order& order) {
    if (order.side == Side::Buy) {
        bidLevels_[order.limit_price].orders.insert(order);
        orderMap_[order.id] = {true, bidLevels_.find(order.limit_price), bidLevels_[order.limit_price].orders.find(order)};
    } else {
        askLevels_[order.limit_price].orders.insert(order);
        orderMap_[order.id] = {false, askLevels_.find(order.limit_price), askLevels_[order.limit_price].orders.find(order)};
    }
    return true;
}

bool Book::modifyOrder(int orderId, int new_limit_price, int new_quantity) {
    auto it = orderMap_.find(orderId);
    if (it == orderMap_.end()) {
        return false;
    }

    OrderLocation& loc = it->second;
    const Order oldOrder = *(loc.orderIt);
    Order updatedOrder = oldOrder;

    
    if (loc.isBid) {
        loc.levelIt->second.orders.erase(loc.orderIt);
        if (loc.levelIt->second.orders.empty()) {
            bidLevels_.erase(loc.levelIt);
        }
    } else {
        loc.levelIt->second.orders.erase(loc.orderIt);
        if (loc.levelIt->second.orders.empty()) {
            askLevels_.erase(loc.levelIt);
        }
    }

    
    updatedOrder.limit_price = new_limit_price;
    updatedOrder.quantity = new_quantity;

    
    if (loc.isBid) {
        bidLevels_[new_limit_price].orders.insert(updatedOrder);
        loc.levelIt = bidLevels_.find(new_limit_price);
        loc.orderIt = bidLevels_[new_limit_price].orders.find(updatedOrder);
    } else {
        askLevels_[new_limit_price].orders.insert(updatedOrder);
        loc.levelIt = askLevels_.find(new_limit_price);
        loc.orderIt = askLevels_[new_limit_price].orders.find(updatedOrder);
    }

    return true;
}

bool Book::cancelOrder(int orderId) {
    auto it = orderMap_.find(orderId);
    if (it == orderMap_.end()) {
        return false;
    }
    OrderLocation& loc = it->second;
    if (loc.isBid) {
        loc.levelIt->second.orders.erase(loc.orderIt);
        if (loc.levelIt->second.orders.empty()) {
            bidLevels_.erase(loc.levelIt);
        }
    } else {
        loc.levelIt->second.orders.erase(loc.orderIt);
        if (loc.levelIt->second.orders.empty()) {
            askLevels_.erase(loc.levelIt);
        }
    }
    orderMap_.erase(it);
    return true;
}

vector<Fill> Book::checkCross(Time now, Order order){
    if (order.side==Side::Buy){
        vector<Fill> ans;
        for (auto it = askLevels_.begin(); it != askLevels_.end(); ++it) {
            double price = it->first;
            BookLevel& level = it->second;
            if(price > order.limit_price) {
                break;
            }
            for(auto elem: level.orders){
                if(elem.quantity <= order.quantity){
                    Fill fill{now, elem.id, elem.owner_id, elem.side, elem.quantity, price};
                    Fill fill1{now, order.id, order.owner_id, order.side, elem.quantity, price};
                    ans.push_back(fill);
                    ans.push_back(fill1);
                    order.quantity -= elem.quantity;
                    cancelOrder(elem.id);
                } else {
                    Fill fill{now, elem.id, elem.owner_id, elem.side, order.quantity, price};
                    Fill fill1{now, order.id, order.owner_id, order.side, order.quantity, price};
                    ans.push_back(fill);
                    ans.push_back(fill1);
                    order.quantity = 0;
                    cancelOrder(order.id);
                }
                if(order.quantity == 0) {
                    break;
                }
            }
            if(order.quantity == 0) {
                break;
            }
        }
        return ans;
    }else{
        vector<Fill> ans;
        for (auto it = bidLevels_.rbegin(); it != bidLevels_.rend(); ++it) {
            double price = it->first;
            BookLevel& level = it->second;
            if(price < order.limit_price) {
                break;
            }
            for(auto elem: level.orders){
                if(elem.quantity <= order.quantity){
                    Fill fill{now, elem.id, elem.owner_id, elem.side, elem.quantity, price};
                    Fill fill1{now, order.id, order.owner_id, order.side, elem.quantity, price};
                    ans.push_back(fill);
                    ans.push_back(fill1);
                    order.quantity -= elem.quantity;
                    cancelOrder(elem.id);
                } else {
                    Fill fill{now, elem.id, elem.owner_id, elem.side, order.quantity, price};
                    Fill fill1{now, order.id, order.owner_id, order.side, order.quantity, price};
                    ans.push_back(fill);
                    ans.push_back(fill1);
                    order.quantity = 0;
                    cancelOrder(order.id);
                }
                if(order.quantity == 0) {
                    break;
                }
            }
            if(order.quantity == 0) {
                break;
            }
        }
        return ans;
    }   
}
