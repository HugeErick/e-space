#include "auction_client.h"
#include <iostream>

AuctionClient::AuctionClient(std::shared_ptr<Channel> channel)
    : stub_(server::Auction::NewStub(channel)) {}

bool AuctionClient::RegisterUser(const std::string& nickname) {
    server::RegisterUserRequest request;
    request.set_nickname(nickname);
    
    server::RegisterUserResponse response;
    ClientContext context;
    
    Status status = stub_->RegisterUser(&context, request, &response);
    
    if (!status.ok()) {
        last_error_ = "RPC failed: " + status.error_message();
        std::cerr << last_error_ << std::endl;
        return false;
    }
    
    if (!response.success()) {
        last_error_ = "User already exists";
        return false;
    }
    
    last_error_.clear();
    return true;
}

bool AuctionClient::AddProduct(const std::string& name, double initial_price, 
                                const std::string& seller, std::string& out_product_id) {
    server::AddProductRequest request;
    request.set_name(name);
    request.set_initial_price(initial_price);
    request.set_seller(seller);
    
    server::AddProductResponse response;
    ClientContext context;
    
    Status status = stub_->AddProduct(&context, request, &response);
    
    if (!status.ok()) {
        last_error_ = "RPC failed: " + status.error_message();
        std::cerr << last_error_ << std::endl;
        return false;
    }
    
    if (!response.success()) {
        last_error_ = "Failed to add product";
        return false;
    }
    
    out_product_id = response.product_id();
    last_error_.clear();
    return true;
}

std::vector<ProductData> AuctionClient::GetProducts() {
    std::vector<ProductData> products;
    
    server::GetProductsRequest request;
    server::GetProductsResponse response;
    ClientContext context;
    
    Status status = stub_->GetProducts(&context, request, &response);
    
    if (!status.ok()) {
        last_error_ = "RPC failed: " + status.error_message();
        std::cerr << last_error_ << std::endl;
        return products;
    }
    
    for (const auto& product : response.products()) {
        ProductData data;
        data.id = product.id();
        data.name = product.name();
        data.initial_price = product.initial_price();
        data.current_price = product.current_price();
        data.seller = product.seller();
        products.push_back(data);
    }
    
    last_error_.clear();
    return products;
}

bool AuctionClient::PlaceBid(const std::string& product_id, const std::string& bidder, double amount) {
    server::PlaceBidRequest request;
    request.set_product_id(product_id);
    request.set_bidder(bidder);
    request.set_amount(amount);
    
    server::PlaceBidResponse response;
    ClientContext context;
    
    Status status = stub_->PlaceBid(&context, request, &response);
    
    if (!status.ok()) {
        last_error_ = "RPC failed: " + status.error_message();
        std::cerr << last_error_ << std::endl;
        return false;
    }
    
    if (!response.success()) {
        last_error_ = "Bid amount must be higher than current price";
        return false;
    }
    
    last_error_.clear();
    return true;
}