#include "Common/thread_utils.hpp"

#include "matching_engine.hpp"
#include "me_order_book.hpp"

namespace Exchange {

    MatchingEngine::MatchingEngine(ClientRequestLFQueue* client_request_queue, ClientResponseLFQueue* client_response_queue, MarketUpdateLFQueue* market_update_queue)
        : client_request_queue_(client_request_queue), client_response_queue_(client_response_queue), market_update_queue_(market_update_queue), logger_("exchange_matching_engine.log") {
        logger_.log("%:% %() %\n", __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str_));
        for(auto i = 0; i < ME_MAX_TICKERS; ++i){
            ticker_order_book_[i] = new MEOrderBook(i, &logger_, this);
        }
    }

    MatchingEngine::~MatchingEngine() {
        running_ = false;

        using namespace std::literals::chrono_literals;
        std::this_thread::sleep_for(1s);

        client_request_queue_ = nullptr;
        client_response_queue_ = nullptr;
        market_update_queue_ = nullptr;

        for(auto& book : ticker_order_book_){
            delete book;
            book = nullptr;
        }
    }

    auto MatchingEngine::start() -> void {
        running_ = true;
        ASSERT(setAndCreateThread(-1, "Exchange/MatchingEngine", [this](){ run(); }) != nullptr, "Failed to start MatchingEngine thread");
    }

    auto MatchingEngine::stop() -> void {
        running_ = false;
    }
}

