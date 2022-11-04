#include <catch2/catch_session.hpp>

#include "test.h"

int main(int argc, char* argv[]) { return Catch::Session{}.run(argc, argv); }