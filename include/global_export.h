#pragma once

#ifdef __cplusplus
#define EXPORT extern "C" __attribute__((visibility("default")))
#define WEBSOCKET_SERVER_LIB_EXPORT __attribute__((visibility("default")))
#else
#define WEBSOCKET_SERVER_LIB_EXPORT __attribute__((visibility("default")))
#endif
