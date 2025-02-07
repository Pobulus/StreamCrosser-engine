#include <iostream>
#include "includes/json.hpp"
#include "includes/httplib.h"

#include "ProcessingNodes/Engine.hpp"

#include <unordered_map>
#include <memory>

using namespace std;

int main(int argc, char **argv) {

    std::cout.precision(10);
    YAML::Node config;
    try {
        config = YAML::LoadFile("conf.yaml");
        cout << "Applying config: " << config["configName"] << endl;
    } catch (YAML::BadFile exc) {
        cerr << exc.what() << endl;
        return 1;
    }


    httplib::Server server;
    if (!server.set_mount_point("/", "./web")) {
        std::cerr << "Couldn't open server dir" << std::endl;
        return 1;
    }
    Engine engine;
    engine.configure(config);


    std::ifstream queryFS("query.json");
    json query;
    try {query = json::parse(queryFS);} catch (nlohmann::json_abi_v3_11_3::detail::parse_error err) {query = json::parse("{}");}
    try {
        engine.installQuery(query);
    } catch(EngineException exc) {
        cerr << exc.what() << endl;
        return 1;
    }
    cout << "Config done!\n" << endl;

    try {
        engine.startServer();
    } catch(EngineException exc) {
        cout << exc.what() << endl;
        return -1;
    }
}
