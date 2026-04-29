#ifndef BPEXPR_H
#define BPEXPR_H

#include <string>

class BpExpr
{
public:
    // Evaluate condition. Returns true to fire breakpoint.
    // Empty/whitespace expression returns true (unconditional).
    // On parse/eval error, returns true (fail-safe — break and let user fix).
    static bool eval(const std::string &expr);

    // Returns empty string if expression parses cleanly, error message otherwise.
    static std::string validate(const std::string &expr);
};

#endif // BPEXPR_H
