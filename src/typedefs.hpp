/*
 * typedefs.hpp
 *
 *  Created on: 06 feb 2018
 *      Author: giovanni
 *
 * Licensed Material - Property of Hex Keep s.r.l.
 * (c) Copyright Hex Keep s.r.l. 2012-2014
 */

#ifndef SRC_TYPEDEFS_HPP_
#define SRC_TYPEDEFS_HPP_
#include <sys/types.h>
#include <assert.h>

typedef char sbyte;
typedef unsigned char ubyte;
typedef short int sh_int;
typedef unsigned short int ush_int;
#if !defined( __cplusplus ) || defined( DEFINE_BOOL )
typedef char bool;
#endif
typedef char byte;




#endif /* SRC_TYPEDEFS_HPP_ */
