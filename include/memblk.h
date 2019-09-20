#ifndef __MEMBLK_H
#define __MEMBLK_H
/*
 * Copyright (c) 2018 Elettronica GF s.r.l.
 *    Andrea Collamati <andrea.collamati@elettronicagf.it>
 *    Stefano Donati <stefano.donati@elettronicagf.it>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifdef CONFIG_MEMBLK

extern u32 base;
extern u32 width;

void memblk_init(void);

#endif	/* CONFIG_MEMBLK */
#endif	/* __MEMBLK_H */
