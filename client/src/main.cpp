#include "app_config.h"
#include "auction_client.h"
#include <SDL3/SDL_vulkan.h>
#include <grpcpp/grpcpp.h>
#include <cstring>

// Application state
struct AppState {
    std::unique_ptr<AuctionClient> client;
    std::string current_user;
    std::vector<ProductData> products;
    bool is_registered;
    char nickname_input[64];
    char product_name_input[128];
    char product_price_input[32];
    char bid_amount_input[32];
    int selected_product;
    std::string status_message;
    float status_timer;
    bool auto_refresh;
    float refresh_timer;
};

void ShowRegistrationWindow(AppState& state) {
    ImGui::Begin("User Registration");
    
    if (state.is_registered) {
        ImGui::Text("Logged in as: %s", state.current_user.c_str());
        if (ImGui::Button("Logout")) {
            state.is_registered = false;
            state.current_user.clear();
        }
    } else {
        ImGui::Text("Enter your nickname to register:");
        ImGui::InputText("Nickname", state.nickname_input, sizeof(state.nickname_input));
        
        if (ImGui::Button("Register")) {
            if (strlen(state.nickname_input) > 0) {
                if (state.client->RegisterUser(state.nickname_input)) {
                    state.current_user = state.nickname_input;
                    state.is_registered = true;
                    state.status_message = "Successfully registered!";
                    state.status_timer = 3.0f;
                } else {
                    state.status_message = "Registration failed: " + state.client->GetLastError();
                    state.status_timer = 3.0f;
                }
            }
        }
    }
    
    ImGui::End();
}

void ShowAddProductWindow(AppState& state) {
    if (!state.is_registered) return;
    
    ImGui::Begin("Add Product");
    
    ImGui::InputText("Product Name", state.product_name_input, sizeof(state.product_name_input));
    ImGui::InputText("Initial Price ($)", state.product_price_input, sizeof(state.product_price_input));
    
    if (ImGui::Button("Add Product")) {
        if (strlen(state.product_name_input) > 0 && strlen(state.product_price_input) > 0) {
            double price = atof(state.product_price_input);
            if (price > 0) {
                std::string product_id;
                if (state.client->AddProduct(state.product_name_input, price, state.current_user, product_id)) {
                    state.status_message = "Product added successfully! ID: " + product_id;
                    state.status_timer = 3.0f;
                    memset(state.product_name_input, 0, sizeof(state.product_name_input));
                    memset(state.product_price_input, 0, sizeof(state.product_price_input));
                } else {
                    state.status_message = "Failed to add product: " + state.client->GetLastError();
                    state.status_timer = 3.0f;
                }
            }
        }
    }
    
    ImGui::End();
}

void ShowProductsWindow(AppState& state, ImGuiIO& io) {
    ImGui::Begin("Auction Products");
    
    ImGui::Checkbox("Auto-refresh", &state.auto_refresh);
    ImGui::SameLine();
    if (ImGui::Button("Refresh Now")) {
        state.products = state.client->GetProducts();
    }
    
    ImGui::Separator();
    ImGui::Text("Products: %zu", state.products.size());
    ImGui::Separator();
    
    if (ImGui::BeginTable("Products", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("Seller");
        ImGui::TableSetupColumn("Initial Price");
        ImGui::TableSetupColumn("Current Price");
        ImGui::TableSetupColumn("Action");
        ImGui::TableHeadersRow();
        
        for (int i = 0; i < (int)state.products.size(); i++) {
            const auto& product = state.products[i];
            ImGui::TableNextRow();
            
            ImGui::TableNextColumn();
            ImGui::Text("%s", product.name.c_str());
            
            ImGui::TableNextColumn();
            ImGui::Text("%s", product.seller.c_str());
            
            ImGui::TableNextColumn();
            ImGui::Text("$%.2f", product.initial_price);
            
            ImGui::TableNextColumn();
            ImGui::Text("$%.2f", product.current_price);
            
            ImGui::TableNextColumn();
            ImGui::PushID(i);
            if (ImGui::Button("Select")) {
                state.selected_product = i;
            }
            ImGui::PopID();
        }
        
        ImGui::EndTable();
    }
    
    // Auto-refresh logic
    if (state.auto_refresh) {
        state.refresh_timer += io.DeltaTime;
        if (state.refresh_timer >= 2.0f) {
            state.products = state.client->GetProducts();
            state.refresh_timer = 0.0f;
        }
    }
    
    ImGui::End();
}

void ShowBiddingWindow(AppState& state) {
    if (!state.is_registered) return;
    
    ImGui::Begin("Place Bid");
    
    if (state.selected_product >= 0 && state.selected_product < (int)state.products.size()) {
        const auto& product = state.products[state.selected_product];
        
        ImGui::Text("Product: %s", product.name.c_str());
        ImGui::Text("Seller: %s", product.seller.c_str());
        ImGui::Text("Current Price: $%.2f", product.current_price);
        ImGui::Separator();
        
        ImGui::InputText("Bid Amount ($)", state.bid_amount_input, sizeof(state.bid_amount_input));
        
        if (ImGui::Button("Place Bid")) {
            if (strlen(state.bid_amount_input) > 0) {
                double amount = atof(state.bid_amount_input);
                if (amount > product.current_price) {
                    if (state.client->PlaceBid(product.id, state.current_user, amount)) {
                        state.status_message = "Bid placed successfully!";
                        state.status_timer = 3.0f;
                        memset(state.bid_amount_input, 0, sizeof(state.bid_amount_input));
                        state.products = state.client->GetProducts(); // Refresh
                    } else {
                        state.status_message = "Failed to place bid: " + state.client->GetLastError();
                        state.status_timer = 3.0f;
                    }
                } else {
                    state.status_message = "Bid must be higher than current price!";
                    state.status_timer = 3.0f;
                }
            }
        }
    } else {
        ImGui::Text("Select a product from the products list to bid.");
    }
    
    ImGui::End();
}

void ShowStatusBar(AppState& state, ImGuiIO& io) {
    if (state.status_timer > 0.0f) {
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y - 50), 
                                ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowBgAlpha(0.8f);
        
        ImGui::Begin("Status", nullptr, 
                     ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | 
                     ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | 
                     ImGuiWindowFlags_NoNav);
        
        ImGui::Text("%s", state.status_message.c_str());
        ImGui::End();
        
        state.status_timer -= io.DeltaTime;
    }
}

int main(int, char**)
{
    // Initialize SDL
    if (!InitializeSDL())
        return 1;

    // Create window
    float mainScale;
    SDL_Window* window = CreateAppWindow(mainScale);
    if (window == nullptr)
        return 1;

    // Setup Vulkan
    VulkanConfig vulkanConfig = {};
    ImVector<const char*> extensions;
    {
        uint32_t sdl_extensions_count = 0;
        const char* const* sdl_extensions = SDL_Vulkan_GetInstanceExtensions(&sdl_extensions_count);
        for (uint32_t n = 0; n < sdl_extensions_count; n++)
            extensions.push_back(sdl_extensions[n]);
    }
    SetupVulkan(extensions, vulkanConfig);

    // Create Window Surface
    VkSurfaceKHR surface;
    if (SDL_Vulkan_CreateSurface(window, vulkanConfig.instance, vulkanConfig.allocator, &surface) == 0)
    {
        printf("Failed to create Vulkan surface.\n");
        return 1;
    }

    // Create Framebuffers
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    ImGui_ImplVulkanH_Window* wd = &vulkanConfig.mainWindowData;
    SetupVulkanWindow(wd, surface, w, h, vulkanConfig);
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(window);

    // Initialize ImGui
    if (!InitializeImGui(window, vulkanConfig, mainScale))
        return 1;

    // Initialize gRPC client
    AppState appState = {};
    appState.client = std::make_unique<AuctionClient>(
        grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials())
    );
    appState.is_registered = false;
    appState.selected_product = -1;
    appState.status_timer = 0.0f;
    appState.auto_refresh = true;
    appState.refresh_timer = 0.0f;
    memset(appState.nickname_input, 0, sizeof(appState.nickname_input));
    memset(appState.product_name_input, 0, sizeof(appState.product_name_input));
    memset(appState.product_price_input, 0, sizeof(appState.product_price_input));
    memset(appState.bid_amount_input, 0, sizeof(appState.bid_amount_input));

    ImVec4 clear_color = ImVec4(0.1f, 0.1f, 0.15f, 1.00f);

    // Main loop
    bool done = false;
    ImGuiIO& io = ImGui::GetIO();
    
    while (!done)
    {
        // Poll and handle events
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT)
                done = true;
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

        // Skip rendering if minimized
        if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED)
        {
            SDL_Delay(10);
            continue;
        }

        // Handle window resize
        int fb_width, fb_height;
        SDL_GetWindowSize(window, &fb_width, &fb_height);
        if (fb_width > 0 && fb_height > 0 && 
            (vulkanConfig.swapChainRebuild || vulkanConfig.mainWindowData.Width != fb_width || 
             vulkanConfig.mainWindowData.Height != fb_height))
        {
            ImGui_ImplVulkan_SetMinImageCount(vulkanConfig.minImageCount);
            ImGui_ImplVulkanH_CreateOrResizeWindow(vulkanConfig.instance, vulkanConfig.physicalDevice, 
                vulkanConfig.device, wd, vulkanConfig.queueFamily, vulkanConfig.allocator, 
                fb_width, fb_height, vulkanConfig.minImageCount, 0);
            vulkanConfig.mainWindowData.FrameIndex = 0;
            vulkanConfig.swapChainRebuild = false;
        }

        // Start the Dear ImGui frame
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        // Show application windows
        ShowRegistrationWindow(appState);
        ShowAddProductWindow(appState);
        ShowProductsWindow(appState, io);
        ShowBiddingWindow(appState);
        ShowStatusBar(appState, io);

        // Rendering
        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();
        const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
        if (!is_minimized)
        {
            wd->ClearValue.color.float32[0] = clear_color.x * clear_color.w;
            wd->ClearValue.color.float32[1] = clear_color.y * clear_color.w;
            wd->ClearValue.color.float32[2] = clear_color.z * clear_color.w;
            wd->ClearValue.color.float32[3] = clear_color.w;
            FrameRender(wd, draw_data, vulkanConfig);
            FramePresent(wd, vulkanConfig);
        }
    }

    // Cleanup
    VkResult err = vkDeviceWaitIdle(vulkanConfig.device);
    CheckVkResult(err);
    
    CleanupImGui();
    CleanupVulkanWindow(vulkanConfig);
    CleanupVulkan(vulkanConfig);
    CleanupSDL(window);

    return 0;
}