#include <catch.hpp>

#include <coreutils/file.h>
#include <coreutils/log.h>
#include <coreutils/path.h>

struct CustomType
{
    int x = 2;
    std::string y = "hey";
};

namespace utils {
bool load_data(CustomType& target, std::string const& file_name)
{
    utils::File f{file_name};
    target.x = f.read<int>();
    target.y = f.readString();
    return true;
}

bool save_data(CustomType const& source, std::string const& file_name)
{
    utils::File f{file_name, utils::File::Mode::Write};
    f.write(source.x);
    f.writeString(source.y);
    return true;
}
} // namespace utils

#include <coreutils/resources.h>

TEST_CASE("resources", "[resources]")
{
    using namespace utils;
    Resources r{"", ""};
    std::string result;
    r.load<std::string>("README.md",
                        [&result](std::shared_ptr<std::string> const& text) {
                            LOGI("Loaded");
                            result = *text;
                        });

    REQUIRE(result.length() > 10);


    std::shared_ptr<CustomType> ct_result;
    r.load<CustomType>("custom.dat",
                       [&ct_result](auto ct) {
                           ct->x++;
                           ct_result = ct;
                       },
                       []() { return std::make_shared<CustomType>(); });
    REQUIRE(ct_result->x == 3);
}
