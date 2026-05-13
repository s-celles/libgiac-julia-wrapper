// probe_gen_layout.cpp
//
// TEMPORARY DIAGNOSTIC — investigating Giac.jl#22 (MPFR reals reported
// as _DOUBLE_ on Windows). Goal: measure giac::gen struct layout on
// Windows (MSYS2 GCC) and verify whether -DGIAC_TYPE_ON_8BITS changes
// the layout in a way that could explain the bug.
//
// Probe is invoked from the libgiac-julia-wrapper CI workflow, compiled
// twice (default + -DGIAC_TYPE_ON_8BITS) against the installed GIAC_jll
// headers. Delete this file (and the corresponding CI step) once the
// bug is diagnosed and fixed.
//
// On Linux x86_64 (GCC 12.2) the two modes produce identical layouts;
// any Windows-specific divergence (e.g. MS-bitfield packing differences
// between GCC 8 and GCC 15) will show up here.

#include <config.h>
#include <giac.h>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <type_traits>

// SFINAE: is &g.type well-formed? (Bitfields are not addressable.)
template <typename T, typename = void>
struct type_is_addressable : std::false_type {};

template <typename T>
struct type_is_addressable<T, std::void_t<decltype(&std::declval<T&>().type)>>
    : std::true_type {};

static void dump_bytes(const std::uint8_t* p, std::size_t n) {
    for (std::size_t i = 0; i < n; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (int)p[i] << " ";
    }
    std::cout << std::dec << std::setfill(' ');
}

int main() {
#ifdef GIAC_TYPE_ON_8BITS
    std::cout << "=== Mode: -DGIAC_TYPE_ON_8BITS (proposed fix) ===\n";
#else
    std::cout << "=== Mode: default (current GIAC_jll — bitfield type:5) ===\n";
#endif

    std::cout << "sizeof(giac::gen)                       = " << sizeof(giac::gen) << "\n";
    std::cout << "alignof(giac::gen)                      = " << alignof(giac::gen) << "\n";
    std::cout << "type field is addressable (8-bit):      "
              << std::boolalpha << type_is_addressable<giac::gen>::value << "\n";

    // Field offsets via raw buffer (skip the gen ctor — it needs giac init
    // and would crash without it).
    alignas(giac::gen) std::uint8_t storage[sizeof(giac::gen)];
    auto* g    = reinterpret_cast<giac::gen*>(storage);
    auto  base = reinterpret_cast<std::uintptr_t>(storage);

#ifdef GIAC_TYPE_ON_8BITS
    std::cout << "offsetof(type)                          = "
              << (reinterpret_cast<std::uintptr_t>(&g->type) - base) << "\n";
#endif
    std::cout << "offsetof(subtype)                       = "
              << (reinterpret_cast<std::uintptr_t>(&g->subtype) - base) << "\n";
    std::cout << "offsetof(reserved)                      = "
              << (reinterpret_cast<std::uintptr_t>(&g->reserved) - base) << "\n";
    std::cout << "offsetof(val)                           = "
              << (reinterpret_cast<std::uintptr_t>(&g->val) - base) << "\n";

    // Individual-field witness — write one field at a time, see which
    // bytes change. Exposes any cross-field aliasing.
    auto bytes = static_cast<const std::uint8_t*>(storage);

    std::memset(storage, 0, sizeof(storage));
    g->type = 1;
    std::cout << "\nAfter g->type = 1:                       ";
    dump_bytes(bytes, 8);
    std::cout << "\n";

    std::memset(storage, 0, sizeof(storage));
    g->subtype = 1;
    std::cout << "After g->subtype = 1:                    ";
    dump_bytes(bytes, 8);
    std::cout << "\n";

    std::memset(storage, 0, sizeof(storage));
    g->reserved = 0xABCD;
    std::cout << "After g->reserved = 0xABCD:              ";
    dump_bytes(bytes, 8);
    std::cout << "\n";

    // Each giac type tag — byte pattern with both type+subtype set.
    struct { const char* name; int value; } tags[] = {
        {"_INT_",   giac::_INT_},
        {"_DOUBLE_",giac::_DOUBLE_},
        {"_ZINT",   giac::_ZINT},
        {"_REAL",   giac::_REAL},
        {"_CPLX",   giac::_CPLX},
        {"_SYMB",   giac::_SYMB},
        {"_VECT",   giac::_VECT},
        {"_FRAC",   giac::_FRAC},
        {"_STRNG",  giac::_STRNG},
    };

    std::cout << "\nBytes 0..7 after memset(0), g->type=TAG, g->subtype=0x42:\n";
    for (auto& t : tags) {
        std::memset(storage, 0, sizeof(storage));
        g->type    = static_cast<unsigned char>(t.value);
        g->subtype = 0x42;
        std::cout << "  " << std::setw(8) << std::left << t.name
                  << " (" << t.value << ") : ";
        dump_bytes(bytes, 8);
        std::cout << "\n";
    }

    // Overflow probe — distinguishes 5-bit truncation vs 8-bit store.
    std::cout << "\nOverflow probe: g->type = 200\n";
    std::memset(storage, 0, sizeof(storage));
    g->type = 200;
    std::cout << "  read-back g->type = " << (int)g->type
              << "  byte[0] = 0x" << std::hex << std::setw(2)
              << std::setfill('0') << (int)bytes[0]
              << std::dec << std::setfill(' ') << "\n";

    // -----------------------------------------------------------------
    // Live-gen probe: actually evaluate expressions through giac and
    // inspect the type tag of the resulting gen. This directly tests
    // the Giac.jl#22 symptom ("MPFR real comes back tagged _DOUBLE_").
    // -----------------------------------------------------------------

    auto tag_name = [](int t) -> const char* {
        switch (t) {
            case giac::_INT_:    return "_INT_";
            case giac::_DOUBLE_: return "_DOUBLE_";
            case giac::_ZINT:    return "_ZINT";
            case giac::_REAL:    return "_REAL";
            case giac::_CPLX:    return "_CPLX";
            case giac::_SYMB:    return "_SYMB";
            case giac::_VECT:    return "_VECT";
            case giac::_FRAC:    return "_FRAC";
            case giac::_STRNG:   return "_STRNG";
            default:             return "?";
        }
    };

    std::cout << "\n=== Live-gen probe (evaluates expressions via giac) ===\n";
    std::cout << "  _DOUBLE_=" << giac::_DOUBLE_
              << ", _REAL=" << giac::_REAL << "\n";

    // Wrapper-style intentional leak — never delete to avoid teardown
    // crashes (matches what libgiac-julia-wrapper does).
    giac::context* ctx = new giac::context();

    const char* exprs[] = {
        "1.0",              // expected _DOUBLE_
        "evalf(pi)",        // default precision — expected _DOUBLE_
        "evalf(pi, 30)",    // moderate precision — expected _REAL on Linux
        "evalf(pi, 50)",    // high precision — expected _REAL on Linux
        "evalf(sqrt(2), 80)" // very high — definitely _REAL on Linux
    };
    for (const char* e : exprs) {
        try {
            giac::gen parsed(e, ctx);
            giac::gen result = giac::eval(parsed, ctx);
            auto rb = reinterpret_cast<const std::uint8_t*>(&result);
            std::cout << "  " << std::setw(22) << std::left << e
                      << " -> type=" << (int)result.type
                      << " (" << tag_name(result.type) << ")"
                      << "  bytes[0..3]=";
            for (int i = 0; i < 4; ++i) {
                std::cout << std::hex << std::setw(2) << std::setfill('0')
                          << (int)rb[i] << " ";
            }
            std::cout << std::dec << std::setfill(' ')
                      << " print=\"" << result.print(ctx) << "\"\n";
        } catch (const std::exception& ex) {
            std::cout << "  " << e << "  -> EXCEPTION: " << ex.what() << "\n";
        }
    }

    return 0;
}
