#pragma once
#include "graph.h"
#include "router.h"
#include "StopsBase.h"
#include "Bus.h"
#include <iostream>
#include <memory>
#include <unordered_map>
#include <string>
#include <vector>
#include <optional>
#include <variant>


struct BusResponce{
    std::string bus_name;
    int span_count;
    double travel_time;
};

struct StopResponce{
    std::string stop_name;
    double wait_time;
};

struct RouteResponse{
    double total_time;
    std::vector<std::variant<BusResponce,StopResponce>> items;
};

struct EdgeInfo {
    std::string bus_name;
    int span_count;
    double travel_time;
};

class TransportSystem{
private:
    std::shared_ptr<BusManager> bus_base_ptr_;
    std::shared_ptr<StopManager> stops_base_ptr_;
    Graph::DirectedWeightedGraph<double> graph_;
    std::unordered_map<std::string, Graph::VertexId> stop_to_vertex_;
    std::unordered_map<Graph::VertexId, std::string> vertex_to_stop_;
    std::unique_ptr<Graph::Router<double>> router_;
    std::unordered_map<Graph::EdgeId, EdgeInfo> edge_info_;
public:
    TransportSystem(std::shared_ptr<BusManager> bus_base,std::shared_ptr<StopManager> stop_base);
    std::optional<RouteResponse> FindRoute(const std::string& from,const std::string& to) const;
private:
    RouteResponse ConvertRouteInfoToResponse(const Graph::Router<double>::RouteInfo& route_info) const;
    void AddBusToGraph(const std::string& bus_name,const BusManager::BusInfo& bus_info);
    void BuildGraph();
    void BuildRouter();
};


std::ostream& operator << (std::ostream& out,const std::vector<std::variant<BusResponce,StopResponce>>& items);