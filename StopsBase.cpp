#include "StopsBase.h"

using namespace std;

ostream& operator << (ostream& out, const StopManager::StopsInfo& stops_info) {
    bool first = true;
    for (const auto& bus : stops_info.buses_) {
        if (!first) {
            out << ", ";
        }
        out << '"' << bus << '"';
        first = false;
    }
    return out;
}

void Coordinates::ConvertToRadians(){
    latitude = latitude * Pi / 180;
    longitude = longitude * Pi / 180;
}

tuple<string,StopManager::StopsInfo> StopManager::ProcessStopRequest(const map<std::string,Json::Node>& request){
    StopsInfo stop_info;
    auto stop_name = request.at("name").AsString();
    auto res = request.at("latitude").AsDouble();
    stop_info.coordinates.latitude = request.at("latitude").AsDouble();
    stop_info.coordinates.longitude = request.at("longitude").AsDouble();
    stop_info.coordinates.ConvertToRadians();
    const auto dist = request.at("road_distances").AsMap();
    for(const auto&[to_stop,distance]: dist){
        stop_info.stops_to_distances_[to_stop] = distance.AsDouble();
    }
    return {stop_name,stop_info};
}

void StopManager::AddStop(const std::string& stop_name,const StopManager::StopsInfo& stop_info){
    if(stops_data_.find(stop_name) == stops_data_.end()){
        stops_data_[stop_name] = stop_info;
    }
    else{
        for(const auto& bus : stop_info.buses_){
            stops_data_[stop_name].buses_.insert(bus);
        }
        for(const auto& [key,val] : stop_info.stops_to_distances_){
            stops_data_[stop_name].stops_to_distances_[key] = val;
        }
        stops_data_[stop_name].coordinates = stop_info.coordinates;
    }
}

void StopManager::AddStopBus(const std::string& bus_num,const std::string& stop_name){
    stops_data_[stop_name].buses_.insert(bus_num);
}

bool StopManager::HasBusOnStop(const std::string& stop_name,const std::string& bus_name) const{
    auto it = stops_data_.find(stop_name);
    if(it != stops_data_.end() && it->second.buses_.count(bus_name) != 0){
        return true;
    }
    return false;
}

optional<Coordinates> StopManager::GetStopCoordinates(const std::string& stop_name) const{
    if(stops_data_.count(stop_name) != 0){
        return stops_data_.at(stop_name).coordinates;
    }
    return nullopt;
}

const StopManager::StopsInfo& StopManager::GetStopInfo(const std::string& stop_name) const{
    return stops_data_.at(stop_name);
}

double StopManager::GetDistance(const string& from_stop,const string& to_stop) const{
    const auto& from_stop_key = stops_data_.at(from_stop);
    auto from_stop_it = from_stop_key.stops_to_distances_.find(to_stop);
    if(from_stop_it != from_stop_key.stops_to_distances_.end()){
        return from_stop_it->second;
    }
    const auto& to_stop_key = stops_data_.at(to_stop);
    auto to_stop_it = to_stop_key.stops_to_distances_.find(from_stop);
    if(to_stop_it != from_stop_key.stops_to_distances_.end()){
        return to_stop_it->second;
    }
    return 0.0;
}

size_t StopManager::GetDataSize() const{
    return stops_data_.size();
}

bool StopManager::HasBusesOnStop(const std::string& stop_name) const {
    auto it = stops_data_.find(stop_name);
    if (it == stops_data_.end() || it->second.buses_.empty()) {
        return false;
    }
    return true;
}

bool StopManager::HasStop(const std::string& stop_name) const {
    if(stops_data_.find(stop_name) == stops_data_.end()){
        return false;
    }
    return true;
}