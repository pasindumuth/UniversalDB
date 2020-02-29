#ifndef UNI_COMMON_H
#define UNI_COMMON_H

#include <cstdint>
#include <functional>
#include <memory>

#include <logging/log.h>

namespace uni {

using timestamp_t = uint64_t; // Timestamp type used in MultiVersion key-value store

template<typename T>
using custom_unique_ptr = std::unique_ptr<T, std::function<void(T*)>>;

}

#endif // UNI_COMMON_H
