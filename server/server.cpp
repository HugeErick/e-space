#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <chrono>
#include <grpcpp/grpcpp.h>
#include "e-space.grpc.pb.h"
#include <grpcpp/ext/proto_server_reflection_plugin.h>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using server::Auction;
using server::RegisterUserRequest;
using server::RegisterUserResponse;
using server::AddProductRequest;
using server::AddProductResponse;
using server::GetProductsRequest;
using server::GetProductsResponse;
using server::ProductInfo;
using server::PlaceBidRequest;
using server::PlaceBidResponse;

struct Product {
  std::string id;
  std::string name;
  double initial_price;
  double current_price;
  std::string seller;
};

struct Bid {
  std::string bidder;
  std::string product_id;
  double amount;
};

class AuctionService final : public Auction::Service {
private:
  std::unordered_map<std::string, std::string> users_;
  std::unordered_map<std::string, Product> products_;
  std::vector<Bid> bids_;
  std::mutex mutex_;

  std::string generateProductId() {
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
    return "PROD_" + std::to_string(timestamp);
  }

public:
  Status RegisterUser(ServerContext* context,
                     const RegisterUserRequest* request,
                     RegisterUserResponse* response) override {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::string nickname = request->nickname();
    std::cout << "[LOG] User registration: " << nickname << std::endl;
    
    if (users_.find(nickname) == users_.end()) {
      users_[nickname] = nickname;
      std::cout << "[LOG] User successfully registered: " << nickname << std::endl;
      response->set_success(true);
    } else {
      std::cout << "[LOG] User already exists: " << nickname << std::endl;
      response->set_success(false);
    }
    
    return Status::OK;
  }

  Status AddProduct(ServerContext* context,
                   const AddProductRequest* request,
                   AddProductResponse* response) override {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::string id = generateProductId();
    Product product;
    product.id = id;
    product.name = request->name();
    product.initial_price = request->initial_price();
    product.current_price = request->initial_price();
    product.seller = request->seller();
    
    products_[id] = product;
    
    std::cout << "[LOG] Product added by " << product.seller 
              << ": " << product.name << " (ID: " << id << ")"
              << " with initial price of " << product.initial_price << std::endl;
    
    response->set_success(true);
    response->set_product_id(id);
    
    return Status::OK;
  }

  Status GetProducts(ServerContext* context,
                    const GetProductsRequest* request,
                    GetProductsResponse* response) override {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::cout << "[LOG] Products list requested, size: " << products_.size() << std::endl;
    
    for (const auto& pair : products_) {
      const Product& product = pair.second;
      ProductInfo* info = response->add_products();
      info->set_id(product.id);
      info->set_name(product.name);
      info->set_initial_price(product.initial_price);
      info->set_current_price(product.current_price);
      info->set_seller(product.seller);
    }
    
    return Status::OK;
  }

  Status PlaceBid(ServerContext* context,
                 const PlaceBidRequest* request,
                 PlaceBidResponse* response) override {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::string product_id = request->product_id();
    std::string bidder = request->bidder();
    double amount = request->amount();
    
    std::cout << "[LOG] " << bidder << " placed bid of $" << amount 
              << " for product " << product_id << std::endl;
    
    auto it = products_.find(product_id);
    if (it != products_.end() && amount > it->second.current_price) {
      it->second.current_price = amount;
      
      Bid bid;
      bid.bidder = bidder;
      bid.product_id = product_id;
      bid.amount = amount;
      bids_.push_back(bid);
      
      std::cout << "[LOG] Bid placed successfully for product " << product_id 
                << " new price: " << amount << std::endl;
      response->set_success(true);
    } else {
      std::cout << "[LOG] Bid failed for product " << product_id 
                << " amount: " << amount << std::endl;
      response->set_success(false);
    }
    
    return Status::OK;
  }
};

void RunServer() {
  std::string addr = "0.0.0.0:50051";
  AuctionService service;
  
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();
  ServerBuilder builder;
  builder.AddListeningPort(addr, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Auction Server listening on " << addr << std::endl;
  
  server->Wait();
}

int main() {
  RunServer();
  return 0;
}
