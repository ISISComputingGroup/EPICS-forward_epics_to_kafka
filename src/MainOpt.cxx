#include "MainOpt.h"

#ifdef _MSC_VER
#include "WinSock2.h"
#include "wingetopt.h"
#include <iso646.h>
#else
#include <getopt.h>
#include <unistd.h>
#endif
#include "SchemaRegistry.h"
#include "blobs.h"
#include "git_commit_current.h"
#include "helper.h"
#include "logger.h"
#include <iostream>
#include <rapidjson/filereadstream.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/schema.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

namespace BrightnESS {
namespace ForwardEpicsToKafka {

using std::string;
using std::vector;

MainOpt::MainOpt() {
  hostname.resize(128);
  gethostname(hostname.data(), hostname.size());
  if (hostname.back()) {
    // likely an error
    hostname.back() = 0;
  }
  set_broker("localhost:9092");
}

void MainOpt::set_broker(string broker) {
  brokers.clear();
  auto a = split(broker, ",");
  for (auto &x : a) {
    uri::URI u1;
    u1.require_host_slashes = false;
    u1.parse(x);
    brokers.push_back(u1);
  }
}

std::string MainOpt::brokers_as_comma_list() const {
  std::string s1;
  int i1 = 0;
  for (auto &x : brokers) {
    if (i1) {
      s1 += ",";
    }
    s1 += x.host_port;
    ++i1;
  }
  return s1;
}

int MainOpt::parse_json_file(string config_file) {
  if (config_file.empty()) {
    LOG(3, "given config filename is empty");
    return -1;
  }
  this->config_file = config_file;
  using namespace rapidjson;
  Document schema_;
  try {
    schema_.Parse(blobs::schema_config_global_json,
                  strlen(blobs::schema_config_global_json));
  } catch (...) {
    LOG(3, "schema is not valid!");
    return -2;
  }
  if (schema_.HasParseError()) {
    LOG(3, "could not parse schema");
    return -3;
  }
  SchemaDocument schema(schema_);

  // Parse the JSON configuration and extract parameters.
  // Currently, these parameters take precedence over what is given on the
  // command line.
  FILE *f1 = fopen(config_file.c_str(), "rb");
  if (not f1) {
    LOG(3, "can not open the requested config-file");
    return -4;
  }
  rapidjson::Document *document = parse_document(f1);

  if (document->HasParseError()) {
    LOG(3, "configuration is not well formed");
    return -5;
  }

  SchemaValidator vali(schema);

  if (!document->Accept(vali)) {
    StringBuffer sb1, sb2;
    vali.GetInvalidSchemaPointer().StringifyUriFragment(sb1);
    vali.GetInvalidDocumentPointer().StringifyUriFragment(sb2);
    LOG(3,
        "command message schema validation:  Invalid schema: {}  keyword: {}",
        sb1.GetString(), vali.GetInvalidSchemaKeyword());
    if (std::string("additionalProperties") == vali.GetInvalidSchemaKeyword()) {
      LOG(3, "Sorry, you have probably specified more properties than what is "
             "allowed by the schema.");
    }
    LOG(3, "configuration is not valid");
    return -6;
  }
  vali.Reset();
  find_broker(*document);
  find_broker_config(*document);
  find_conversion_threads(*document);
  find_conversion_worker_queue_size(*document);
  find_main_poll_interval(*document);
  find_brokers_config(*document);
  find_status_uri(*document);

  return 0;
}

rapidjson::Document *MainOpt::parse_document(FILE *f1) {
  fseek(f1, 0, SEEK_END);
  int N1 = ftell(f1);
  fseek(f1, 0, SEEK_SET);
  vector<char> buf1;
  buf1.resize(N1);
  rapidjson::FileReadStream is(f1, buf1.data(), N1);
  json = std::make_shared<rapidjson::Document>();
  auto &d = *json;
  d.ParseStream(is);
  fclose(f1);
  f1 = nullptr;
  return &d;
}

void MainOpt::find_status_uri(rapidjson::Document &document) {
  auto &value = document.FindMember("status-uri")->value;
  if (value.IsString()) {
    uri::URI u1;
    u1.port = 9092;
    u1.parse(v.GetString());
    status_uri = u1;
  }
}

void MainOpt::find_brokers_config(rapidjson::Document &document) {
  auto kafka = document.FindMember("kafka");
  if (kafka != document.MemberEnd()) {
    auto &brokers = kafka->value;
    if (brokers.IsObject()) {
      auto broker = brokers.FindMember("broker");
      if (broker != document.MemberEnd()) {
        auto &broker_properties = broker->value;
        if (broker_properties.IsObject()) {
          for (auto &broker_property : broker_properties.GetObject()) {
            auto const &name = broker_property.name.GetString();
            if (strncmp("___", name, 3)) {
              if (broker_property.value.IsString()) {
                auto const &value = broker_property.value.GetString();
                LOG(6, "kafka broker config {}: {}", name, value);
                broker_opt.conf_strings[name] = value;
              } else if (broker_property.value.IsInt()) {
                auto const &value = broker_property.value.GetInt();
                LOG(6, "kafka broker config {}: {}", name, value);
                broker_opt.conf_ints[name] = value;
              } else {
                LOG(3, "ERROR can not understand option: {}", name);
              }
            }
          }
        }
      }
    }
  }
}

void MainOpt::find_main_poll_interval(rapidjson::Document &document) {
  auto &value = document.FindMember("main-poll-interval")->value;
  if (value.IsInt()) {
    main_poll_interval = value.GetInt();
  }
}

void MainOpt::find_conversion_worker_queue_size(rapidjson::Document &document) {
  auto &value = document.FindMember("conversion-worker-queue-size")->value;
  if (value.IsInt()) {
    conversion_worker_queue_size = value.GetInt();
  }
}

void MainOpt::find_conversion_threads(rapidjson::Document &document) {
  auto &value = document.FindMember("conversion-threads")->value;
  if (value.IsInt()) {
    conversion_threads = value.GetInt();
  }
}

void MainOpt::find_broker_config(rapidjson::Document &document) {
  auto &value = document.FindMember("broker-config")->value;
  if (value.IsString()) {
    broker_config = {value.GetString()};
  }
}

void MainOpt::find_broker(rapidjson::Document &document) {
  auto &value = document.FindMember("broker")->value;
  if (value.IsString()) {
    set_broker(value.GetString());
  }
}

std::pair<int, std::unique_ptr<MainOpt>> parse_opt(int argc, char **argv) {
  std::pair<int, std::unique_ptr<MainOpt>> ret{
      0, std::unique_ptr<MainOpt>(new MainOpt)};
  auto &opt = *ret.second;
  int option_index = 0;
  bool getopt_error = false;
  while (true) {
    int c = getopt_long(argc, argv, "hvQ", LONG_OPTIONS, &option_index);
    if (c == -1)
      break;
    if (c == '?') {
      getopt_error = true;
    }
    switch (c) {
    case 'h':
      opt.help = true;
      break;
    case 'v':
      log_level = (std::min)(9, log_level + 1);
      break;
    case 'Q':
      log_level = (std::max)(0, log_level - 1);
      break;
    default:
      auto lname = LONG_OPTIONS[option_index].name;
      // long option without short equivalent:
      if (std::string("help") == lname) {
        opt.help = true;
      }
      if (std::string("config-file") == lname) {
        if (opt.parse_json_file(optarg) != 0) {
          opt.help = true;
          ret.first = 1;
        }
      }
      if (std::string("log-file") == lname) {
        opt.log_file = optarg;
      }
      if (std::string("broker-config") == lname) {
        uri::URI u1;
        u1.port = 9092;
        u1.parse(optarg);
        opt.broker_config = u1;
      }
      if (std::string("broker") == lname) {
        opt.set_broker(optarg);
      }
      if (std::string("kafka-gelf") == lname) {
        opt.kafka_gelf = optarg;
      }
      if (std::string("graylog-logger-address") == lname) {
        opt.graylog_logger_address = optarg;
      }
      if (std::string("influx-url") == lname) {
        opt.influx_url = optarg;
      }
      if (std::string("forwarder-ix") == lname) {
        opt.forwarder_ix = std::stoi(optarg);
      }
      if (std::string("write-per-message") == lname) {
        opt.write_per_message = std::stoi(optarg);
      }
      if (std::string("teamid") == lname) {
        opt.teamid = strtoul(optarg, nullptr, 0);
      }
      if (std::string("status-uri") == lname) {
        uri::URI u1;
        u1.port = 9092;
        u1.parse(optarg);
        opt.status_uri = u1;
      }
    }
  }
  if (optind < argc) {
    LOG(6, "Left-over commandline options:");
    for (int i1 = optind; i1 < argc; ++i1) {
      LOG(6, "{:2} {}", i1, argv[i1]);
    }
  }
  if (getopt_error) {
    opt.help = true;
    ret.first = 1;
  }
  if (opt.help) {
    fmt::print("forward-epics-to-kafka-0.1.0 {:.7} (ESS, BrightnESS)\n"
               "  Contact: dominik.werder@psi.ch\n\n",
               GIT_COMMIT);
    fmt::print(MAN_PAGE, opt.brokers_as_comma_list());
    ret.first = 1;
  }
  return ret;
}

void MainOpt::init_logger() {
  if (!kafka_gelf.empty()) {
    uri::URI uri(kafka_gelf);
    log_kafka_gelf_start(uri.host, uri.topic);
    LOG(3, "Enabled kafka_gelf: //{}/{}", uri.host, uri.topic);
  }
  if (!graylog_logger_address.empty()) {
    fwd_graylog_logger_enable(graylog_logger_address);
  }
}
}
}