#include "Backtest.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
using namespace std;

Backtest::Backtest(string& historicalDataPath,Book& book,Strategy& strategy,Time startTime,Time endTime,Time strategyLatency)
    : historicalDataPath_(historicalDataPath),book_(book),strategy_(strategy),startTime_(startTime),endTime_(endTime),strategyLatency_(strategyLatency) {
        load_historical_data(historicalDataPath);
    }

void Backtest::load_historical_data(string historicalDataPath) {
    ifstream file(historicalDataPath);

    if (!file.is_open()) {
        throw runtime_error("Failed to open historical data file.");
    }

    string line;

    while (getline(file, line)) {
        stringstream ss(line);
        string cell;

        OrderCommand command;

        // Time
        getline(ss, cell, ',');
        command.ts = stod(cell);
        command.ts=command.ts*1e9; // convert to nanoseconds

        // Event type
        getline(ss, cell, ',');
        int eventType = stoi(cell);

        if (eventType != 1 && eventType != 2 && eventType != 3)
            continue;

        // Order ID
        getline(ss, cell, ',');
        command.order_id = stoll(cell);

        // Size
        getline(ss, cell, ',');
        int size = stoi(cell);

        // Price
        getline(ss, cell, ',');
        command.limit_price = stod(cell)/10000.0; // convert to dollars

        // Direction
        getline(ss, cell, ',');
        command.side = (stoi(cell) == 1 ? Side::Buy : Side::Sell);

        switch (eventType) {
        case 1:
            command.type = OrderCommandType::AddOrder;
            command.quantity = size;
            break;

        case 2:
            command.type = OrderCommandType::ModifyOrder;
            command.new_quantity = size;   // quantity to cancel
            break;

        case 3:
            command.type = OrderCommandType::CancelOrder;
            break;
        }
        eventPool_.push(command);
    }
}

bool Backtest::AddOrder(Time now, const OrderCommand& command) {
    Order order;
    order.owner_id = command.owner_id;
    order.side = command.side;
    order.quantity = command.quantity;
    order.limit_price = command.limit_price;
    order.id = command.order_id;
    order.ts = now;
    bool worked=book_.addOrder(order);
    orderQuantities_[order.id] = order.quantity;
    if(worked && command.owner_id==1)portfolio_.addOrder(order);
    return worked;
}

bool Backtest::ModifyOrder(Time now, const OrderCommand& command) {
    bool worked=book_.modifyOrder(command.order_id, command.new_limit_price, command.new_quantity);
    // i do this because the modify command is used to cancel orders as well, so if the owner is external, we need to subtract the new quantity from the existing quantity
    if(command.owner_id==1)orderQuantities_[command.order_id] = command.new_quantity;
    else orderQuantities_[command.order_id] = orderQuantities_[command.order_id]-command.new_quantity;

    if(worked && command.owner_id==1)portfolio_.modifyOrder(command.order_id, command.new_limit_price, orderQuantities_[command.order_id]);
    return worked;
}

bool Backtest::CancelOrder(Time now, const OrderCommand& command) {
    bool worked=book_.cancelOrder(command.order_id);
    orderQuantities_.erase(command.order_id);
    if(worked && command.owner_id==1)portfolio_.removeOrder(command.order_id);
    return worked;
}
optional<OrderEvent> Backtest::applyFill(Time now, const Fill& fill) {
    orderQuantities_[fill.order_id] -= fill.quantity;
    if(orderQuantities_[fill.order_id] <= 0) {
        orderQuantities_.erase(fill.order_id);
    }
    if(fill.owner_id==1){
        portfolio_.applyFill(fill);
        return optional<OrderEvent>{OrderEvent{now, OrderEventType::OrderFilled, fill.owner_id, fill.order_id, fill.side, fill.quantity, orderQuantities_[fill.order_id], fill.price}};
    }
    return nullopt;
}
void Backtest::scheduleEvent(const OrderCommand& event) {
    eventPool_.push(event);
}
void Backtest::applyOrderCommand(Time now, const OrderCommand& command) {
    switch (command.type) {
        case OrderCommandType::AddOrder: {
            bool worked=AddOrder(now_,command);
            if(command.owner_id==1){
                recent_order_events_.push_back(OrderEvent{now_, worked?OrderEventType::OrderAccepted:OrderEventType::OrderRejected, command.owner_id, command.order_id, command.side, command.quantity, orderQuantities_[command.order_id], command.limit_price});
            }
            auto fills=book_.checkCross(now_,Order{command.order_id,command.owner_id,now_,command.side,command.quantity,command.limit_price});
            for(const auto& fill:fills){
                auto order_event=applyFill(now_,fill);
                if(order_event.has_value()){
                    recent_order_events_.push_back(order_event.value());
                }
            }
            break;
        }
        case OrderCommandType::ModifyOrder: {
            bool worked=ModifyOrder(now_,command);
            if(command.owner_id==1){
                recent_order_events_.push_back(OrderEvent{now_, worked?OrderEventType::OrderModified:OrderEventType::OrderModifyFailed, command.owner_id, command.order_id, command.side, command.new_quantity, orderQuantities_[command.order_id], command.new_limit_price});
            }
            auto fills=book_.checkCross(now_,Order{command.order_id,command.owner_id,now_,command.side,orderQuantities_[command.order_id],command.new_limit_price});
            for(const auto& fill:fills){
                auto order_event=applyFill(now_,fill);
                if(order_event.has_value()){
                    recent_order_events_.push_back(order_event.value());
                }
            }
            break;
        }
        case OrderCommandType::CancelOrder: {
            bool worked=CancelOrder(now_,command);
            if(command.owner_id==1){
                recent_order_events_.push_back(OrderEvent{now_, worked?OrderEventType::OrderCancelled:OrderEventType::OrderCancelFailed, command.owner_id, command.order_id, command.side, command.quantity, orderQuantities_[command.order_id], command.limit_price});
            }
            auto fills=book_.checkCross(now_,Order{command.order_id,command.owner_id,now_,command.side,orderQuantities_[command.order_id],command.limit_price});
            for(const auto& fill:fills){
                auto order_event=applyFill(now_,fill);
                if(order_event.has_value()){
                    recent_order_events_.push_back(order_event.value());
                }
            }
            break;
        }
    }
}
void Backtest::run() {
    cout<<"Started Backtest from "<<startTime_<<" to "<<endTime_<<endl;
    for(Time i=startTime_;i<=endTime_;i=eventPool_.empty()?endTime_+1:eventPool_.top().ts){
        recent_order_events_.clear();
        now_=i;
        while(!eventPool_.empty() && eventPool_.top().ts<=now_){
            OrderCommand command=eventPool_.top();
            eventPool_.pop();
            applyOrderCommand(now_,command);
        }
        auto new_commands=strategy_.onTimeMove(now_,book_,portfolio_,recent_order_events_);
        if(new_commands.has_value()){
            for(auto& new_command:new_commands.value()){
                new_command.ts = now_ + strategyLatency_;
                if(new_command.type==OrderCommandType::AddOrder){
                    new_command.order_id=nextOrderId_++;
                }
                eventPool_.push(new_command);
            }
        }
    }
}

void Backtest::printResults() {
    cout << "Final Cash: " << portfolio_.getCash() << endl;
    cout << "Final Position: " << portfolio_.getPosition() << endl;
    cout << "Open Orders: " << endl;
    cout<< "Final estimated money: "<<portfolio_.getCash() + portfolio_.getPosition() *book_.bestBid()<<endl;
    for (const auto& order : portfolio_.getOpenOrders()) {
        cout << "Order ID: " << order.id
             << ", Side: " << (order.side == Side::Buy ? "Buy" : "Sell")
             << ", Quantity: " << order.quantity
             << ", Limit Price: " << order.limit_price
             << endl;
    }
}