/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2014, 2015 Sen "Senorsen" Zhang <sen@senorsen.com>
 *
 * All Rights Reserved. 
 *
 */

#ifndef _MACROS_H_
#define _MACROS_H_

#define LIKELY(expr) (__builtin_expect (expr, 1))
#define UNLIKELY(expr) (__builtin_expect (expr, 0))

#endif

