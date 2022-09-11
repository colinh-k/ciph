#include <iostream>
#include "clap.hpp"

extern "C" {
    #include "../include/crypt.h"
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

    parser.addArg({"operation"}, "specify the type of operation to perform {encipher or decipher}", clap::Type<std::string>({"encipher", "decipher"}));
    parser.addArg({"input-file", "-i"}, "the input filename (must exist)", clap::Type<std::string>());
    parser.addArg({"output-file", "-o"}, "the output filename (overwritten if already exists)", clap::Type<std::string>());
    parser.addArg({"--key-file", "-k"}, "the key file (must exist)", clap::Type<std::string>());
    parser.addArg({"--range", "-r"}, "range for operation {first-byte last-byte}", clap::Type<std::vector<std::size_t>>(), 2);

    // std::size_t argc = 11;
    // char const *argv[] = {
    //     "./utility", "decipher",
    //     "-i", "./src/cipher.txt",
    //     "-o", "./src/decipher.txt",
    //     "-k", "./src/key.txt",
    //     "-r", "1", "59"
    // };

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
    std::string fnameIn = map.get<std::string>("input-file");
    std::string fnameOut = map.get<std::string>("output-file");

    // handle key file
    std::string fnameKey;
    if (map.hasValue("key-file")) {
        fnameKey = map.get<std::string>("key-file");
        if (access(fnameKey.c_str(), F_OK) != 0) {
            // keyfile doesnt exist
            Crypt_GenerateKeyFile(fnameKey.c_str(), KEY_LEN);
        }
    } else {
        // no key file specified, so generate a new one using the name of the input file + .ciphkey
        // TODO: what happens when input doesnt contain a '.' extension ?
        fnameKey = fnameIn.substr(0, fnameIn.find_last_of('.') + 1) + ".ciphkey";
        Crypt_GenerateKeyFile(fnameKey.c_str(), KEY_LEN);
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
        std::cout << fnameIn.c_str() << '\n' << fnameKey.c_str() << '\n' << fnameOut.c_str() << '\n' << rangeStart << '\n' << rangeEnd << '\n';
        std::cout << CRYPT_CALC_ENDPT(rangeStart, rangeEnd) << '\n';
        Crypt_DecipherRange(fnameIn.c_str(), fnameKey.c_str(), fnameOut.c_str(), rangeStart, CRYPT_CALC_ENDPT(rangeStart, rangeEnd));
    }

    return EXIT_SUCCESS;
}
