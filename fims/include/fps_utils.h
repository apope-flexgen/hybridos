/*
 * fps_utils.h
 *
 *  Created on: July 30, 2018
 *      Author: srappl
 */
#ifndef FPS_UTILS_H_
#define FPS_UTILS_H_

#define FPS_RELEASE_PRINT(...)  printf(__VA_ARGS__)
#define FPS_ERROR_PRINT(...)    fprintf(stderr, __VA_ARGS__)

#ifdef FPS_DEBUG_MODE
#define FPS_DEBUG_PRINT(...)    printf(__VA_ARGS__)
#define FPS_TEST_PRINT(...)     
#elif FPS_TEST_MODE
#define FPS_DEBUG_PRINT(...)    printf(__VA_ARGS__)
#define FPS_TEST_PRINT(...)     printf(__VA_ARGS__)
#else
#define FPS_DEBUG_PRINT(...)
#define FPS_TEST_PRINT(...)
#endif

#endif
