#include "Bus.h"

using namespace std;

BusManager::BusInfo BusManager::ProcessStops(const map<string,Json::Node>& request){
    BusInfo bus_info;
    vector<string> route;
    const auto& stops = request.at("stops").AsArray();
    for(const auto& stop : stops){
        route.push_back(stop.AsString());
    }
    bus_info.is_round_trip = request.at("is_roundtrip").AsBool();
    if(!bus_info.is_round_trip && route.size() > 1){
        vector<string> forward_route;
        for(int i = route.size()-2 ; i >= 0; i--){
            forward_route.push_back(route[i]);
        }
        for(int i = 0; i < forward_route.size(); i++){
            route.push_back(forward_route[i]);
        }
    }
    bus_info.stops_sequence = move(route);
    return bus_info;
}

tuple<string,BusManager::BusInfo> BusManager::ProcessBusRequest(const map<string,Json::Node>& request){
    auto bus_info = ProcessStops(request);
    auto bus_num = request.at("name").AsString();
    return {move(bus_num),move(bus_info)};
}

void BusManager::AddBusToStop(const std::string& bus_num,const BusManager::BusInfo& bus_stops){
    for(const auto& stop : bus_stops.stops_sequence){
        stop_base_ptr_->AddStopBus(bus_num,stop);
    }
}

void BusManager::AddBus(const std::string& bus_name,const BusInfo& bus){
    bus_data_[bus_name] = bus;
}

void BusManager::AddBusRoutingSettings(const Json::Node& route_settings_node) {
    const auto& settings_map = route_settings_node.AsMap();
    route_settings_.bus_wait_time = settings_map.at("bus_wait_time").AsDouble();
    route_settings_.bus_velocity = settings_map.at("bus_velocity").AsDouble() * 1000 / 60;
}

double BusManager::CalculateAndSetGeographicalLength(const string& bus_num){
    if(bus_data_.count(bus_num) == 0){
        return 0.0;
    }
    auto& bus_info = bus_data_.at(bus_num);
    if(bus_info.geographical_length != 0){
        return bus_info.geographical_length;
    }
    const auto& stops = bus_data_.at(bus_num).stops_sequence;
    if(stops.size() < 2){
        return 0.0;
    }
    double route_length = 0;
    for(size_t i = 1; i < stops.size(); i++){
        const auto coord1 = stop_base_ptr_->GetStopCoordinates(stops[i-1]);
        const auto coord2 = stop_base_ptr_->GetStopCoordinates(stops[i]);
        if(coord1 && coord2){
            route_length += acos(sin(coord1->latitude) * sin(coord2->latitude) + cos(coord1->latitude) 
            * cos(coord2->latitude) * cos(abs(coord1->longitude - coord2->longitude))) * Earth_Radius;
        }
    }
    bus_info.geographical_length = route_length;
    return bus_info.geographical_length;
}

double BusManager::CalculateAndSetRoadLength(const std::string& bus_num){
    if(bus_data_.count(bus_num) == 0){
        return 0.0;
    }
    const auto& stops = bus_data_.at(bus_num).stops_sequence;
    auto& bus_info = bus_data_.at(bus_num);
    if(bus_info.road_length != 0.0){
        return bus_info.road_length;
    }
    if(stops.size() < 2){
        return 0.0;
    }
    double road_length = 0;
    for(size_t i = 1; i < stops.size(); i++){
        const auto& stop1 = stops[i-1];
        const auto& stop2 = stops[i];
        road_length += stop_base_ptr_->GetDistance(stop1,stop2);
    }
    bus_info.road_length = road_length;
    return bus_info.road_length;
}

double BusManager::CalculateAndSetCurvature(const string& bus_num){
    if(bus_data_.count(bus_num) == 0){
        return 0.0;
    }
    auto& bus_info = bus_data_.at(bus_num);
    if(bus_info.curvature == 0.0){
        bus_info.curvature = bus_info.road_length / bus_info.geographical_length;
    }
    return bus_info.curvature;
}

double BusManager::CalculateAndSetUniqueStopCount(const string& bus_num){
    if(bus_data_.count(bus_num) == 0){
        return 0.0;
    }
    auto& bus_info = bus_data_.at(bus_num);
    if(bus_info.unique_stops == 0.0){
        unordered_set<string> unique_stops(bus_info.stops_sequence.begin(),bus_info.stops_sequence.end());
        bus_info.unique_stops = unique_stops.size();
    }
    return bus_info.unique_stops;
}

double BusManager::CalculateAndSetStopCount(const string& bus_num){
    if(bus_data_.count(bus_num) == 0){
        return 0.0;
    }
    auto& bus_info = bus_data_.at(bus_num);
    bus_info.cnt_stops = bus_info.stops_sequence.size();
    return bus_info.cnt_stops;
}

BusManager::RouteSettings BusManager::GetRouteSettings() const{
    return route_settings_;
}

int BusManager::GetCountStops(const std::string& bus_num) const {
    if(bus_data_.count(bus_num) == 0){
        return 0;
    }
    return bus_data_.at(bus_num).cnt_stops;
}

int BusManager::GetCountUniqueStops(const std::string& bus_num) const{
    if(bus_data_.count(bus_num) == 0){
        return 0;
    }
    return bus_data_.at(bus_num).unique_stops;
}

double BusManager::GetRouteLength(const std::string& bus_num) const{
    if(bus_data_.count(bus_num) == 0){
        return 0.0;
    }
    auto& bus_info = bus_data_.at(bus_num);
    return bus_info.road_length;
}

double BusManager::GetCurvature(const std::string& bus_num) const{
    if(bus_data_.count(bus_num) == 0){
        return 0.0;
    }
    auto& bus_info = bus_data_.at(bus_num);
    return bus_info.curvature;
}

bool BusManager::HasBus(const std::string& bus_num) const{
    return bus_data_.count(bus_num);
}