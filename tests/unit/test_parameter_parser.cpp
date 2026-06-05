#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <iostream>

#include "utility/parameter_parser.h"


//helper function to convert strings- to char*-vector
std::vector<char*> createArgv(const std::vector<std::string>& argStrings) {
    std::vector<char*> argv;
    for (const auto& arg : argStrings) {
        argv.push_back(const_cast<char*>(arg.c_str()));
    }
    return argv;
}

TEST_CASE("parseParameters default values", "[parameter_parser]") {
    Parameters params;
    std::vector<std::string> testArgs = {"Physarum"};
    std::vector<char*> argv = createArgv(testArgs);

    bool result = parseParameters(argv.size(), argv.data(), params);

    REQUIRE(result == true);
    REQUIRE(params.width == 800);
    REQUIRE(params.height == 800);
    REQUIRE(params.numParticles == 10000);
    REQUIRE_THAT(params.slimeRatio, Catch::Matchers::WithinAbs(0.15f, 1e-5f));
    REQUIRE(params.workGroupDivider == 8);
    REQUIRE(params.audioDevice == "");
    REQUIRE(params.customResolution == false);
    REQUIRE(params.customParticleCount == false);

    SECTION("with all parameters") {
        argv.push_back(const_cast<char*>("--width"));
        argv.push_back(const_cast<char*>("1280"));
        argv.push_back(const_cast<char*>("--height"));
        argv.push_back(const_cast<char*>("720"));
        argv.push_back(const_cast<char*>("--particles"));
        argv.push_back(const_cast<char*>("100000"));
        argv.push_back(const_cast<char*>("--workGroupDivider"));
        argv.push_back(const_cast<char*>("8"));
        argv.push_back(const_cast<char*>("--audioDevice"));
        argv.push_back(const_cast<char*>("default"));

        bool result = parseParameters(argv.size(), argv.data(), params);

        REQUIRE(result == true);
        REQUIRE(params.width == 1280);
        REQUIRE(params.height == 720);
        REQUIRE(params.numParticles == 100000);
        REQUIRE(params.workGroupDivider == 8); // unchanged
        REQUIRE(params.audioDevice == "default");
        REQUIRE(params.customResolution == true);
        REQUIRE(params.customParticleCount == true);
    }
}

TEST_CASE("parseParameters parses --width", "[parameter_parser]") {
    Parameters params;
    std::vector<std::string> testArgs = {"Physarum", "--width"};
    std::vector<char*> argv = createArgv(testArgs);

    SECTION("normal value") {
        argv.push_back(const_cast<char*>("1920"));
        bool result = parseParameters(argv.size(), argv.data(), params);
        REQUIRE(result == true);
        REQUIRE(params.width == 1920);
        REQUIRE(params.height == 800); // unchanged
        REQUIRE(params.customResolution == true);
    }

    SECTION("missing value") {
        bool result = parseParameters(argv.size(), argv.data(), params);
        REQUIRE(result == false); // should return false for missing value

        SECTION("missing value with subsequent args") {
            argv.push_back(const_cast<char*>("--height"));
            argv.push_back(const_cast<char*>("1080"));

            bool result = parseParameters(argv.size(), argv.data(), params);
            REQUIRE(result == false);
        }
    }

    SECTION("zero value") {
        argv.push_back(const_cast<char*>("0"));
        bool result = parseParameters(argv.size(), argv.data(), params);
        REQUIRE(result == false);
    }

    SECTION("negative value") {
        argv.push_back(const_cast<char*>("-800"));
        bool result = parseParameters(argv.size(), argv.data(), params);
        REQUIRE(result == false);
    }

    SECTION("non-integer value") {
        argv.push_back(const_cast<char*>("16.34"));
        bool result = parseParameters(argv.size(), argv.data(), params);
        REQUIRE(result == true); // atoi will parse the integer part and ignore the rest
        REQUIRE(params.width == 16);
    }

    SECTION("non-numeric value") {
        argv.push_back(const_cast<char*>("abc"));
        bool result = parseParameters(argv.size(), argv.data(), params);
        REQUIRE(result == false);
    }
}

TEST_CASE("parseParameters parses --height", "[parameter_parser]") {
    Parameters params;
    std::vector<std::string> testArgs = {"Physarum", "--height", "1080"};
    std::vector<char*> argv = createArgv(testArgs);

    bool result = parseParameters(argv.size(), argv.data(), params);

    REQUIRE(result == true);
    REQUIRE(params.height == 1080);
    REQUIRE(params.customResolution == true);
    REQUIRE(params.width == 800); // unchanged
}

TEST_CASE("parseParameters parses --width and --height together", "[parameter_parser]") {
    Parameters params;
    std::vector<std::string> testArgs = {"Physarum", "--width", "2560", "--height", "1440"};
    std::vector<char*> argv = createArgv(testArgs);

    bool result = parseParameters(argv.size(), argv.data(), params);

    REQUIRE(result == true);
    REQUIRE(params.width == 2560);
    REQUIRE(params.height == 1440);
    REQUIRE(params.customResolution == true);
}

TEST_CASE("parseParameters parses --particles", "[parameter_parser]") {
    Parameters params;
    std::vector<std::string> testArgs = {"Physarum", "--particles", "500000"};
    std::vector<char*> argv = createArgv(testArgs);

    bool result = parseParameters(argv.size(), argv.data(), params);

    REQUIRE(result == true);
    REQUIRE(params.numParticles == 500000);
    REQUIRE(params.customParticleCount == true);
}

TEST_CASE("parseParameters parses --slimeRatio", "[parameter_parser]") {
    Parameters params;
    std::vector<std::string> testArgs = {"Physarum", "--slimeRatio"};
    std::vector<char*> argv = createArgv(testArgs);

    SECTION("typical value") {
        argv.push_back(const_cast<char*>("0.5"));

        bool result = parseParameters(argv.size(), argv.data(), params);

        REQUIRE(result == true);
        REQUIRE_THAT(params.slimeRatio, Catch::Matchers::WithinAbs(0.5f, 1e-5f));
        REQUIRE(params.customParticleCount == false);
    }

    SECTION("minimum value 0.0") {
        argv.push_back(const_cast<char*>("0.0"));

        bool result = parseParameters(argv.size(), argv.data(), params);

        REQUIRE(result == true);
        REQUIRE_THAT(params.slimeRatio, Catch::Matchers::WithinAbs(0.0f, 1e-5f));
    }

    SECTION("negative value") {
        argv.push_back(const_cast<char*>("-0.5"));

        bool result = parseParameters(argv.size(), argv.data(), params);

        REQUIRE(result == false);
    }
}

TEST_CASE("parseParameters parses --audioDevice", "[parameter_parser]") {
    Parameters params;
    std::vector<std::string> testArgs = {"Physarum", "--audioDevice", "hw:0,0"};
    std::vector<char*> argv = createArgv(testArgs);

    bool result = parseParameters(argv.size(), argv.data(), params);

    REQUIRE(result == true);
    REQUIRE_THAT(params.audioDevice, Catch::Matchers::Equals("hw:0,0"));

    SECTION("with subsequent args") {
        argv.push_back(const_cast<char*>("--width"));
        argv.push_back(const_cast<char*>("1280"));

        bool result = parseParameters(argv.size(), argv.data(), params);

        REQUIRE(result == true);
        REQUIRE_THAT(params.audioDevice, Catch::Matchers::Equals("hw:0,0"));
        REQUIRE(params.width == 1280);
        REQUIRE(params.customResolution == true);
    }
}

TEST_CASE("parseParameters returns true for --help", "[parameter_parser]") {
    Parameters params;

    SECTION("--help flag") {
        std::vector<std::string> testArgs = {"Physarum", "--help"};
        std::vector<char*> argv = createArgv(testArgs);
 
        bool result = parseParameters(argv.size(), argv.data(), params);

        REQUIRE(result == true);
    }

    SECTION("-h flag") {
        std::vector<std::string> testArgs = {"Physarum", "-h"};
        std::vector<char*> argv = createArgv(testArgs);

        bool result = parseParameters(argv.size(), argv.data(), params);

        REQUIRE(result == true);
    }
}

TEST_CASE("parseParameters returns false for unknown argument", "[parameter_parser]") {
    Parameters params;
    std::vector<std::string> testArgs = {"Physarum", "--unknown"};
    std::vector<char*> argv = createArgv(testArgs);

    bool result = parseParameters(argv.size(), argv.data(), params);

    REQUIRE(result == false);
}

TEST_CASE("parseParameters --particles does not affect slimeRatio flag", "[parameter_parser]") {
    Parameters params;
    std::vector<std::string> testArgs = {"Physarum", "--particles", "200000"};
    std::vector<char*> argv = createArgv(testArgs);

    bool result = parseParameters(argv.size(), argv.data(), params);
    REQUIRE(result == true);
    REQUIRE(params.customParticleCount == true);
    // slimeRatio remains default, customParticleCount signals override
    REQUIRE_THAT(params.slimeRatio, Catch::Matchers::WithinAbs(0.15f, 1e-5f));
}