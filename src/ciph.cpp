#include <iostream>
#include "clap.hpp"

extern "C" {
    #include "../include/ciph.h"
}

// for windows compatability for access()
#ifdef WIN32
#include <io.h>
#define F_OK 0
#define access _access
#else
#include <unistd.h>  // for access()
#endif

#define KEY_LEN 16

int main(int argc, char const *argv[]) {
    clap::ArgumentParser parser;

    parser.addArg({"operation"}, "specify the type of operation to perform {encipher, decipher, or keygen}", clap::Type<std::string>({"encipher", "decipher", "keygen"}));
    parser.addArg({"--input-file", "-i"}, "the input filename required for enciphering or deciphering (must exist)", clap::Type<std::string>());
    parser.addArg({"--output-file", "-o"}, "the output filename required for enciphering or deciphering (overwritten if already exists)", clap::Type<std::string>());
    parser.addArg({"--key-file", "-k"}, "the key filename", clap::Type<std::string>());
    parser.addArg({"--key-size", "-s"}, "the key size in bits (must be compliant with AES) {128, 192, 256}", clap::Type<std::size_t>({128, 192, 256}));
    parser.addArg({"--range", "-r"}, "range for operation {first-byte last-byte}", clap::Type<std::vector<std::size_t>>(), 2);

    clap::ArgumentMap map;
    try {
        map = parser.parse(argc, argv);
    } catch (clap::HelpException& e) {
        // help flag was supplied
        std::cout << parser.help();
        return EXIT_SUCCESS;
    } catch (clap::ClapException& e) {
        // parsing failed
        std::cerr << e.what() << '\n';
        std::cerr << parser.getUsage() << '\n';
        return EXIT_FAILURE;
    }

    std::string op = map.get<std::string>("operation");

    if (op == "keygen") {
        if (map.hasValue("key-file") && map.hasValue("key-size")) {
            // key-size / 8 because we want to convert from bits to bytes
            Crypt_GenerateKeyFile((map.get<std::string>("key-file") + ".ciphkey").c_str(), map.get<std::size_t>("key-size") / 8);
            return EXIT_SUCCESS;
        } else {
            std::cerr << clap::ParseException("keygen operation requires key-file and key-size.").what() << '\n';
            std::cerr << parser.getUsage() << '\n';
            return EXIT_FAILURE;
        }
    }

    if (!map.hasValue("input-file") || !map.hasValue("output-file") || !map.hasValue("key-file")) {
        std::cerr << clap::ParseException("encipher and decipher operations require input-file, output-file, and key-file.").what() << '\n';
        std::cerr << parser.getUsage() << '\n';
        return EXIT_FAILURE;
    }

    std::string fnameIn = map.get<std::string>("input-file");
    std::string fnameOut = map.get<std::string>("output-file");

    // handle key file
    std::string fnameKey = map.get<std::string>("key-file");
    if (access(fnameKey.c_str(), F_OK) != 0) {
        std::cerr << clap::ParseException("key-file does not exist or is inaccessible.").what() << '\n';
        std::cerr << parser.getUsage() << '\n';
        return EXIT_FAILURE;
    }

    // handle range
    std::size_t rangeStart = 0, rangeEnd = CRYPT_EOF;
    if (map.hasValue("range")) {
        std::vector<std::size_t> range = map.get<std::vector<std::size_t>>("range");
        rangeStart = range[0];
        rangeEnd = range[1];
    }

    // handle op mode
    if (op == "encipher") {
        Crypt_EncipherRange(fnameIn.c_str(), fnameKey.c_str(), fnameOut.c_str(), rangeStart, rangeEnd);
    } else {
        if (rangeEnd != CRYPT_EOF) {
            // special case; if rangeEnd == CRYPT_EOF, CRYPT_CALC_ENDPT does not correctly calculate
            // the endpoint, so just pass CRYPT_EOF if this is the case
            rangeEnd = CRYPT_CALC_ENDPT(rangeStart, rangeEnd);
        }
        Crypt_DecipherRange(fnameIn.c_str(), fnameKey.c_str(), fnameOut.c_str(), rangeStart, rangeEnd);
    }

    return EXIT_SUCCESS;
}
