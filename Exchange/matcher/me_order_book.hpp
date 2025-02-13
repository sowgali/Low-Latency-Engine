#pragma once

#include <array>

#include "Common/types.hpp"
#include "Common/logging.hpp"
#include "Common/mem_pool.hpp"

#include "Exchange/order_server/client_response.hpp"
#include "Exchange/market_data/market_update.hpp"
#include "me_order.hpp"

namespace Exchange {
    
    class MatchingEngine;
    class MEOrderBook final {
        private:
            TickerId ticker_id_ = TickerId_INVALID;
            MatchingEngine* matching_engine_ = nullptr;
            Logger* logger_ = nullptr;
            std::string time_str_;
            ClientOrderHashMap client_id_to_order_map;
            Mempool<MEOrder> order_pool_;
            Mempool<MEOrderAtPrice> order_at_price_pool_;
            MEOrderAtPrice* bids_by_price_ = nullptr;
            MEOrderAtPrice* asks_by_price_ = nullptr;
            OrdersAtPriceHashMap orders_at_price_map_;
            OrderId next_market_order_id_ = 1;
            MEClientResponse client_response_;
            MEMarketUpdate market_update_;
        public:
            explicit MEOrderBook(TickerId ticker_id_, Logger* logger, MatchingEngine* matching_engine_);
            ~MEOrderBook();

            MEOrderBook() = delete;
            MEOrderBook(const MEOrderBook&) = delete;
            MEOrderBook(const MEOrderBook&&) = delete;
            auto operator=(const MEOrderBook&) -> MEOrderBook& = delete;
            auto operator=(const MEOrderBook&&) -> MEOrderBook& = delete;

            auto generateNewMarketOrderId() noexcept -> OrderId {
                return next_market_order_id_++;
            }

            auto priceToIndex(Price price) const noexcept {
                return price % ME_MAX_PRICE_LEVELS;
            }

            auto getOrdersAtPrice(Price price) const noexcept -> MEOrderAtPrice* {
                return orders_at_price_map_.at(priceToIndex(price));
            }

            auto getNextPriority(Price price) noexcept {
                const auto orders_at_price = getOrdersAtPrice(price);
                if(orders_at_price == nullptr)
                    return 1lu;
                return orders_at_price->first_me_order_->prev_order_->priority + 1;
            }
            
            auto add(ClientId client_id_, Price price_, Qty qty_, Side side_, OrderId client_order_id_, TickerId ticker_id_) noexcept -> void;
            auto cancel(ClientId client_id_, OrderId client_order_id_, TickerId ticker_id_) noexcept -> void;
            auto addOrdersAtPrice(MEOrderAtPrice* new_orders_at_price) noexcept -> void;
            auto removeOrdersAtprice(Side side, Price price) noexcept -> void;
            auto addOrder(MEOrder* order) noexcept -> void;
            auto removeOrder(MEOrder* order) noexcept -> void;
            auto match(TickerId ticker_id_, ClientId client_id_, Side side, OrderId client_order_id_, OrderId new_market_order_id_, MEOrder* itr, Qty* leaves_qty) noexcept -> void;
            auto checkForMatch(ClientId client_id_, OrderId client_order_id_, TickerId ticker_id_, Side side, Price price, Qty qty, OrderId new_market_order_id_) noexcept -> Qty;

            auto toString(bool detailed, bool validity_check) const -> std::string;

    };

    typedef std::array<MEOrderBook*, ME_MAX_TICKERS> OrderBookHashMap;
}