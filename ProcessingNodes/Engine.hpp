#pragma once
#include "SourceTable.hpp"
#include "SourceStream.hpp"

#include "Output.hpp"
#include "StreamOutput.hpp"
#include "Join.hpp"
#include "Prop.hpp"
#include "Set.hpp"
#include "Filter.hpp"
#include "Expression.hpp"
#include "EngineExceptions.hpp"
#include "Constant.hpp"
#include "Window.hpp"
#include <cstdlib>

#define ATTACH(X, Y, Z) (X).attachOutput([Y, Z](json data){(Y)->dataInput(data, Z);});

class Engine {
    httplib::Server server;
    unordered_map<string, shared_ptr<IInputNode>> inputs;


    unordered_map<int, shared_ptr<IProcNode>> processingNodes;
    unordered_map<string, shared_ptr<Output>> outputs;

    void initServer() {
        if (!server.set_mount_point("/", "./web")) {
            std::cerr << "couldn't open server dir" << std::endl;
            throw EngineException("web dir missing");
        }

    }
public:
    Engine() { }
    void configure(YAML::Node config) {
        cout << "\n=== CONFIGURING: SERVER ===" << endl;
        initServer();
        // Returns the list of streams from config for the StreamSim
        server.Get("/str", [config](const httplib::Request& /*req*/, httplib::Response& res) {
            YAML::Emitter emitter;
            emitter << YAML::DoubleQuoted << YAML::Flow << YAML::BeginSeq << config["streams"];
            res.set_content(emitter.c_str()+1, "application/json");
        });
        // request handler for queries
        server.Post("/query", [](const httplib::Request& req, httplib::Response& res) {
            string cmd = "./parser '"+req.body+"' > query.json";
            cout << "Parsing query" << req.body << endl;
            system(cmd.c_str());
            std::ifstream queryFS("query.json");
            std::string queryStr((std::istreambuf_iterator<char>(queryFS)), std::istreambuf_iterator<char>());
            res.set_content(queryStr, "application/json");
        });
        server.Get("/out", [this](const httplib::Request& req, httplib::Response& res) {
            vector<string> out;
            for(auto output : this->outputs) {
                out.push_back(output.second->getName());
            }
            cout << (json)out << endl;
            res.set_content(((json) out).dump(), "application/json");
        });

        server.Post("/out", [this](const httplib::Request& req, httplib::Response& res) {
            string name = req.body;
            if(this->outputs.count(name))
                res.set_content(this->outputs[name]->getData().dump(), "application/json");
        });

        server.Post("/install", [this](const httplib::Request& req, httplib::Response& res) {
            try {
                this->installQuery(json::parse(req.body));
                res.set_content("ok", "text/plain");
                this->reload();
            } catch(EngineException exc) {
                res.set_content(exc.what(), "text/plain");
            }
        });
        server.Post("/start", [this](const httplib::Request& req, httplib::Response& res) {
            try {
                this->installQuery(json::parse(req.body));
                res.set_content("ok", "text/plain");
            } catch(EngineException exc) {
                res.set_content(exc.what(), "text/plain");
            }
        });
        cout << "\n=== CONFIGURING: STREAMS ===" << endl;

        for(auto stream : config["streams"]){
            auto name = stream["name"].as<string>();
            inputs[name] = make_shared<SourceStream>(stream);
            server.Post("/str/"+name, [stream, this](const httplib::Request& req, httplib::Response& res) {
                cout << "Stream data for: "<< stream["name"] << ':' << req.body << std::endl;
                auto input = this->inputs[stream["name"].as<string>()];
                if(input) {
                    input->dataInput(json::parse(req.body));
                    res.set_content("data delivered", "text/plain");
                } else res.set_content("no processing node connected to this stream", "text/plain");

            });
            server.Get("/str/"+name, [stream](const httplib::Request& req, httplib::Response& res) {
                YAML::Emitter emitter;
                emitter << YAML::DoubleQuoted << YAML::Flow << YAML::BeginSeq << stream;
                res.set_content(emitter.c_str()+1, "application/json");
                cout  << stream["name"] << endl;
                // string resp = "{";
                // for(auto it=stream["schema"].begin();it!=stream["schema"].end();++it) {
                //     resp += "\""+it->first.as<string>()+"\":"+"\""+it->second.as<string>()+"\",";
                // }
                // if(resp.length() > 1) resp.pop_back(); // trailing comma
                // resp += "}";
                // res.set_content(resp, "application/json");
            });
        }
        cout << "\n=== CONFIGURING: TABLES ===" << endl;
        for(auto tableDef : config["tables"]){
            auto table = make_shared<SourceTable>(tableDef);
            cout << tableDef << endl;
            inputs[tableDef["name"].as<string>()] = table;
        }
    }
    void installQuery(json query) {
        // clear projection data from inputs
        for(auto inp : inputs) { cout << inp.first << endl; inp.second->reset();}
        // for(auto inp : inputTables) { cout << inp.first << endl; inp.second->reset();}
        processingNodes.clear();
        outputs.clear();
        cout << "\n=== INSTALLING: QUERY VERIFICATION ===" << endl;
        for(auto source : query["sources"]) {
            if(inputs.count(source["text"])) {
                processingNodes[source["id"]] = inputs[source["text"]]; // add to node map
                string srcId = std::to_string((int) source["id"]);
                // TODO: mutidimensional prop handling on source tables
                for(auto prop : query["props"].items()) {
                    const string propName = (string) prop.key();
                    int dotIndex = propName.find(".");
                    string propSrc = propName.substr(0, dotIndex);
                    if(propSrc == srcId) {
                        inputs[source["text"]]->selectProp(propName.substr(dotIndex+1, string::npos));
                    }
                }
            } else throw EngineException("No source with name " + (string) source["text"]);
        }
        cout << "\n=== INSTALLING: CREATE ===" << endl;

        // Create nodes
        for(auto node : query["nodes"]) {
            const int id = node["id"];
            const string type = node["type"];
            if(type == "join") {
            processingNodes[id]= make_shared<Join>(node["text"]);
            } else if(type == "tabl") {
            auto output = make_shared<Output>(node["text"]);
            processingNodes[id] = output;
            outputs[node["text"]] = output;
            } else if(type == "strm") {
            auto output = make_shared<StreamOutput>(node["text"]);
            processingNodes[id] = output;
            outputs[node["text"]] = output;
            } else if(type == "list") {
            processingNodes[id]= make_shared<Set>(node["text"]);
            } else if(type == "prop") {
            processingNodes[id]= make_shared<Prop>(node["text"]);
            } else if(type == "expr") {
            processingNodes[id]= make_shared<Expression>(node["text"]);
            } else if(type == "filt") {
            processingNodes[id]= make_shared<Filter>();
            } else if(type == "cons") {
            processingNodes[id]= make_shared<Constant>(node["text"]);
            } else if(type == "buff") {
            processingNodes[id]= make_shared<Window>();
            }

        }
        cout << "created " << processingNodes.size() << " nodes" << endl;
        cout << "\n=== INSTALLING: ATTACH ===" << endl;
        // Attach nodes to eachother
        for(auto node : query["nodes"]) {
            const int id = node["id"];
            for(int i = 0; i < node["inputs"].size(); i++) {
                auto dest = processingNodes[id];
                auto src = processingNodes[node["inputs"][i]];

                if(src != nullptr && dest != nullptr) {
                    cout << "attaching " << node["inputs"][i] << " to " << id << " on input " << i << endl;
                    ATTACH(*src, dest, i);
                }
            }
        }

        cout << "\n=== INSTALLING: PROJECTION CHECK ===" << endl;

        for(auto node : outputs){
            if(!node.second->hasStructure()) {
                cout << "found output without structure\nremoving projections:" << endl;
                for(auto inp : inputs) { cout << inp.first << endl; inp.second->selectAllProps();}
            }
        }
        cout << "\n=== INSTALLING: COMPLETE ===" << endl;


    }
    void reload() {
        cout << "\n=== LOADING: TABLE DATA ===" << endl;
        // Load all tables (only attached nodes will load files)
        try {
            for(auto table : inputs) {
                std::cout << "accessing data for " << table.first << endl;
                table.second->dataInput("", 1); // this signals SourceTables to load files but is ignored by SourceStreams
            }
        } catch(EngineException exc) {
            cout << exc.what() << endl;
        }

    }
    void startServer() {
        reload();
        cout << "\n=== STARTING: LAUNCH SERVER ===" << endl;

        std::string addres ="localhost";
        int port = 8080;
        std::cout << "server listening on: http://" << addres << ":" << port << std::endl;
        server.listen(addres, port);

    }

};
