#include <catch.hpp>

#include <coreutils/file.h>
#include <coreutils/path.h>

using namespace utils;

TEST_CASE("Create and read file", "[file]")
{
    File{"dummy", File::Write}.write<uint32_t>(0x12345678);
    REQUIRE(File{"dummy"}.read<uint32_t>() == 0x12345678);
    utils::remove("dummy");
}

TEST_CASE("FILE conversion", "[file]")
{
    File f{"dummy", File::Write};
    fwrite("testing", 1, 8, f); // Include 0-terminator
    f.writeString("write");     // Does not include 0-terminator
    f.close();

    FILE* fp = fopen("dummy", "rb");
    File f2{fp};
    auto s = f2.readString();
    REQUIRE(s == "testing");
    char buffer[16];
    REQUIRE(fread(buffer, 1, sizeof(buffer), fp) == 5);
    REQUIRE(strcmp(buffer, "write") == 0);
    f2.close();

    utils::remove("dummy");
}

/* TEST_CASE("List files", "[file]") { */
/* 	std::error_code ec; */
/* 	utils::create_directories("path1/path2/path3", ec); */
/* 	File{"path1/dummy1", File::Write}.write<uint8_t>(1); */
/* 	File{"path1/path2/dummy2", File::Write}.write<uint8_t>(2); */
/* 	File{"path1/path2/dummy3", File::Write}.write<uint8_t>(4); */
/* 	File{"path1/path2/path3/dummy4", File::Write}.write<uint8_t>(8); */

/* 	int total = 0; */
/* 	listRecursive("path1", */
/* 	              [&total](auto const &name) { total +=
 * File{name}.read<uint8_t>(); }); */
/* 	REQUIRE(total == 15); */

/* 	fs::remove_all("path1"); */
/* } */

TEST_CASE("Read lines", "[file]")
{
    File f{"lines.txt", File::Write};
    for (int i = 0; i < 100; i++)
        f.writeln(std::to_string(i));
    f.close();

    int i = 0;
    for (auto const& line : File{"lines.txt"}.lines()) {
        REQUIRE(i++ == std::stol(line));
    }

    utils::remove("lines.txt");
}
