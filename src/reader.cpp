#include <iostream>
#include <fstream>
#include "utils.hpp"
#include "parse_json.hpp"

BrewMap gBrewMap;

// Anonymous class will commit static initialization for gBrewMap before main function.
// Generally, dynamic initialization before main function is compiler-dependent.
#define RegisterBrewFunction(func) \
namespace { \
class __Registerer_##func { \
 public: \
  __Registerer_##func() { \
    gBrewMap[#func] = &func; \
  } \
}; \
__Registerer_##func g_registerer_##func; \
}

DEFINE_string(input, "",
        "json format to read");
DEFINE_string(output, "",
        "json filename to write");

static BrewFunction GetBrewFunction(const std::string& name) {
        if (gBrewMap.count(name)) {
                return gBrewMap[name];
        } else {
                LOG(ERROR) << "Available json reader actions:";
                for (BrewMap::iterator iter = gBrewMap.begin(); iter != gBrewMap.end(); iter++) {
                        LOG(ERROR) << "\t" << iter->first;
                }
                LOG(FATAL) << "Unknow action: " << name;
                return NULL;
        }
}

int reshape() {
    CHECK_GT(FLAGS_input.size(), 0) << "Need input .json file.";
    CHECK_GT(FLAGS_output.size(), 0) << "Need output .json name.";

    std::ofstream ofs;
    ofs.open(FLAGS_output, std::ofstream::out);

    if (ofs.is_open()) {
        json_parser *jParser = json_parser_init(FLAGS_input.c_str());
        if (jParser) {
            LOG(INFO) << "Opened " << FLAGS_input;
            json_object* jObj = json_parser_getJobj(jParser);

            if (jObj) {
                LOG(INFO) << "Parsing completed. Let's make readable JSON.";
                ofs << json_object_to_json_string_ext(jObj, JSON_C_TO_STRING_PRETTY);
                // Release json object
                json_object_put(jObj);
                ofs.close();
                LOG(INFO) << "Finished!";
            }
            json_parser_close(jParser);
        } else {
            LOG(ERROR) << "Fail to initialize json parser: " << FLAGS_input;
            return 1;
        }
    } else {
        LOG(ERROR) << "Fail to open " << FLAGS_output;
    }

    return 0;
}
RegisterBrewFunction(reshape);

int scheme() {
    CHECK_GT(FLAGS_input.size(), 0) << "Need input .json file.";
    json_parser *jParser = json_parser_init(FLAGS_input.c_str());

    if (jParser) {
        LOG(INFO) << "Opened " << FLAGS_input;
        json_object* jObj = json_parser_getJobj(jParser);

        if (jObj) {
            json_retrieve_scheme(jObj);
            // Release json object
            json_object_put(jObj);
        }
        json_parser_close(jParser);
    } else {
        LOG(ERROR) << "Fail to initialize json parser: " << FLAGS_input;
        return 1;
    }
    return 0;
}
RegisterBrewFunction(scheme);

int main(int argc, char** argv) {
    // Print output to stderr (while still logging).
    FLAGS_alsologtostderr = 1;
    // Set version
    gflags::SetVersionString("json_reader.0.0.0");
    // Usage message
    gflags::SetUsageMessage("command line brew\n"
            "usage: json_reader <command> <args>\n\n"
            "commands: \n"
            "  scheme         retrieve 1st element in json"
            "  reshape        change JSON to readable text");
    // Google flags.
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    // Google logging.
    google::InitGoogleLogging(argv[0]);
    // Provide a backtrace on segfault.
    google::InstallFailureSignalHandler();

    if (argc < 2) {
        gflags::ShowUsageWithFlagsRestrict(argv[0], "reader");
        return 1;
    } else {
        return GetBrewFunction(std::string(argv[1]))();
    }
    return 0;
}