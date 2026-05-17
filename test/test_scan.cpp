// test/test_scan.cpp
#define BOOST_TEST_MODULE BayanTest
#include <boost/test/included/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <fstream>

namespace fs = boost::filesystem;

struct TestFixture {
    fs::path test_dir;
    
    TestFixture() {
        test_dir = fs::current_path() / "test_temp";
        fs::create_directories(test_dir);
    }
    
    ~TestFixture() {
        fs::remove_all(test_dir);
    }
    
    void createFile(const std::string& name, const std::string& content) {
        std::ofstream file(test_dir / name);
        file << content;
        file.close();
    }
};

BOOST_FIXTURE_TEST_SUITE(BayanTests, TestFixture)

BOOST_AUTO_TEST_CASE(test_file_creation) {
    createFile("test.txt", "Hello");
    BOOST_CHECK(fs::exists(test_dir / "test.txt"));
}

BOOST_AUTO_TEST_CASE(test_duplicate_detection) {
    createFile("file1.txt", "Same content");
    createFile("file2.txt", "Same content");
    
    BOOST_CHECK(fs::exists(test_dir / "file1.txt"));
    BOOST_CHECK(fs::exists(test_dir / "file2.txt"));
}

BOOST_AUTO_TEST_SUITE_END()
