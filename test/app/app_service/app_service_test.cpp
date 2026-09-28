#include <iostream>
#include <jsoncons/json.hpp>
#include <string>

using namespace jsoncons;

int main(int argc, char* argv[])
{
  std::string data = R"(
    {
       "an_int": 34,
       "a_float": -24.8,
       "a_bool": "true",
       "not_an_array": "",
       "another_object": {
          "something1": "value1",
          "something2": "value2"
       },
       "an_array": [ "1", "2", "3" ]
    }
  )";
  json j = json::parse(data);

  std::cout << std::boolalpha;
  std::cout << "Is j an array:" << j.is_array() << std::endl;
  std::cout << "Is j[not_an_array] an array:" << j["not_an_array"].is_array() << std::endl;
  std::cout << "Is j[an_array] an array:" << j["an_array"].is_array() << std::endl;
  std::cout << "Does j[an_array] contain 1: " << j["an_array"].contains("1") << std::endl;

  std::string another_object = j["another_object"].as_string();

  std::cout << "another_object: " << another_object << std::endl;

  bool bool_parse_result;
  if (j["a_bool"].is_bool()) {
    bool_parse_result = j["a_bool"].as_bool();
    std::cout << "bool_parse_result: " << bool_parse_result << std::endl;
  } else {
    std::cout << "bool_parse_result is not a bool" << std::endl;
  }

  std::string a_bool_as_string = j["a_bool"].as_string();
  std::cout << "a_bool_as_string: " << a_bool_as_string << std::endl;

  std::string an_int_as_string = j["an_int"].as_string();
  std::cout << "an_int_as_string: " << an_int_as_string << std::endl;

  std::string a_float_as_string = j["a_float"].as_string();
  std::cout << "a_float_as_string: " << a_float_as_string << std::endl;

  std::string an_array_as_string = j["an_array"].as_string();
  std::cout << "an_array_as_string: " << an_array_as_string << std::endl;
  
  std::cout << std::endl;
  for(const auto& member: j.object_range()){
    std::cout << member.key() << ": " << member.value() << std::endl;
  }

  return 0;
}