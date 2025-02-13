#pragma once

#include "Common/types.hpp"
#include "Common/spsc_lf_queue.hpp"

using namespace Common;

namespace Exchange {
    #pragma pack(push, 1)
    enum class ClientResponseType : uint8_t {
        INVALID=0,
        ACCEPTED=1,
        CANCELLED=2,
        FILLED=3,
        CANCEL_REJECTED=4
    };

    inline auto clientResponseTypeToString(ClientResponseType t) -> std::string {
        switch(t){
            case ClientResponseType::INVALID: return "INVALID";
            case ClientResponseType::ACCEPTED: return "ACCEPTED";
            case ClientResponseType::CANCELLED: return "CANCELLED";
            case ClientResponseType::FILLED: return "FILLED";
            case ClientResponseType::CANCEL_REJECTED: return "CANCEL_REJECTED";
            default: return "UNKNOWN";
        }
    }

    struct MEClientResponse {
        ClientResponseType type = ClientResponseType::INVALID;
        ClientId clientId = ClientId_INVALID;
        TickerId tickerId = TickerId_INVALID;
        OrderId client_order_id_ = OrderId_INVALID;
        OrderId market_order_id_ = OrderId_INVALID;
        Side side = Side::INVALID;
        Qty exec_qty_ = Qty_INVALID;
        Qty remaining_qty_ = Qty_INVALID;
        Price price = Price_INVALID;

        inline auto toString() const -> std::string {
            return "MEClientResponse{"
                "type=" + clientResponseTypeToString(type) + ", "
                "clientId=" + clientIdToString(clientId) + ", "
                "tickerId=" + tickerIdToString(tickerId) + ", "
                "client_order_id_=" + orderIdToString(client_order_id_) + ", "
                "market_order_id_=" + orderIdToString(market_order_id_) + ", "
                "side=" + sideToString(side) + ", "
                "exec_qty_=" + qtyToString(exec_qty_) + ", "
                "remaining_qty_=" + qtyToString(remaining_qty_) + ", "
                "price=" + priceToString(price) + "}";
        }
    };
    #pragma pack(pop)

    typedef LFQueue<MEClientResponse> ClientResponseLFQueue;
}