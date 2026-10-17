// Your own tests. Add at least one per session for a case the provided tests do not cover.
// Files named tests/student_*.cpp are built into the ex01-student test executable.
// Run: ctest --preset dev -L ex01-student

#include "sensorSync/odometryBuffer.h"
#include "testHelpers.h"

#include <gtest/gtest.h>

namespace sensorSync::test
{
namespace
{

using namespace std::chrono_literals;

// Example of the pattern; replace or extend it.
TEST(StudentOdometryBuffer, NewBufferIsEmpty)
{
    OdometryBuffer const buffer{10};
    EXPECT_EQ(buffer.size(), 0U);
}

} // namespace
} // namespace sensorSync::test
