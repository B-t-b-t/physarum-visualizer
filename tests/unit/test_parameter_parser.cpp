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
    std::vector<char*> argv = createArgv({"Physarum"});

    int result = parseParameters(argv.size(), argv.data(), params, 8);

    REQUIRE(result == 0);
    REQUIRE(params.width == 1600);
    REQUIRE(params.height == 896);
    REQUIRE(params.numParticles == 300000);
    REQUIRE_THAT(params.slimeRatio, Catch::Matchers::WithinAbs(0.15f, 1e-5f));
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
        argv.push_back(const_cast<char*>("--audioDevice"));
        argv.push_back(const_cast<char*>("default"));

        int result = parseParameters(argv.size(), argv.data(), params, 8);

        REQUIRE(result == 0);
        REQUIRE(params.width == 1280);
        REQUIRE(params.height == 720);
        REQUIRE(params.numParticles == 100000);
        REQUIRE(params.audioDevice == "default");
        REQUIRE(params.customResolution == true);
        REQUIRE(params.customParticleCount == true);
    }
}

TEST_CASE("parseParameters parses --width", "[parameter_parser]") {
    Parameters params;
    std::vector<char*> argv = createArgv({"Physarum", "--width"});

    SECTION("normal value") {
        argv.push_back(const_cast<char*>("1920"));
        int result = parseParameters(argv.size(), argv.data(), params, 8);
        REQUIRE(result == 0);
        REQUIRE(params.width == 1920);
        REQUIRE(params.height == 896); // unchanged
        REQUIRE(params.customResolution == true);
    }

    SECTION("missing value") {
        int result = parseParameters(argv.size(), argv.data(), params, 8);
        REQUIRE(result == -1); // should return -1 for missing value

        SECTION("missing value with subsequent args") {
            argv.push_back(const_cast<char*>("--height"));
            argv.push_back(const_cast<char*>("1080"));

            int result = parseParameters(argv.size(), argv.data(), params, 8);
            REQUIRE(result == -1);
        }
    }

    SECTION("zero value") {
        argv.push_back(const_cast<char*>("0"));
        int result = parseParameters(argv.size(), argv.data(), params, 8);
        REQUIRE(result == -1);
    }

    SECTION("negative value") {
        argv.push_back(const_cast<char*>("-800"));
        int result = parseParameters(argv.size(), argv.data(), params, 8);
        REQUIRE(result == -1);
    }

    SECTION("non-integer value") {
        argv.push_back(const_cast<char*>("12.34"));
        int result = parseParameters(argv.size(), argv.data(), params, 8);
        REQUIRE(result == 0);
        REQUIRE(params.width == 12);
    }

    SECTION("non-numeric value") {
        argv.push_back(const_cast<char*>("abc"));
        int result = parseParameters(argv.size(), argv.data(), params, 8);
        REQUIRE(result == -1);
    }
}

TEST_CASE("parseParameters parses --height", "[parameter_parser]") {
    Parameters params;
    std::vector<char*> argv = createArgv({"Physarum", "--height", "1080"});

    int result = parseParameters(argv.size(), argv.data(), params, 8);

    REQUIRE(result == 0);
    REQUIRE(params.height == 1080);
    REQUIRE(params.customResolution == true);
    REQUIRE(params.width == 1600); // unchanged
}

TEST_CASE("parseParameters parses --width and --height together", "[parameter_parser]") {
    Parameters params;
    std::vector<char*> argv = createArgv({"Physarum", "--width", "2560", "--height", "1440"});

    int result = parseParameters(argv.size(), argv.data(), params, 8);

    REQUIRE(result == 0);
    REQUIRE(params.width == 2560);
    REQUIRE(params.height == 1440);
    REQUIRE(params.customResolution == true);
}

TEST_CASE("parseParameters parses --particles", "[parameter_parser]") {
    Parameters params;
    std::vector<char*> argv = createArgv({"Physarum", "--particles", "500000"});
    
    int result = parseParameters(argv.size(), argv.data(), params, 8);

    REQUIRE(result == 0);
    REQUIRE(params.numParticles == 500000);
    REQUIRE(params.customParticleCount == true);
}

TEST_CASE("parseParameters parses --slimeRatio", "[parameter_parser]") {
    Parameters params;
    std::vector<char*> argv = createArgv({"Physarum", "--slimeRatio"});

    SECTION("typical value") {
        argv.push_back(const_cast<char*>("0.5"));

        int result = parseParameters(argv.size(), argv.data(), params, 8);

        REQUIRE(result == 0);
        REQUIRE_THAT(params.slimeRatio, Catch::Matchers::WithinAbs(0.5f, 1e-5f));
        REQUIRE(params.customParticleCount == false);
    }

    SECTION("minimum value 0.0") {
        argv.push_back(const_cast<char*>("0.0"));

        int result = parseParameters(argv.size(), argv.data(), params, 8);

        REQUIRE_THAT(params.slimeRatio, Catch::Matchers::WithinAbs(0.0f, 1e-5f));
    }

    SECTION("negative value") {
        argv.push_back(const_cast<char*>("-0.5"));

        int result = parseParameters(argv.size(), argv.data(), params, 8);

        REQUIRE(result == -1);
    }
}

TEST_CASE("parseParameters parses --audioDevice", "[parameter_parser]") {
    Parameters params;
    std::vector<char*> argv = createArgv({"Physarum", "--audioDevice", "hw:0,0"});

    int result = parseParameters(argv.size(), argv.data(), params, 8);

    REQUIRE(result == 0);
    REQUIRE_THAT(params.audioDevice, Catch::Matchers::Equals("hw:0,0"));

    SECTION("with subsequent args") {
        argv.push_back(const_cast<char*>("--width"));
        argv.push_back(const_cast<char*>("1280"));

        int result = parseParameters(argv.size(), argv.data(), params, 8);

        REQUIRE(result == 0);
        REQUIRE_THAT(params.audioDevice, Catch::Matchers::Equals("hw:0,0"));
        REQUIRE(params.width == 1280);
        REQUIRE(params.customResolution == true);
    }
}

TEST_CASE("parseParameters returns -1 for --help", "[parameter_parser]") {
    Parameters params;

    SECTION("--help flag") {
        std::vector<char*> argv = createArgv({"Physarum", "--help"});
 
        int result = parseParameters(argv.size(), argv.data(), params, 8);

        REQUIRE(result == -1);
    }

    SECTION("-h flag") {
        std::vector<char*> argv = createArgv({"Physarum", "-h"});

        int result = parseParameters(argv.size(), argv.data(), params, 8);

        REQUIRE(result == -1);
    }
}

TEST_CASE("parseParameters returns -1 for unknown argument", "[parameter_parser]") {
    Parameters params;
    std::vector<char*> argv = createArgv({"Physarum", "--unknown"});

    int result = parseParameters(argv.size(), argv.data(), params, 8);

    REQUIRE(result == -1);
}

TEST_CASE("parseParameters --particles does not affect slimeRatio flag", "[parameter_parser]") {
    Parameters params;
    std::vector<char*> argv = createArgv({"Physarum", "--particles", "200000"});

    parseParameters(argv.size(), argv.data(), params, 8);
    REQUIRE(params.customParticleCount == true);
    // slimeRatio remains default, customParticleCount signals override
    REQUIRE_THAT(params.slimeRatio, Catch::Matchers::WithinAbs(0.15f, 1e-5f));
}