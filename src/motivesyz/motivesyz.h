/**
 * @file motivesyz.h
 * @brief Main header for MotiveSyz library
 *
 * Includes all public modules of the MotiveSyz C utility library.
 * Provides clean, safe alternatives to standard C library functions.
 */

#ifndef MOTIVESYZ_H
#define MOTIVESYZ_H

/**
 * @defgroup motivesyz MotiveSyz Library
 * @brief Clean C utilities for modern development
 *
 * MotiveSyz provides safe, expressive alternatives to standard C library
 * functions with better error handling and modern APIs.
 */

#include "core/ms_memory.h"   /**< Safe memory allocation utilities */
#include "core/ms_print.h"    /**< Simplified output and formatting */

/**
 * @brief Library version information
 */
#define MOTIVESYZ_VERSION "0.2.0"     /**< Current library version */
#define MOTIVESYZ_AUTHOR "elrt(elliktronic)"  /**< Library author */

#endif
