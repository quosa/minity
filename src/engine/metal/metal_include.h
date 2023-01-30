#pragma once

// METAL - INCLUDE IS MESSY SO GET THAT OUT OF THE WAY...
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdtor-name"
// #pragma clang diagnostic ignored "-Werror"
#pragma clang diagnostic ignored "-Wc99-extensions"
#pragma clang diagnostic ignored "-Wc99-designator"
#pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
#pragma clang diagnostic ignored "-Wnested-anon-types"
#pragma clang diagnostic ignored "-Wpedantic"
// ignore the warnings from metal-cpp :-/
// In file included from external/metal-cpp/Foundation/Foundation.hpp:42:
// external/metal-cpp/Foundation/NSSharedPtr.hpp:162:33: error: ISO C++ requires the name after '::~' to be found in the same scope as the name before '::~' [-Werror,-Wdtor-name]
// _NS_INLINE NS::SharedPtr<_Class>::~SharedPtr()
#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>
#pragma clang diagnostic pop
