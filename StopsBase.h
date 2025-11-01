#pragma once
#include "json.h"
#include <unordered_map>
#include <map>
#include <optional>
#include <set>
#include <tuple>
#include <string>
#include <string_view>
#include <iostream>

static const double Pi = 3.1415926535;
static const double Earth_Radius = 6371000;

struct Coordinates{
    void ConvertToRadians();
    double latitude;
    double longitude;
};

class BusManager;

class StopManager{
public:
    friend class BusManager;
    struct StopsInfo{
        Coordinates coordinates;
        std::set<std::string> buses_;
        std::unordered_map<std::string,double> stops_to_distances_;
    };
    StopManager() = default;
    ~StopManager() = default;

    std::tuple<std::string,StopsInfo>ProcessStopRequest(const std::map<std::string,Json::Node>& request);
    void AddStop(const std::string& stop_name,const StopsInfo& stop_info);
    
    auto begin() const {
        return stops_data_.begin();
    }
    auto end() const{
        return stops_data_.end();
    }
    const StopsInfo& GetStopInfo(const std::string& stop_name) const;
    std::optional<Coordinates> GetStopCoordinates(const std::string& stop_name) const;
    size_t GetDataSize() const;
    double GetDistance(const std::string& from_stop,const std::string& to_stop) const;

    bool HasBusesOnStop(const std::string& stop_name) const;
    bool HasBusOnStop(const std::string& stop_name,const std::string& bus_name) const;
    bool HasStop(const std::string& stop_name) const;

private:

    void AddStopBus(const std::string& bus_num,const std::string& stop_name);
    
    std::unordered_map<std::string,StopsInfo> stops_data_;
};

std::ostream& operator << (std::ostream& out,const StopManager::StopsInfo& stops_info);
