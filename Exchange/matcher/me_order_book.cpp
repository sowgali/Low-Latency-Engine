#include <sstream>
#include "me_order_book.hpp"
#include "matching_engine.hpp"

namespace Exchange {
    MEOrderBook::MEOrderBook(TickerId ticker_id_, Logger *logger, MatchingEngine *matching_engine_) : ticker_id_(ticker_id_), logger_(logger), 
    matching_engine_(matching_engine_), order_pool_(ME_MAX_ORDER_IDS), order_at_price_pool_(ME_MAX_PRICE_LEVELS) {}

    MEOrderBook::~MEOrderBook(){
        logger_->log("%:% %() % OrderBook\n%\n", __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str_), toString(false, true));
        matching_engine_ = nullptr;
        bids_by_price_ = nullptr;
        asks_by_price_ = nullptr;
        for(auto& it: client_id_to_order_map)
            it.fill(nullptr);
    }

    auto MEOrderBook::toString(bool detailed, bool validity_check) const -> std::string {
        std::stringstream ss;
        std::string time_str;

        auto printer = [&](std::stringstream& ss, MEOrderAtPrice* it, Side side, Price& last_price, bool sanity_check){
            char buf[4096];
            Qty qty = 0;
            size_t num_orders_ = 0;

            for(auto o_itr = it->first_me_order_;;o_itr = o_itr->next_order_){
                qty += o_itr->qty;
                ++num_orders_;
                if (o_itr->next_order_ == it->first_me_order_)
                    break;
            }

            sprintf(buf, " <px:%3s p:%3s n:%3s> %-3s @ %-5s(%-4s)", 
                priceToString(it->price).c_str(),
                priceToString(it->prev_price_->price).c_str(),
                priceToString(it->next_price_->price).c_str(),
                priceToString(it->price).c_str(),
                qtyToString(qty).c_str(),
                std::to_string(num_orders_).c_str()
            );

            ss << buf;

            for(auto o_itr = it->first_me_order_;;o_itr = o_itr->next_order_){
                if(detailed){
                    sprintf(buf, "[oid:%s q:%s p:%s n:%s] ", 
                        orderIdToString(o_itr->market_order_id_).c_str(),
                        qtyToString(o_itr->qty).c_str(),
                        orderIdToString(o_itr->prev_order_ ? o_itr->prev_order_->market_order_id_ : OrderId_INVALID).c_str(),
                        orderIdToString(o_itr->next_order_ ? o_itr->next_order_->market_order_id_ : OrderId_INVALID).c_str()
                    );
                    ss << buf;
                }
                if (o_itr->next_order_ == it->first_me_order_)
                    break;
                
                ss << std::endl;

                if(sanity_check){
                    if((side == Side::SELL && last_price >= it->price) || (side == Side::BUY && last_price <= it->price)){
                        FATAL("Bids/Asks not sorted by ascending/descending prices last:" + priceToString(last_price) + " itr:" + it->toString());
                    }
                    last_price = it->price;
                }
            }
        };

        ss << "Ticker: " << tickerIdToString(ticker_id_) << std::endl;

        {
            auto ask_itr = asks_by_price_;
            auto last_ask_price = std::numeric_limits<Price>::min();
            for(size_t count = 0; ask_itr; ++count){
                ss << "ASK L:" << count << " => ";
                auto next_ask_itr_ = (ask_itr->next_price_ == asks_by_price_ ? nullptr : ask_itr->next_price_);
                printer(ss, ask_itr, Side::SELL, last_ask_price, validity_check);
                ask_itr = next_ask_itr_;
            }
        }

        ss << std::endl << "-----------------X" << std::endl << std::endl;

        {
            auto bid_itr = bids_by_price_;
            auto last_bid_price = std::numeric_limits<Price>::min();
            for(size_t count = 0; bid_itr; count++){
                ss << "BID L:" << count << " => ";
                auto next_bid_itr = (bid_itr->next_price_ == bids_by_price_ ? nullptr : bid_itr->next_price_);
                printer(ss, bid_itr, Side::BUY, last_bid_price, validity_check);
                bid_itr = next_bid_itr;
            }
        }

        return ss.str();

    }

    auto MEOrderBook::addOrdersAtPrice(MEOrderAtPrice* new_orders_at_price_) noexcept -> void {
        orders_at_price_map_.at(priceToIndex(new_orders_at_price_->price)) = new_orders_at_price_;

        const auto best_orders_by_price = (new_orders_at_price_->side == Side::BUY ? bids_by_price_ : asks_by_price_);

        auto check_add_after_ = [](const MEOrderAtPrice* new_orders_at_price_, const MEOrderAtPrice* target){
            return (new_orders_at_price_->side == Side::BUY && target->price > new_orders_at_price_->price) || \
            (new_orders_at_price_->side == Side::SELL && target->price < new_orders_at_price_->price);
        };

        auto check_top_of_book_ = [](const MEOrderAtPrice* new_orders_at_price_, const MEOrderAtPrice* best_orders_by_price){
            return (new_orders_at_price_->side == Side::BUY && best_orders_by_price->price < new_orders_at_price_->price) || \
            (new_orders_at_price_->side == Side::SELL && best_orders_by_price->price > new_orders_at_price_->price);
        };

        if(best_orders_by_price == nullptr) [[unlikely]] {
            (new_orders_at_price_->side == Side::BUY ? bids_by_price_ : asks_by_price_) = new_orders_at_price_;
            new_orders_at_price_->prev_price_ = new_orders_at_price_->next_price_ = new_orders_at_price_;
        } else {
            auto target = best_orders_by_price;
            bool add_after = check_add_after_(new_orders_at_price_, target);
            if(add_after){
                target = target->next_price_;
                add_after = check_add_after_(new_orders_at_price_, target);
            }
            while(add_after && target != best_orders_by_price){
                add_after = check_add_after_(new_orders_at_price_, target);
                if(add_after)
                    target = target->next_price_;
            }
            if(add_after){
                if(target == best_orders_by_price)
                    target = target->prev_price_;
                target->next_price_->prev_price_ = new_orders_at_price_;
                new_orders_at_price_->next_price_ = target->next_price_;
                target->next_price_ = new_orders_at_price_;
                new_orders_at_price_->prev_price_ = target;

            } else {
                target->prev_price_->next_price_ = new_orders_at_price_;
                new_orders_at_price_->prev_price_ = target->prev_price_;
                target->prev_price_ = new_orders_at_price_;
                new_orders_at_price_->next_price_ = target;

                if(check_top_of_book_(new_orders_at_price_, best_orders_by_price)){
                    target->next_price_ = (target->next_price_ == best_orders_by_price ? new_orders_at_price_ : target->next_price_);
                    (new_orders_at_price_->side == Side::BUY ? bids_by_price_ : asks_by_price_) = new_orders_at_price_;
                }
            }

        }


    }

    auto MEOrderBook::removeOrdersAtprice(Side side, Price price) noexcept -> void {
        const auto best_order_at_price = side == Side::BUY ? bids_by_price_ : asks_by_price_;
        auto orders_at_price_ = getOrdersAtPrice(price);

        if(orders_at_price_ == orders_at_price_->next_price_) [[unlikely]] {
            (side == Side::BUY ? bids_by_price_ : asks_by_price_) = nullptr;
        } else {
            orders_at_price_->prev_price_->next_price_ = orders_at_price_->next_price_;
            orders_at_price_->next_price_->prev_price_ = orders_at_price_->prev_price_;

            if(orders_at_price_ == best_order_at_price){
                (side == Side::BUY ? bids_by_price_ : asks_by_price_) = orders_at_price_->next_price_;
            }

            orders_at_price_->prev_price_ = orders_at_price_->next_price_ = nullptr;
        }
        
        orders_at_price_map_.at(priceToIndex(price)) = nullptr;
        order_at_price_pool_.deallocate(orders_at_price_);
    }

    auto MEOrderBook::addOrder(MEOrder* order) noexcept -> void{
        const auto order_at_price_ = getOrdersAtPrice(order->price);
        if(order_at_price_ == nullptr) [[unlikely]] {
            auto new_order_at_price_ = order_at_price_pool_.allocate(order->price, order->side, order, nullptr, nullptr);
            addOrdersAtPrice(new_order_at_price_);
        } else {
            auto first_order_ = order_at_price_->first_me_order_;
            first_order_->prev_order_->next_order_ = order;
            order->next_order_ = first_order_;
            order->prev_order_ = first_order_->prev_order_;
            first_order_->prev_order_ = order;
        }
        client_id_to_order_map.at(order->clientId).at(order->client_order_id_) = order;
    }

    auto MEOrderBook::removeOrder(MEOrder* order) noexcept -> void {
        auto order_at_price_ = getOrdersAtPrice(order->price);
        if(order->prev_order_ == order){
            removeOrdersAtprice(order->side, order->price);
        } else {
            order->prev_order_->next_order_ = order->next_order_;
            order->next_order_->prev_order_ = order->prev_order_;
            if(order_at_price_->first_me_order_ == order){
                order_at_price_->first_me_order_ = order->next_order_;
            }
            order->prev_order_ = order->next_order_ = nullptr;
        }
        client_id_to_order_map.at(order->clientId).at(order->client_order_id_) = nullptr;
        order_pool_.deallocate(order);
    }

    auto MEOrderBook::add(ClientId client_id_, Price price_, Qty qty_, Side side_, OrderId client_order_id_, TickerId ticker_id_) noexcept  -> void {
        const auto new_market_order_id_ = generateNewMarketOrderId();
        client_response_ = {ClientResponseType::ACCEPTED, client_id_, ticker_id_, client_order_id_, new_market_order_id_, side_, 0, qty_, price_};
        matching_engine_->sendClientResponse(&client_response_);

        const auto leaves_qty_ = checkForMatch(client_id_, client_order_id_, ticker_id_, side_, price_, qty_, new_market_order_id_);

        if(leaves_qty_) [[likely]] {
            const auto priority_ = getNextPriority(price_);

            auto order = order_pool_.allocate(client_id_, ticker_id_, client_order_id_, new_market_order_id_, side_, price_, leaves_qty_, priority_);

            addOrder(order);

            market_update_ = {MarketUpdateType::ADD, ticker_id_, new_market_order_id_, side_, price_, leaves_qty_, priority_};
            matching_engine_->sendMarketUpdate(&market_update_);
        }
    }
    
    auto MEOrderBook::cancel(ClientId client_id_, OrderId client_order_id_, TickerId ticker_id_) noexcept -> void {
        auto is_cancelable = (client_id_ < client_id_to_order_map.size());
        MEOrder* matching_order_ = nullptr;
        if(is_cancelable) [[likely]] {
            client_response_ = {ClientResponseType::CANCELLED, client_id_, ticker_id_, client_order_id_, matching_order_->market_order_id_, matching_order_->side, Qty_INVALID, matching_order_->qty, matching_order_->price };
            market_update_ = {MarketUpdateType::CANCEL, matching_order_->tickerId, matching_order_->market_order_id_, matching_order_->side, matching_order_->price, 0, matching_order_->priority};

            removeOrder(matching_order_);

            matching_engine_->sendClientResponse(&client_response_);
            matching_engine_->sendMarketUpdate(&market_update_);
        } else {
            client_response_ = {ClientResponseType::CANCEL_REJECTED, client_id_, ticker_id_, client_order_id_, OrderId_INVALID, Side::INVALID, Qty_INVALID, Qty_INVALID, Price_INVALID};
            matching_engine_->sendClientResponse(&client_response_);
        }
    }

    auto MEOrderBook::checkForMatch(ClientId client_id_, OrderId client_order_id_, TickerId ticker_id_, Side side, Price price, Qty qty, OrderId new_market_order_id_) noexcept -> Qty {
        auto leaves_qty_ = qty;

        if(side == Side::BUY){
            while(leaves_qty_ && asks_by_price_) {
                const auto ask_itr = asks_by_price_->first_me_order_;
                if(asks_by_price_->price > price) [[likely]]
                    break;
                match(ticker_id_, client_id_, side, client_order_id_, new_market_order_id_, ask_itr, &leaves_qty_);
            }
        }

        if(side == Side::SELL){
            while(leaves_qty_ && bids_by_price_){
                const auto bid_itr = bids_by_price_->first_me_order_;
                if(bids_by_price_->price < price) [[likely]]
                    break;
                match(ticker_id_, client_id_, side, client_order_id_, new_market_order_id_, bid_itr, &leaves_qty_);
            }
        }

        return leaves_qty_;
    }

    auto MEOrderBook::match(TickerId ticker_id_, ClientId client_id_, Side side_, OrderId client_order_id, OrderId new_market_id_, MEOrder* itr, Qty* leaves_qty_) noexcept -> void {
        const auto order = itr;
        const auto order_qty_ = itr->qty;
        const auto fill_qty = std::min(*leaves_qty_, order_qty_);

        *leaves_qty_ -= fill_qty;
        itr->qty -= fill_qty;

        client_response_ = {ClientResponseType::FILLED, client_id_, ticker_id_, client_order_id, new_market_id_, side_, fill_qty, *leaves_qty_, order->price};
        matching_engine_->sendClientResponse(&client_response_);

        client_response_ = {ClientResponseType::FILLED, order->clientId, ticker_id_, order->client_order_id_, order->market_order_id_, side_, fill_qty, *leaves_qty_, order->price};
        matching_engine_->sendClientResponse(&client_response_);

        market_update_ = {MarketUpdateType::TRADE, ticker_id_, new_market_id_, side_, order->price, fill_qty, order->priority};
        matching_engine_->sendMarketUpdate(&market_update_);

        if(!order->qty) {
            market_update_ = {MarketUpdateType::CANCEL, ticker_id_, order->market_order_id_, side_, order->price, order->qty, Priority_INVALID};
            matching_engine_->sendMarketUpdate(&market_update_);
            removeOrder(order);
        } else {
            market_update_ = {MarketUpdateType::MODIFY, ticker_id_, order->market_order_id_, side_, order->price, order->qty, order->priority};
            matching_engine_->sendMarketUpdate(&market_update_);
        }

    }

    

}