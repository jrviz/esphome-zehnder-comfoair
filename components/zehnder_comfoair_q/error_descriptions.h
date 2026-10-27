#pragma once

// Shared shape every translation in this folder implements. See error_descriptions_en.h for the
// reference language and instructions on adding another one.
//
// Deliberately plain constant data (no std::map etc.): it is constant-initialised, so it lives in
// flash and costs no RAM or boot-time work.

#include <cstddef>
#include <cstdint>

namespace zehnder_comfoair_q
{
    namespace error_descriptions
    {
        struct Entry
        {
            uint16_t id;
            const char *text;
        };

        struct Table
        {
            const char *no_active_errors;
            const char *unknown_error;
            const Entry *entries;
            size_t count;
        };
    }
}
