// rflink_util.h
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifndef RFLINK_UTIL_H
#define RFLINK_UTIL_H

bool decode(const uint16_t pulses[], size_t pulseCount);

#endif // RFLINK_UTIL_H
