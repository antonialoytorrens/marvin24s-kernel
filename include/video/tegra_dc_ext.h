/*
 * Copyright (C) 2011, NVIDIA Corporation
 *
 * Author: Robert Morell <rmorell@nvidia.com>
 * Some code based on fbdev extensions written by:
 *	Erik Gilling <konkers@android.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 */

#ifndef __TEGRA_DC_EXT_H
#define __TEGRA_DC_EXT_H

#include <linux/types.h>
#include <linux/ioctl.h>
#if defined(__KERNEL__)
# include <linux/time.h>
#else
# include <time.h>
# include <unistd.h>
#endif

#define TEGRA_DC_EXT_FMT_P1		0
#define TEGRA_DC_EXT_FMT_P2		1
#define TEGRA_DC_EXT_FMT_P4		2
#define TEGRA_DC_EXT_FMT_P8		3
#define TEGRA_DC_EXT_FMT_B4G4R4A4	4
#define TEGRA_DC_EXT_FMT_B5G5R5A	5
#define TEGRA_DC_EXT_FMT_B5G6R5		6
#define TEGRA_DC_EXT_FMT_AB5G5R5	7
#define TEGRA_DC_EXT_FMT_B8G8R8A8	12
#define TEGRA_DC_EXT_FMT_R8G8B8A8	13
#define TEGRA_DC_EXT_FMT_B6x2G6x2R6x2A8	14
#define TEGRA_DC_EXT_FMT_R6x2G6x2B6x2A8	15
#define TEGRA_DC_EXT_FMT_YCbCr422	16
#define TEGRA_DC_EXT_FMT_YUV422		17
#define TEGRA_DC_EXT_FMT_YCbCr420P	18
#define TEGRA_DC_EXT_FMT_YUV420P	19
#define TEGRA_DC_EXT_FMT_YCbCr422P	20
#define TEGRA_DC_EXT_FMT_YUV422P	21
#define TEGRA_DC_EXT_FMT_YCbCr422R	22
#define TEGRA_DC_EXT_FMT_YUV422R	23
#define TEGRA_DC_EXT_FMT_YCbCr422RA	24
#define TEGRA_DC_EXT_FMT_YUV422RA	25

#define TEGRA_DC_EXT_BLEND_NONE		0
#define TEGRA_DC_EXT_BLEND_PREMULT	1
#define TEGRA_DC_EXT_BLEND_COVERAGE	2

struct tegra_dc_ext_flip_windowattr {
	__s32	index;
	__u32	buff_id;
	__u32	blend;
	__u32	offset;
	__u32	offset_u;
	__u32	offset_v;
	__u32	stride;
	__u32	stride_uv;
	__u32	pixformat;
	/*
	 * x, y, w, h are fixed-point: 20 bits of integer (MSB) and 12 bits of
	 * fractional (LSB)
	 */
	__u32	x;
	__u32	y;
	__u32	w;
	__u32	h;
	__u32	out_x;
	__u32	out_y;
	__u32	out_w;
	__u32	out_h;
	__u32	z;
	__u32	swap_interval;
	struct timespec timestamp;
	__u32	pre_syncpt_id;
	__u32	pre_syncpt_val;
	/* Leave some wiggle room for future expansion */
	__u32   pad[8];
};

#define TEGRA_DC_EXT_FLIP_N_WINDOWS	3

struct tegra_dc_ext_flip {
	struct tegra_dc_ext_flip_windowattr win[TEGRA_DC_EXT_FLIP_N_WINDOWS];
	__u32	post_syncpt_id;
	__u32	post_syncpt_val;
};

/*
 * Cursor image format:
 * - Tegra hardware supports two colors: foreground and background, specified
 *   by the client in RGB8.
 * - The image should be specified as two 1bpp bitmaps immediately following
 *   each other in memory.  Each pixel in the final cursor will be constructed
 *   from the bitmaps with the following logic:
 *		bitmap1 bitmap0
 *		(mask)  (color)
 *		  1	   0	transparent
 *		  1	   1	inverted
 *		  0	   0	background color
 *		  0	   1	foreground color
 * - Exactly one of the SIZE flags must be specified.
 */
#define TEGRA_DC_EXT_CURSOR_IMAGE_FLAGS_SIZE_32x32	1
#define TEGRA_DC_EXT_CURSOR_IMAGE_FLAGS_SIZE_64x64	2
struct tegra_dc_ext_cursor_image {
	struct {
		__u8	r;
		__u8	g;
		__u8	b;
	} foreground, background;
	__u32	buff_id;
	__u32	flags;
};

/* Possible flags for struct nvdc_cursor's flags field */
#define TEGRA_DC_EXT_CURSOR_FLAGS_VISIBLE	1

struct tegra_dc_ext_cursor {
	__s16 x;
	__s16 y;
	__u32 flags;
};

/*
 * Color conversion is performed as follows:
 *
 * r = sat(kyrgb * sat(y + yof) + kur * u + kvr * v)
 * g = sat(kyrgb * sat(y + yof) + kug * u + kvg * v)
 * b = sat(kyrgb * sat(y + yof) + kub * u + kvb * v)
 *
 * Coefficients should be specified as fixed-point values; the exact format
 * varies for each coefficient.
 * The format for each coefficient is listed below with the syntax:
 * - A "s." prefix means that the coefficient has a sign bit (twos complement).
 * - The first number is the number of bits in the integer component (not
 *   including the optional sign bit).
 * - The second number is the number of bits in the fractional component.
 *
 * All three fields should be tightly packed, justified to the LSB of the
 * 16-bit value.  For example, the "s.2.8" value should be packed as:
 * (MSB) 5 bits of 0, 1 bit of sign, 2 bits of integer, 8 bits of frac (LSB)
 */
struct tegra_dc_ext_csc {
	__u32 win_index;
	__u16 yof;	/* s.7.0 */
	__u16 kyrgb;	/*   2.8 */
	__u16 kur;	/* s.2.8 */
	__u16 kvr;	/* s.2.8 */
	__u16 kug;	/* s.1.8 */
	__u16 kvg;	/* s.1.8 */
	__u16 kub;	/* s.2.8 */
	__u16 kvb;	/* s.2.8 */
};

#define TEGRA_DC_EXT_SET_NVMAP_FD \
	_IOW('D', 0x00, __s32)

#define TEGRA_DC_EXT_GET_WINDOW \
	_IOW('D', 0x01, __u32)
#define TEGRA_DC_EXT_PUT_WINDOW \
	_IOW('D', 0x02, __u32)

#define TEGRA_DC_EXT_FLIP \
	_IOWR('D', 0x03, struct tegra_dc_ext_flip)

#define TEGRA_DC_EXT_GET_CURSOR \
	_IO('D', 0x04)
#define TEGRA_DC_EXT_PUT_CURSOR \
	_IO('D', 0x05)
#define TEGRA_DC_EXT_SET_CURSOR_IMAGE \
	_IOW('D', 0x06, struct tegra_dc_ext_cursor_image)
#define TEGRA_DC_EXT_SET_CURSOR \
	_IOW('D', 0x07, struct tegra_dc_ext_cursor)

#define TEGRA_DC_EXT_SET_CSC \
	_IOW('D', 0x08, struct tegra_dc_ext_csc)

/*
 * Returns the auto-incrementing vblank syncpoint for the head associated with
 * this device node
 */
#define TEGRA_DC_EXT_GET_VBLANK_SYNCPT \
	_IOR('D', 0x09, __u32)


enum tegra_dc_ext_control_output_type {
	TEGRA_DC_EXT_DSI,
	TEGRA_DC_EXT_LVDS,
	TEGRA_DC_EXT_VGA,
	TEGRA_DC_EXT_HDMI,
	TEGRA_DC_EXT_DVI,
};

/*
 * Get the properties for a given output.
 *
 * handle (in): Which output to query
 * type (out): Describes the type of the output
 * connected (out): Non-zero iff the output is currently connected
 * associated_head (out): The head number that the output is currently
 *      bound to.  -1 iff the output is not associated with any head.
 * head_mask (out): Bitmask of which heads the output may be bound to (some
 *      outputs are permanently bound to a single head).
 */
struct tegra_dc_ext_control_output_properties {
	__u32 handle;
	enum tegra_dc_ext_control_output_type type;
	__u32 connected;
	__s32 associated_head;
	__u32 head_mask;
};

struct tegra_dc_ext_control_output_edid {
	__u32 handle;
	__u32 size;
	void *data;
};

struct tegra_dc_ext_event {
	__u32	type;
	ssize_t	data_size;
	char	data[0];
};

#define TEGRA_DC_EXT_EVENT_HOTPLUG	0x1
struct tegra_dc_ext_control_event_hotplug {
	__u32 handle;
};

#define TEGRA_DC_EXT_CONTROL_GET_NUM_OUTPUTS \
	_IOR('C', 0x00, __u32)
#define TEGRA_DC_EXT_CONTROL_GET_OUTPUT_PROPERTIES \
	_IOWR('C', 0x01, struct tegra_dc_ext_control_output_properties)
#define TEGRA_DC_EXT_CONTROL_GET_OUTPUT_EDID \
	_IOWR('C', 0x02, struct tegra_dc_ext_control_output_edid)
#define TEGRA_DC_EXT_CONTROL_SET_EVENT_MASK \
	_IOW('C', 0x03, __u32)

#endif /* __TEGRA_DC_EXT_H */
