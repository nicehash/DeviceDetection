## shared library

If `trivial_json_printer.hpp` doesn't work for your case you can use https://github.com/nlohmann/json (**tested with https://github.com/nlohmann/json/tree/v3.0.1**). Download `json.hpp` inside this (`shared`) directory and define `NLOHMANN_JSON` to use `json.hpp` instead of `trivial_json_printer.hpp`.



relative paths don't work
so it is copied in every subproject dir

