#pragma once
#include "StopsBase.h"
#include "json.h"
#include <memory>
#include <tuple>
#include <unordered_set>
#include <unordered_map>
#include <string_view>
#include <string>
#include <vector>
#include <cmath>

class BusManager{
public:
    BusManager(std::shared_ptr<StopManager> stops_base) : stop_base_ptr_(stops_base){}
    struct BusInfo{
        std::vector<std::string> stops_sequence;
        int cnt_stops = 0;
        int unique_stops = 0;
        double geographical_length = 0.0;
        double road_length = 0.0;
        double curvature = 0.0;
        bool is_round_trip = false;
    };

    struct RouteSettings{
        double bus_wait_time = 0;
        double bus_velocity = 0;
    };

    auto begin() const{
        return bus_data_.begin();
    }
    
    auto end() const {
        return bus_data_.end();
    }

    std::tuple<std::string,BusInfo> ProcessBusRequest(const std::map<std::string,Json::Node>& request);
    void AddBus(const std::string& bus_num,const BusInfo& bus);
    void AddBusToStop(const std::string& bus_num,const BusInfo& bus_stops);
    void AddBusRoutingSettings(const Json::Node& route_settings_node);

    double CalculateAndSetGeographicalLength(const std::string& bus_num);
    double CalculateAndSetRoadLength(const std::string& bus_num);
    double CalculateAndSetCurvature(const std::string& bus_num);
    double CalculateAndSetUniqueStopCount(const std::string& bus_num);
    double CalculateAndSetStopCount(const std::string& bus_num);

    double GetRouteLength(const std::string& bus_num) const;
    int GetCountStops(const std::string& bus_num) const;
    int GetCountUniqueStops(const std::string& bus_num) const;
    double GetCurvature(const std::string& bus_num) const;
    RouteSettings GetRouteSettings() const;

    bool HasBus(const std::string& bus_num) const;
private:

    BusInfo ProcessStops(const std::map<std::string,Json::Node>& request);

    std::unordered_map<std::string, BusInfo> bus_data_;
    std::shared_ptr<StopManager> stop_base_ptr_;
    RouteSettings route_settings_;
};
