#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <cstddef>
using namespace std;
using Time=long long;
const Time latency=3;
enum class Side{
    Buy,
    Sell
};

struct BookLevel{
    double price;
    int quantity; 
};

struct BookSnapshot{
    string symbol;
    Time ts;
    vector<BookLevel> bids; // decreasing
    vector<BookLevel> asks; // increasing
};
struct Order{
    int id;
    string symbol;
    Time ts;
    Side side;
    int quantity;
    double limit_price;
};

struct AddOrder{
    Time sent_ts;
    Order order;
};

struct ModifyOrder{
    Time sent_ts;
    int order_id;
    int new_quantity;
    double new_limit_price;
};

struct CancelOrder{
    Time sent_ts;
    int order_id;
};

struct Fill{
    string symbol;
    Time ts;
    int order_id;
    Side side;
    int quantity;
    double price;
};

struct PositionState{
    int position=0;
    double cash=0.0;
};

double best_ask(const BookSnapshot& book){
    return book.asks[0].price;
}

double best_bid(const BookSnapshot& book){
    return book.bids[0].price;
}

double spread(const BookSnapshot& book){
    return best_ask(book)-best_bid(book);
}

double mid_price(const BookSnapshot& book){
    return (best_ask(book)+best_bid(book))/2.0;
}

int signed_quantity(const Fill& fill){
    if (fill.side == Side::Buy) {
        return fill.quantity;
    }
    return -fill.quantity;
}

void apply_fill(PositionState& state, const Fill& fill){
    int quantity=signed_quantity(fill);
    state.position+=quantity;
    state.cash-=quantity*fill.price;
}

double equity(const PositionState& state,double mark_price){
    return state.cash+mark_price*state.position;
}

Time arrival_time(Time sent_ts,Time latency){
    return sent_ts+latency;
}
int main(){
    //testing
    BookSnapshot book{
        "ABC",
        1'000,
        {{100.00, 50}, {99.95, 75}, {99.90, 100}},
        {{100.05, 40}, {100.10, 60}, {100.15, 90}}
    };

    cout << fixed << setprecision(2);
    cout << "Best bid: " << best_bid(book) << "\n";
    cout << "Best ask: " << best_ask(book) << "\n";
    cout << "Spread: " << spread(book) << "\n";
    cout << "Mid: " << mid_price(book) << "\n\n";

    Order buy_order{1, "ABC", 1'010, Side::Buy, 40, 100.05};
    AddOrder add_buy{1'010 , buy_order};

    Fill buy_fill{"ABC", 1'013, buy_order.id, Side::Buy, 40, 100.05};
    Fill sell_fill{"ABC", 1'020, 2, Side::Sell, 15, 100.00};

    PositionState state;
    state.position=0;
    state.cash=100'000.00;
    apply_fill(state, buy_fill);
    cout << "After buy fill: position=" << state.position << ", cash=" << state.cash << "\n";

    apply_fill(state, sell_fill);
    cout << "After sell fill: position=" << state.position << ", cash=" << state.cash << "\n";
}