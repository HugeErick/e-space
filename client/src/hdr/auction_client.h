#ifndef AUCTION_CLIENT_H
#define AUCTION_CLIENT_H

#include <memory>
#include <string>
#include <vector>
#include <grpcpp/grpcpp.h>
#include "../../server/build/e-space.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

struct ProductData {
    std::string id;
    std::string name;
    double initial_price;
    double current_price;
    std::string seller;
};

class AuctionClient {
public:
    AuctionClient(std::shared_ptr<Channel> channel);
    
    bool RegisterUser(const std::string& nickname);
    bool AddProduct(const std::string& name, double initial_price, const std::string& seller, std::string& out_product_id);
    std::vector<ProductData> GetProducts();
    bool PlaceBid(const std::string& product_id, const std::string& bidder, double amount);
    
    const std::string& GetLastError() const { return last_error_; }
    
private:
    std::unique_ptr<server::Auction::Stub> stub_;
    std::string last_error_;
};

#endif // AUCTION_CLIENT_H