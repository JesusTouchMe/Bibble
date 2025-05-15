// Copyright 2025 JesusTouchMe

#ifndef BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_DIAGNOSTIC_ASSERT_H
#define BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_DIAGNOSTIC_ASSERT_H 1

namespace diagnostic {
    [[noreturn]]
    void AssertFail(const char* message, const char* file, unsigned int line);
}

#define assert(expr) \
    (void)           \
    ((!!(expr)) ||   \
     (diagnostic::AssertFail(#expr, __FILE__, __LINE__), 0))

#endif // BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_DIAGNOSTIC_ASSERT_H
