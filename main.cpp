#include "json.h"
#include "StopsBase.h"
#include "Bus.h"
#include "graph.h"
#include "router.h"
#include "Transport.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <memory>
#include <optional>
using namespace std;


struct StatsRequest{
    string type;
    string name;
    string from;
    string to;
    long long request_id;
};

void PrintBusResult(const StatsRequest& stat,
    shared_ptr<BusManager> bus_base){
    if(!bus_base->HasBus(stat.name)){
        cout << "   " << '"' << "request_id" << '"' << ": " << stat.request_id << "," << endl;
        cout << "   " << '"' << "error_message" << '"' << ": " << '"' << "not found" << '"' << endl;
    }
    else{
        cout << "   "  << '"' << "route_length" << '"' << ": " << bus_base->GetRouteLength(stat.name) << "," << endl;
        cout << "   "  << '"' << "request_id" << '"' << ": " << stat.request_id << ","<< endl; 
        cout << "   "  << '"' << "curvature" << '"' << ": " << bus_base->GetCurvature(stat.name) << "," << endl;
        cout << "   "  << '"' << "stop_count" << '"' << ": " << bus_base->GetCountStops(stat.name) << "," << endl;
        cout << "   "  << '"' << "unique_stop_count" << '"' << ": " << bus_base->GetCountUniqueStops(stat.name) << endl;
    }
}

void PrintStopResult(const StatsRequest& stat, shared_ptr<StopManager> stops_base) {
    if (!stops_base->HasStop(stat.name)) {
        cout << "   " << '"' << "request_id" << '"' << ": " << stat.request_id << "," << endl;
        cout << "   " << '"' << "error_message" << '"' << ": " << '"' << "not found" << '"' << endl;
    } else {
        cout << "   " << '"' << "buses" << '"' << ": [";
        if (stops_base->HasBusesOnStop(stat.name)) {
            cout << stops_base->GetStopInfo(stat.name);
        }
        cout << "]," << endl; 
        cout << "   " << '"' << "request_id" << '"' << ": " << stat.request_id << endl;
    }
}

void PrintRouteResult(const StatsRequest& stat, const TransportSystem& transport_system) {
    auto route_response = transport_system.FindRoute(stat.from, stat.to);
    if (!route_response) {
        cout << "   " << '"' << "request_id" << '"' << ": " << stat.request_id << "," << endl;
        cout << "   " << '"' << "error_message" << '"' << ": " << '"' << "not found" << '"' << endl;
        return;
    }
    cout << "   " << '"' << "items" << '"' << ": [" << endl;
    cout << route_response->items;
    cout << "   " << "]," << endl;
    cout << "   " << '"' << "total_time" << '"' << ": " << route_response->total_time << "," << endl;
    cout << "   " << '"' << "request_id" << '"' << ": " << stat.request_id << endl;
}

void PrintResult(const vector<StatsRequest>& stats_request,
    shared_ptr<StopManager> stops_base, 
    shared_ptr<BusManager> bus_base,const TransportSystem& transport_system){
    cout << '[' << endl;
    for(int i = 0; i < stats_request.size(); i++){
        cout << "  {" << endl;
        if(stats_request[i].type == "Stop"){
            PrintStopResult(stats_request[i],stops_base);
        }
        else if(stats_request[i].type == "Bus"){
            PrintBusResult(stats_request[i],bus_base);
        }
        else if(stats_request[i].type == "Route"){
            PrintRouteResult(stats_request[i],transport_system);
        }
        cout << "  }";
        if(i + 1 < stats_request.size()){
            cout << ',';
        }
        cout << endl;
    }
    cout << ']' << endl;
}

void CalculateAndSetBusParams(shared_ptr<BusManager> bus_base, const string& bus_num){
    bus_base->CalculateAndSetGeographicalLength(bus_num);
    bus_base->CalculateAndSetRoadLength(bus_num);
    bus_base->CalculateAndSetCurvature(bus_num);
    bus_base->CalculateAndSetUniqueStopCount(bus_num);
    bus_base->CalculateAndSetStopCount(bus_num);
}

vector<StatsRequest> ReadStatsRequests(const vector<Json::Node>& stats_request){
    vector<StatsRequest> result;
    for(const auto& request_node : stats_request){
        const auto& request = request_node.AsMap();
        if(request.at("type").AsString() == "Route"){
            result.push_back({
                request.at("type").AsString(),
                {},
                request.at("from").AsString(),
                request.at("to").AsString(),
                static_cast<long long>(request.at("id").AsDouble())
            });
        }
        else{
            result.push_back({
                request.at("type").AsString(),
                request.at("name").AsString(),
                {},
                {},
                static_cast<long long>(request.at("id").AsDouble())
            });
        }
    }
    return result;
}

void ProcessStatsRequest(const vector<StatsRequest>& stats_request,
    shared_ptr<StopManager> stops_base,
    shared_ptr<BusManager> bus_base){
        for(const auto& stat : stats_request){
            if(stat.type == "Bus"){
                CalculateAndSetBusParams(bus_base,stat.name);
            }
        }

}

void ProcessBaseRequest(const map<string,Json::Node>& request,
    shared_ptr<StopManager> stops_base,
    shared_ptr<BusManager> bus_base) {
    const auto& type = request.at("type").AsString();
    if(type == "Stop"){
        auto [stop_name,stop_info] = stops_base->ProcessStopRequest(request);
        stops_base->AddStop(move(stop_name),move(stop_info));
    }
    else if(type == "Bus"){
        auto [bus_name,bus_info] = bus_base->ProcessBusRequest(request);
        bus_base->AddBusToStop(bus_name,bus_info);
        bus_base->AddBus(bus_name,bus_info);
    }
}


pair<shared_ptr<StopManager>,shared_ptr<BusManager>> CreateManagerWithConfig(const Json::Document& doc){
    auto stops_base = make_shared<StopManager>();
    auto bus_base = make_shared<BusManager>(stops_base);
    const auto& root = doc.GetRoot().AsMap();
    auto request_node = root.at("routing_settings");
    bus_base->AddBusRoutingSettings(request_node);
    return {move(stops_base),move(bus_base)};
}


tuple<vector<StatsRequest>,shared_ptr<StopManager>,shared_ptr<BusManager>> ProcessInput(const Json::Document& doc) {
    auto[stops_base,bus_base] = CreateManagerWithConfig(doc);
    const auto& root = doc.GetRoot().AsMap();
    for(const auto& node : root.at("base_requests").AsArray()){
        ProcessBaseRequest(node.AsMap(),stops_base,bus_base);
    }
    const auto& stats_request = ReadStatsRequests(root.at("stat_requests").AsArray());
    ProcessStatsRequest(stats_request,stops_base,bus_base);
    return {stats_request,stops_base,bus_base};
}

Json::Document ParceInput(istream& input = cin){
    auto doc = Json::Load(input);
    return doc;
}


int main(){
    setprecision(6);
    Json::Document document = ParceInput();
    auto [stats_request,stops_base,bus_base] = ProcessInput(document);
    TransportSystem transport_system(bus_base,stops_base);
    PrintResult(stats_request,stops_base,bus_base,transport_system);
    return 0;
}