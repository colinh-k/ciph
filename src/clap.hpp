#pragma once

#include <iostream>
#include <map>
#include <vector>
#include <set>
#include <string>
#include <sstream>  // for stringstream
#include <algorithm>
#include <iterator>
#include <cstdlib>
#include <memory>
#include <stdexcept>  // for standard exception classes

namespace clap {
    bool isShortFlag(const std::string& name) {
        // short flags have the form '-c', where c can be any character
        // NOTE: this check DOES recognize the '--' flag which may have a special meaning
        return name.size() == 2 && name.at(0) == '-';
    }

    bool isLongFlag(const std::string& name) {
        // long flags have the form '--name', where name is any sequence of
        // at least one character.
        // TODO: this definiton does not include the string '--', which might have a special meaning
        return name.size() >= 3 && name.rfind("--", 0) == 0;
        // return name.size() >= 3 && name.substr(0, 2) == "--";
    }

    bool isFlag(const std::string& name) {
        // is either long or short flag
        return isShortFlag(name) || isLongFlag(name);
    }

    // exception classes:
    class ClapException : public std::runtime_error {
        // base exception class
        public:
            ClapException(const std::string& message) :
                std::runtime_error("error: " + message) {}
    };

    class ParseException : public ClapException {
        // thrown when argument(s) are supplied in the
        // wrong order, or omitted, or of the wrong type
        public:
            ParseException(const std::string& message) :
                ClapException(message) {}
    };

    class TypeException : public ClapException {
        // thrown when an argument cannot be interpreted
        // as the required type, or the user
        public:
            TypeException(const std::string& message) :
                ClapException(message) {}
    };

    class InvalidNameException : public ClapException {
        // thrown when an invalid name is given, either
        // to ArgumentParser::addArg, or ArgumentMap::operator[]
        public:
            InvalidNameException(const std::string& message) :
                ClapException(message) {}
    };

    class HelpException : public ClapException {
        // thrown when the help flag is passed in the
        // argument list (-h, --help)
        public:
            HelpException(const std::string& message) :
                ClapException(message) {}
    };

    template <typename T>
    void parseValue(const std::string& str, T& value) {
        std::stringstream stream;
        stream << str;
        stream >> value;
        char c;
        if (stream.fail() || stream.get(c)) {
            // not an integer
            throw TypeException("cannot parse '" + str + "' as the requested type");
        }
    }

    template <typename T>
    void parseValue(const std::string& str, std::vector<T>& value) {
        T t;
        std::stringstream stream;
        stream << str;
        stream >> t;
        char c;
        if (stream.fail() || stream.get(c)) {
            // not an integer
            throw TypeException("cannot parse '" + str + "' as the requested type");
        }
        value.push_back(t);
    }

    class ValueTypeBase {
        public:
            ValueTypeBase() = default;
            virtual ~ValueTypeBase() = default;
            virtual void fromString(const std::string& str) = 0;
            virtual bool hasValue() const = 0;
            // public std::enable_shared_from_this<ValueTypeBase>
            // virtual std::shared_ptr<ValueTypeBase> defaultValue() = 0;
            // virtual std::shared_ptr<ValueTypeBase> oneOf() = 0;
    }; // class Type

    template <typename T>
    class ValueType : public ValueTypeBase {
        public:
            ValueType() : result(std::make_shared<T>()), _hasValue(false) {}
            ValueType(T& _defaultResult) :
                result(std::make_shared<T>()),
                // NOTE: invokes copy constructor of T
                defaultResult(std::make_shared<T>(_defaultResult)),
                _hasValue(false) {}

            ValueType(std::set<T>& _valueSet) :
                result(std::make_shared<T>()),
                // NOTE: invokes copy constructor of std::set
                valueSet(std::make_shared<std::set<T>>(_valueSet)),
                _hasValue(false) {}

            ~ValueType() override = default;
            virtual void fromString(const std::string& str) override {
                parseValue(str, *result);
                if (valueSet != nullptr && !(*valueSet).count(*result)) {
                    // check if the parsed value is part of the expected set
                    throw TypeException("parsed value '" + str + "' not part of the expected set");
                }
                _hasValue = true;
            }
            const T& get() const {
                if (_hasValue) {
                    return *result;
                } else if (defaultResult != nullptr) {
                    return *defaultResult;
                }
                throw ClapException("no value to return");
            }
            bool hasValue() const override { return _hasValue; }
            // bool hasDefault() const override { return defaultResult == nullptr; }

        private:
            // TODO: change this variable name
            std::shared_ptr<T> result{};
            std::shared_ptr<T> defaultResult{};
            // the set to which the actul argument must belong. i.e., the
            // actual argument must be a member of this set
            std::shared_ptr<std::set<T>> valueSet{};
            // true if a value has been successfully parsed via fromString,
            // false otherwise; used to indicate if the default
            // value should be used. distinguishes between the
            // default-ctor value of result and an actual / legitimate parsed value
            bool _hasValue;
    }; // class Type

    template <typename T>
    std::shared_ptr<ValueTypeBase> Type() {
        return std::static_pointer_cast<ValueTypeBase>(std::make_shared<ValueType<T>>());
    }

    template <typename T>
    std::shared_ptr<ValueTypeBase> Type(T defaultValue) {
        return std::static_pointer_cast<ValueTypeBase>(std::make_shared<ValueType<T>>(defaultValue));
    }

    template <
        typename T,
        // restrict the types that can have expected value sets to only integral types
        // and strings
        typename std::enable_if<
            std::is_integral<T>::value ||
            std::is_same<T, std::string>::value ||
            std::is_same<T, bool>::value ||
            std::is_same<T, double>::value ||
            std::is_same<T, float>::value
        >::type* = nullptr
    >
    std::shared_ptr<ValueTypeBase> Type(std::set<T> valueSet) {
        return std::static_pointer_cast<ValueTypeBase>(std::make_shared<ValueType<T>>(valueSet));
    }

    // TODO: add another Type() function that accepts a value set *and* a default value

    // TODO: should this be a struct or class ? it holds
    // information about an argument (to be stored in a map)
    // template <typename T>
    class ArgInfo {
        public:
            ArgInfo(const std::string& _longName,
                    const std::string& _shortName,
                    const std::string& _description,
                    const std::shared_ptr<ValueTypeBase>& valueTypePtr,
                    const std::size_t& _nargs) :
                    longName(_longName),
                    shortName(_shortName),
                    description(_description),
                    value(valueTypePtr),
                    nargs(_nargs) {}
            ArgInfo() = default;  // required for std::map to default construct elements

            void addValue(const std::string& _value) {
                value->fromString(_value);
            }

            bool isRequired() const {
                return !isLongFlag(longName);
            }

            bool hasValue() const { return value->hasValue(); }

            std::string longName;
            std::string shortName;
            std::string description;
            std::shared_ptr<ValueTypeBase> value;
            std::size_t nargs;
            // whether a value was added to this argument or not (for
            // testing if this argument was given in the actual arg list)
            // bool hasValue;
    };

    struct ArgNames {
        // shortname will be empty if no shortname was given
        ArgNames(const std::string& _longName,
                const std::string& _shortName=std::string()) :
                longName(_longName), shortName(_shortName) {};
        std::string longName, shortName;
    };

    class ArgumentMap {
        public:
            ArgumentMap() = default;
            ArgumentMap(std::map<std::string, std::shared_ptr<ArgInfo>>&& _nameToArg, std::string&& _usage) :
                nameToArg(_nameToArg), usage(_usage) {}

            template <typename T>
            const T& get(const std::string& name) {
                // lookup by either long name or short name, or throw if
                // the name does not exist
                try {
                    // std::cerr << "1oof\n";
                    // std::shared_ptr<ArgInfo> a = getArg(name);
                    // std::cerr << "2oof\n";
                    // std::shared_ptr<ValueTypeBase> v = a->value;
                    // std::cerr << "3oof\n";
                    // ValueTypeBase& b = *v;
                    // std::cerr << "4oof\n";
                    // ValueType<T>& t = dynamic_cast<ValueType<T>&>(b);
                    // std::cerr << "5oof\n";
                    // T e = t.get();
                    // std::cerr << "6oof\n";
                    // return e;
                    return dynamic_cast<ValueType<T>&>(*(getArg(name)->value)).get();
                } catch (const std::bad_cast& e) {
                    throw TypeException("type parameter used to access value of '" + name + "' does not match declared type");
                }
            }

            bool hasValue(const std::string& name) {
                // throws if the name does not exist
                return getArg(name)->hasValue();
            }

        private:
            std::map<std::string, std::shared_ptr<ArgInfo>> nameToArg;
            std::string usage;

            std::shared_ptr<ArgInfo>& getArg(const std::string& name) {
                if (nameToArg.count(name)) {
                    return nameToArg[name];
                } else if (!isShortFlag(name) && name[0] != '-' && nameToArg.count("-" + name)) {
                    // if it was a short flag, it would have been returned above
                    return nameToArg["-" + name];
                } else if (!isLongFlag(name) && nameToArg.count("--" + name)) {
                    return nameToArg["--" + name];
                }
                throw InvalidNameException("argument name '" + name + "' does not exist");
            }
    }; // class ArgumentMap

    class ArgumentParser {
        public:
            ArgumentParser(const std::string& _description=std::string()) :
                description(_description) {
                // add the first reserved argument
                addArg({"--help", "-h"}, "display this help message", 0);
            }

            void addArg(const ArgNames& names,
                        const std::string& description,
                        std::shared_ptr<ValueTypeBase> valueTypePtr,
                        const std::size_t& nargs=1) {
                // check for valid parameters
                if (names.longName.empty()) {
                    throw InvalidNameException("a long name is required for every argument");
                } else if (!isLongFlag(names.longName) && !(names.shortName.empty())) {
                    // throw InvalidNameException("a positional argument may not have a short name");
                } else if (!isShortFlag(names.shortName) && !names.shortName.empty()) {
                    throw InvalidNameException("invalid short name '" + names.shortName + "'. must have the form '-c', where 'c' can be any single character");
                } else if (isShortFlag(names.longName)) {
                    throw InvalidNameException("long name '" + names.longName + "' may not be a short flag");
                } else if (nameToArg.count(names.longName)) {
                    throw InvalidNameException("long name '" + names.longName + "' already exists");
                } else if (nameToArg.count(names.shortName)) {
                    throw InvalidNameException("short name '" + names.shortName + "' already exists");
                }

                // TODO: consider moving the isRequired logic from ArgInfo ctor
                // to here, so we can avoid storing argInfo.long/shortName.
                // remember to also strip the '-/--' prefixes if we move the logic.
                // NOTE: we can pass an isRequired param to the ArgInfo ctor instead
                std::shared_ptr<ArgInfo> argInfo = std::make_shared<ArgInfo>(
                    names.longName, names.shortName, description, valueTypePtr, nargs
                );
                // TODO: consider storing the names with their prefixes ('-/--') in
                // the map. this will prevent long names from being used with positional
                // arguments ?
                nameToArg[argInfo->longName] = argInfo;
                if (!argInfo->shortName.empty()) {
                    // do not add the empty string to the map
                    nameToArg[argInfo->shortName] = argInfo;
                }
                // store a pointer to the argument info in order
                // TODO: only do this if the name is supposed to be positional,
                // i.e., only when no flag name is specified ?
                if (argInfo->isRequired()) {
                    positionalArgs.push_back(argInfo);
                } else {
                    optionalArgs.push_back(argInfo);
                }
            }

            ArgumentMap parse(int argc, char const **argv);
            std::string help();
            inline std::string getUsage() const { return usage; }

        private:
            // store names without dash prefixes as keys: e.g. f, filename
            std::map<std::string, std::shared_ptr<ArgInfo>> nameToArg;
            // store pointers to the argument info type in the order added
            std::vector<std::shared_ptr<ArgInfo>> positionalArgs;
            // pointers to optional / flag / non-required arguments
            std::vector<std::shared_ptr<ArgInfo>> optionalArgs;  // TODO: could this be a std::set ?
            // usage strings:
            std::string fname;
            std::string usage;
            // a description of the program to be displayed with help
            std::string description;

            void makeUsage() {
                usage = "Usage: " + fname;
                for (const auto& optionalArg : optionalArgs) {
                    usage += " " + formatUsage(optionalArg);
                }
                for (const auto& positionalArg : positionalArgs) {
                    usage += " " + formatUsage(positionalArg);
                }
            }

            // helper function to format each ArgInfo into a usage string
            const std::string formatUsage(const std::shared_ptr<ArgInfo>& argInfo) {
                // choose the name to be repeated in the usage format, like: fname fname fname.
                // NOTE: choose the long name (which is guaranteed to exist)
                // making sure to remove the dash prefix if it exists
                const std::string& repeatName = (!argInfo->isRequired()) ?
                                                argInfo->longName.substr(2) :
                                                argInfo->longName;

                std::string usage = std::string();
                for (std::size_t i = 0; i < argInfo->nargs; i++) {
                    usage += repeatName + ' ';
                }
                // remove trailing space
                usage.pop_back();

                if (!argInfo->isRequired()) {
                    // optional argument
                    const std::string& argName = (argInfo->shortName.empty()) ?
                                                  argInfo->longName :
                                                  argInfo->shortName;
                    usage = '[' + argName + ((usage.empty()) ? "" : ' ' + usage) + ']';
                }
                return usage;
            }
    };  // class ArgumentParser

    ArgumentMap ArgumentParser::parse(int argc, char const **argv) {
        if (!argc) {
            // TODO: should we accept 0 arguments when attempting to parse ?.
            // below, we assume at least one argument (the filename)
            // throw ParseException("No arguments availible to parse");
            // usage("<program name>");
            throw InvalidNameException("no arguments available to parse");
        }

        // TODO: expect the first argument to be the filename;
        fname = argv[0];
        argv = &argv[1];
        argc--;

        // create the usage string once
        makeUsage();

        std::size_t posi = 0;  // index into positional arg list
        std::size_t argi = 0;  // index into actual argument list
        while (argi < argc) {
            const std::string arg{argv[argi]};

            if (arg == "-h" || arg == "--help") {
                // special help case
                // help();
                throw HelpException("received help flag");
            }

            std::shared_ptr<ArgInfo> argInfo = nullptr;
            if (isFlag(arg)) {
                if (nameToArg.count(arg)) {
                    argInfo = nameToArg[arg];
                    argi++;  // consume this argument as a flag name
                } else {
                    // invalid flag
                    throw ParseException("unrecognized flag '" + arg + "'");
                }
            } else {
                // non flag argument, so intepret it as the next positional argument
                if (posi < positionalArgs.size()) {
                    argInfo = positionalArgs[posi];
                    // update the current positional argument index
                    posi++;
                    // do not consume the current argument (by updating argi) because
                    // we need to add it to the argument list below since it
                    // is a positional argument
                } else {
                    // if the index is out of bounds, report an error
                    throw ParseException("unexpected positional argument '" + arg + "'");
                }
            }

            for (std::size_t j = 0; j < argInfo->nargs; j++) {
                if (argi >= argc) {
                    // index out of range for actual argument list
                    throw ParseException("expected " + std::to_string(argInfo->nargs) +
                                         " argument(s) for '" + argInfo->longName + "'");
                }

                const std::string argj{argv[argi]};

                if (argj == "-h" || argj == "--help") {
                    // special help case
                    // help();
                    throw HelpException("received help flag");
                }
                if (isFlag(argj)) {
                    throw ParseException("unexpected flag '" + argj +
                                         "' when parsing argument '" +
                                         argInfo->longName + "'");
                }
                // consume the next nargs arguments and store them
                // in the argInfo struct
                // increment
                argInfo->addValue(argj);
                argi++;
            }
        }

        // check if all the required arguments recieved a value
        for (auto& pair : nameToArg) {
            if (pair.second->isRequired() && !(pair.second->hasValue())) {
                throw ParseException("'" + pair.second->longName + "' is required");
            }
        }

        return ArgumentMap(std::move(nameToArg), std::move(usage));
    }

    std::string ArgumentParser::help() {
        std::stringstream helpMsg;
        helpMsg << usage << "\n";

        if (!description.empty()) {
            helpMsg << "Description:\n\t" << description << "\n\n";
        }

        helpMsg << "Positional arguments:\n";
        for (const auto& positionalArg : positionalArgs) {
            helpMsg << "\t" << positionalArg->longName
                        << "\t" << positionalArg->description << "\n";
        }

        helpMsg << "\nOptional arguments:\n";
        for (const auto& optionalArg : optionalArgs) {
            const std::string& names = (optionalArg->shortName.empty()) ?
                                        optionalArg->longName :
                                        optionalArg->shortName + ", " +
                                        optionalArg->longName;
            helpMsg << "\t" << names
                        << "\t" << optionalArg->description << "\n";
        }

        return helpMsg.str();
    }
};  // namespace clap