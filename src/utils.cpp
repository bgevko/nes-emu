#include "utils.h"
#include <cstddef>
#include <exception>
#include <cstdint>
#include <cctype>
#include <fstream>
#include <iostream>
#include <regex>
#include <stdexcept>
#include <string>
#include <vector>

/*
################################
||                            ||
||      Regex log parsers     ||
||                            ||
################################
* Helper function that takes a regex match
* and returns a CPU state struct, based on the match
* Used on CPU output logs from other emulators. The logs are in different
* formats, but the general information we want from them is the same
*
*/

namespace utils
{
MatchResult parseLogLine( const std::string &line, const std::regex &pattern, std::size_t expectedMatches )
{
    std::smatch match;
    MatchResult fields;

    if ( std::regex_match( line, match, pattern ) ) {
        if ( match.size() < expectedMatches ) {
            throw std::runtime_error( "Not enough groups found in the line." );
        }
        // Skip match[0], which is the entire match.
        for ( std::size_t i = 1; i < match.size(); ++i ) {
            fields.push_back( match[i].str() );
        }
    } else {
        string const red = "\033[31m";
        string const reset = "\033[0m";
        cerr << '\n';
        std::cerr << red << "Regex did not match line:\n" << line << reset << '\n';
        cerr << '\n';

        throw std::runtime_error( "parseLogLine:Error: Regex did not match line." );
    }
    return fields;
}

MatchResults parseLog( const std::string &filename, const std::regex &pattern, std::size_t expectedMatches )
{
    MatchResults matches;

    std::ifstream log( filename );
    if ( !log.is_open() ) {
        throw runtime_error( "utils::parseLog:Error opening file: " + filename );
    }

    std::string line;
    size_t      lineNum = 0;
    while ( std::getline( log, line ) ) {
        try {
            matches.push_back( parseLogLine( line, pattern, expectedMatches ) );
        } catch ( const exception &e ) {
            throw runtime_error( "utils::parseLog:Error parsing line " + to_string( lineNum ) + ": " +
                                 e.what() );
        }
        ++lineNum;
    }

    return matches;
}

struct InstrV5LogInfo {
    uint16_t pc;
    uint8_t  a;
    uint8_t  x;
    uint8_t  y;
    uint8_t  p;
    uint8_t  sp;
    uint16_t ppuCycles;
    uint16_t scanline;
    uint64_t cpuCycles;
};

static std::vector<InstrV5LogInfo> parseV5Log( const std::string &filename, const std::regex &pattern )
{
    std::vector<InstrV5LogInfo> logInfo;

    MatchResults const matches = parseLog( filename, pattern, 9 );
    for ( const auto &match : matches ) {
        InstrV5LogInfo entry{};
        entry.pc = static_cast<uint16_t>( stoul( match[0], nullptr, 16 ) );
        entry.a = static_cast<uint8_t>( stoul( match[1], nullptr, 16 ) );
        entry.x = static_cast<uint8_t>( stoul( match[2], nullptr, 16 ) );
        entry.y = static_cast<uint8_t>( stoul( match[3], nullptr, 16 ) );
        entry.sp = static_cast<uint8_t>( stoul( match[4], nullptr, 16 ) );
        entry.scanline = static_cast<uint16_t>( stoul( match[6], nullptr, 10 ) );
        entry.ppuCycles = static_cast<uint16_t>( stoul( match[7], nullptr, 10 ) );
        entry.cpuCycles = stoull( match[8], nullptr, 10 );

        // Status
        // Mesen status come in form of string: nv--dIZC
        // We'll convert it to a byte
        const std::string &statusStr = match[5];
        uint8_t            status = 0;
        for ( int i = 0; i < 8; ++i ) {
            // ignore '-', 'I' and lower case
            if ( statusStr[i] != '-' && ( std::isupper( statusStr[i] ) == 0 ) && statusStr[i] != 'I' ) {
                status |= 1 << ( 7 - i );
            }
        }
        entry.p = status;
        logInfo.push_back( entry );
    }
    return logInfo;
}
} // namespace utils
