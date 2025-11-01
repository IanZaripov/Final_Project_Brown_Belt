#include "Transport.h"

using namespace std;

TransportSystem::TransportSystem(shared_ptr<BusManager> bus_base,shared_ptr<StopManager> stop_base) : 
bus_base_ptr_(bus_base), stops_base_ptr_(stop_base), graph_(stops_base_ptr_->GetDataSize()){
    BuildGraph();
    BuildRouter();
}

void TransportSystem::AddBusToGraph(const string& bus_name,const BusManager::BusInfo& bus_info){
    const auto routing_settings = bus_base_ptr_->GetRouteSettings();
    for(size_t start_idx = 0; start_idx < bus_info.stops_sequence.size(); start_idx++){
        double travel_time = 0.0;
        int spant_count = 0;
        for(size_t end_idx = start_idx + 1; end_idx < bus_info.stops_sequence.size(); end_idx++){
            const auto& from_stop = bus_info.stops_sequence[end_idx-1];
            const auto& to_stop = bus_info.stops_sequence[end_idx];
            
            double distance = stops_base_ptr_->GetDistance(from_stop,to_stop);
            double segment_travel_time = distance / routing_settings.bus_velocity;
            travel_time += segment_travel_time;
            spant_count++;
            double total_time = routing_settings.bus_wait_time + travel_time;

            Graph::VertexId from_v = stop_to_vertex_[bus_info.stops_sequence[start_idx]];
            Graph::VertexId to_v = stop_to_vertex_[to_stop];
            Graph::EdgeId edge_id  = graph_.AddEdge({from_v,to_v,total_time});
            edge_info_[edge_id] = {bus_name,spant_count,travel_time};
        }
    }
}

void TransportSystem::BuildGraph(){
    Graph::VertexId vertex_id = 0;
    Graph::DirectedWeightedGraph<double> graph(stops_base_ptr_->GetDataSize());
    for(const auto& it : *stops_base_ptr_){
        stop_to_vertex_[it.first] = vertex_id;
        vertex_to_stop_[vertex_id] = it.first;
        vertex_id++;
    }

    for(const auto& it : *bus_base_ptr_){
        AddBusToGraph(it.first,it.second);
    }
    
}

void TransportSystem::BuildRouter(){
    router_ = make_unique<Graph::Router<double>>(graph_);
}

RouteResponse TransportSystem::ConvertRouteInfoToResponse(const Graph::Router<double>::RouteInfo& route_info) const {
    RouteResponse response;
    response.total_time = route_info.weight;
    
    if (route_info.edge_count == 0) return response;
    
    Graph::EdgeId first_edge_id = router_->GetRouteEdge(route_info.id, 0);
    const auto& first_edge = graph_.GetEdge(first_edge_id);
    
    response.items.push_back(StopResponce{
        vertex_to_stop_.at(first_edge.from),
        bus_base_ptr_->GetRouteSettings().bus_wait_time
    });

    for (size_t i = 0; i < route_info.edge_count; ++i) {
        Graph::EdgeId edge_id = router_->GetRouteEdge(route_info.id, i);
        const auto& edge = graph_.GetEdge(edge_id);
        const auto& info = edge_info_.at(edge_id);
        response.items.push_back(BusResponce{
            info.bus_name, 
            info.span_count, 
            info.travel_time
        });
        
        if (i < route_info.edge_count - 1) {
            response.items.push_back(StopResponce{
                vertex_to_stop_.at(edge.to),
                bus_base_ptr_->GetRouteSettings().bus_wait_time
            });
        }
    }
    return response;
}

optional<RouteResponse> TransportSystem::FindRoute(const string& from,const string& to) const {
    if(!stop_to_vertex_.count(from) || !stop_to_vertex_.count(to)){
        return nullopt;
    }
    auto from_v = stop_to_vertex_.at(from);
    auto to_v = stop_to_vertex_.at(to);
    auto route_info = router_->BuildRoute(from_v,to_v);
    if(!route_info){
        return nullopt;
    }
    return ConvertRouteInfoToResponse(*route_info);
}

ostream& operator << (ostream& out,const vector<variant<BusResponce,StopResponce>>& items){
    for (size_t i = 0; i < items.size(); i++) {
        if (std::holds_alternative<StopResponce>(items[i])) {
            const auto& wait = get<StopResponce>(items[i]);
            out << "    {" << endl;
            out << "      " << '"' << "type" << '"' << ": " << '"' << "Wait" << '"' << "," << endl;
            out << "      " << '"' << "stop_name" << '"' << ": " << '"' << wait.stop_name << '"' << "," << endl;
            out << "      " << '"' << "time" << '"' << ": " << wait.wait_time << endl;
            out << "    }";
        } 
        else {
            const auto& bus = std::get<BusResponce>(items[i]);
            out << "    {" << endl;
            out << "      " << '"' << "type" << '"' << ": " << '"' << "Bus" << '"' << "," << endl;
            out << "      " << '"' << "bus" << '"' << ": " << '"' << bus.bus_name << '"' << "," << endl;
            out << "      " << '"' << "span_count" << '"' << ": " << bus.span_count << "," << endl;
            out << "      " << '"' << "time" << '"' << ": " << bus.travel_time << endl;
            out << "    }";
        }
        
        if (i + 1 < items.size()) {
            out << ",";
        }
        out << endl;
    }
    return out;
}