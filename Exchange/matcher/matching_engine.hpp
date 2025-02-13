#pragma once

#include "Common/logging.hpp"
#include "Exchange/order_server/client_request.hpp"
#include "Exchange/order_server/client_response.hpp"
#include "Exchange/market_data/market_update.hpp"

#include "me_order_book.hpp"

using namespace Common;

namespace Exchange {
    class MatchingEngine final {
        public:
            MatchingEngine(ClientRequestLFQueue* client_request_queue, ClientResponseLFQueue* client_response_queue, MarketUpdateLFQueue* market_update_queue);
            
            ~MatchingEngine();

            auto start() -> void;
            auto stop() -> void;

            auto processClientRequest(const MEClientRequest* request) -> void {
                auto order_book_ = ticker_order_book_[request->tickerId];
                switch(request->type){
                    case ClientRequestType::NEW:
                        order_book_->add(request->clientId, request->price, request->qty, request->side, request->orderId, request->tickerId);
                        break;
                    case ClientRequestType::CANCEL:
                        order_book_->cancel(request->clientId, request->orderId, request->tickerId);
                        break;
                    default:
                        FATAL("Invalid request type %" + clientRequestTypeToString(request->type));
                        break;
                }
            }

            auto sendClientResponse(const MEClientResponse* response) -> void {
                logger_.log("%:% %() % Sending %\n", __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str_), response->toString());
                auto next_write_ = client_response_queue_->getNextWriteLocation();
                *next_write_ = std::move(*response);
                client_response_queue_->updateNextToWrite();
            }

            auto sendMarketUpdate(const MEMarketUpdate* update) -> void {
                logger_.log("%:% %() % Sending %\n", __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str_), update->toString());
                auto next_write_ = market_update_queue_->getNextWriteLocation();
                *next_write_ = std::move(*update);
                market_update_queue_->updateNextToWrite();
            }

            auto run() noexcept -> void {
                logger_.log("%:% %() %\n", __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str_));
                while(running_){
                    const auto client_request_ = client_request_queue_->getNextReadLocation();
                    if(client_request_) [[likely]] {
                        logger_.log("%:% %() % Processing %\n", __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str_), client_request_->toString());
                        processClientRequest(client_request_);
                        client_request_queue_->updateNextToRead();
                    }
                }
            }

            MatchingEngine() = delete;
            MatchingEngine(const MatchingEngine&) = delete;
            MatchingEngine(const MatchingEngine&&) = delete;
            auto operator=(MatchingEngine&) -> MatchingEngine& = delete;
            auto operator=(MatchingEngine&&) -> MatchingEngine& = delete;
        
        private:
            ClientRequestLFQueue* client_request_queue_;
            ClientResponseLFQueue* client_response_queue_;
            MarketUpdateLFQueue* market_update_queue_;

            volatile bool running_ = false;
            OrderBookHashMap ticker_order_book_;

            Logger logger_;
            std::string time_str_;

    };
}