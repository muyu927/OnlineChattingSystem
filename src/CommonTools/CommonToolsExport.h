#pragma once

#ifdef __cplusplus
#define EXPORT extern "C" __attribute__((visibility("default")))
#define COMMON_TOOLS_EXPORT __attribute__((visibility("default")))
#else
#define COMMON_TOOLS_EXPORT __attribute__((visibility("default")))
#endif