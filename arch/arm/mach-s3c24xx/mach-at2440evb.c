/* linux/arch/arm/mach-s3c2440/mach-at2440evb.c
 *
 * Copyright (c) 2008 Ramax Lo <ramaxlo@gmail.com>
 *      Based on mach-anubis.c by Ben Dooks <ben@simtec.co.uk>
 *      and modifications by SBZ <sbz@spgui.org> and
 *      Weibing <http://weibing.blogbus.com>
 *
 * For product information, visit http://www.arm.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/gpio.h>
#include <linux/gpio_keys.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/io.h>
#include <linux/serial_core.h>
#include <linux/serial_s3c.h>
#include <linux/dm9000.h>
#include <linux/platform_device.h>


#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>

#include <mach/hardware.h>
#include <mach/fb.h>
#include <asm/irq.h>
#include <asm/mach-types.h>

#include <mach/regs-gpio.h>
#include <mach/regs-lcd.h>
#include <mach/gpio-samsung.h>
#include <linux/platform_data/mtd-nand-s3c2410.h>
#include <linux/platform_data/i2c-s3c2410.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/mtd/partitions.h>

#include <plat/devs.h>
#include <plat/cpu.h>
#include <linux/platform_data/mmc-s3cmci.h>
#include <plat/samsung-time.h>

#include <sound/s3c24xx_uda134x.h>
#include "common.h"

static struct map_desc at2440evb_iodesc[] __initdata = {
	/* Nothing here */
};

#define UCON S3C2410_UCON_DEFAULT
#define ULCON (S3C2410_LCON_CS8 | S3C2410_LCON_PNONE)
#define UFCON (S3C2410_UFCON_RXTRIG8 | S3C2410_UFCON_FIFOMODE)

static struct s3c2410_uartcfg at2440evb_uartcfgs[] __initdata = {
	[0] = {
		.hwport	     = 0,
		.flags	     = 0,
		.ucon	     = UCON,
		.ulcon	     = ULCON,
		.ufcon	     = UFCON,
		.clk_sel	= S3C2410_UCON_CLKSEL1 | S3C2410_UCON_CLKSEL2,
	},
	[1] = {
		.hwport	     = 1,
		.flags	     = 0,
		.ucon	     = UCON,
		.ulcon	     = ULCON,
		.ufcon	     = UFCON,
		.clk_sel	= S3C2410_UCON_CLKSEL1 | S3C2410_UCON_CLKSEL2,
	},
	[2] = {
		.hwport	     = 2,
		.flags	     = 0,
		.ucon	     = UCON,
		.ulcon	     = ULCON,
		.ufcon	     = UFCON,
		.clk_sel	= S3C2410_UCON_CLKSEL1 | S3C2410_UCON_CLKSEL2,
	},
};

/* NAND Flash on AT2440EVB board */

static struct mtd_partition __initdata at2440evb_default_nand_part[] = {
	[0] = {
		.name	= "U-boot",
		.size	= SZ_512K,
		.offset	= 0,
	},
	[1] = {
		.name	= "Prarm",
		.size	= SZ_512K,
		.offset	= SZ_512K,
	},
	[2] = {
		.name	= "Kernel",
		.size	= SZ_4M,
		.offset	= SZ_1M,
	},
	[3] = {
		.name	= "RootFS",
		.offset	= SZ_4M+SZ_1M,
		.size	= MTDPART_SIZ_FULL,
	},
};

static struct s3c2410_nand_set __initdata at2440evb_nand_sets[] = {
	[0] = {
		.name		= "nand",
		.nr_chips	= 1,
		.nr_partitions	= ARRAY_SIZE(at2440evb_default_nand_part),
		.partitions	= at2440evb_default_nand_part,
	},
};

static struct s3c2410_platform_nand __initdata at2440evb_nand_info = {
	.tacls		= 10,
	.twrph0		= 25,
	.twrph1		= 10,
	.nr_sets	= ARRAY_SIZE(at2440evb_nand_sets),
	.sets		= at2440evb_nand_sets,
};

/* DM9000AEP 10/100 ethernet controller */
/*
static struct resource at2440evb_dm9k_resource[] = {
	[0] = DEFINE_RES_MEM(S3C2410_CS3, 4),
	[1] = DEFINE_RES_MEM(S3C2410_CS3 + 4, 4),
	[2] = DEFINE_RES_NAMED(IRQ_EINT7, 1, NULL, IORESOURCE_IRQ \
					| IORESOURCE_IRQ_HIGHEDGE),
};*/
static struct resource at2440evb_dm9k_resource[] = {
	[0] = DEFINE_RES_MEM(S3C2410_CS3+0x300, 3),
	[1] = DEFINE_RES_MEM(S3C2410_CS3+0x300 + 4, 3),
	[2] = DEFINE_RES_NAMED(IRQ_EINT7, 1, NULL, IORESOURCE_IRQ \
					| IORESOURCE_IRQ_HIGHEDGE),
};


static struct dm9000_plat_data at2440evb_dm9k_pdata = {
	.flags		= (DM9000_PLATF_16BITONLY | DM9000_PLATF_NO_EEPROM),
};

static struct platform_device at2440evb_device_eth = {
	.name		= "dm9000",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(at2440evb_dm9k_resource),
	.resource	= at2440evb_dm9k_resource,
	.dev		= {
		.platform_data	= &at2440evb_dm9k_pdata,
	},
};

static struct s3c24xx_mci_pdata at2440evb_mci_pdata __initdata = {
	.gpio_detect	= S3C2410_GPG(10),
};

/* 7" LCD panel */

static struct s3c2410fb_display at2440evb_lcd_cfg __initdata = {

	.lcdcon5	= S3C2410_LCDCON5_FRM565 |
			  S3C2410_LCDCON5_INVVLINE |
			  S3C2410_LCDCON5_INVVFRAME |
			  S3C2410_LCDCON5_PWREN |
			  S3C2410_LCDCON5_HWSWP,

	.type		= S3C2410_LCDCON1_TFT,

	.width		= 240,
	.height		= 320,

	.pixclock	= 166667, /* HCLK 60 MHz, divisor 2 */
	.xres		= 240,
	.yres		= 320,
	.bpp		= 16,
	.left_margin	= 7,
	.right_margin	= 19,
	.hsync_len	= 3,
	.upper_margin	= 6,
	.lower_margin	= 7,
	.vsync_len	= 3,
};

static struct s3c2410fb_mach_info at2440evb_fb_info __initdata = {
	.displays	= &at2440evb_lcd_cfg,
	.num_displays	= 1,
	.default_display = 0,
};

//buttons
/*
#define DECLARE_BUTTON(p, k, n, w)	\
	{				\
		.gpio		= p,	\
		.code		= k,	\
		.desc		= n,	\
		.wakeup		= w,	\
		.active_low	= 1,	\
	}
static struct gpio_keys_button at2440_buttons[] = {
        DECLARE_BUTTON(S3C2410_GPF(3),KEY_1,"BUTTON1",0),
        DECLARE_BUTTON(S3C2410_GPF(4),KEY_2,"BUTTON2",0),
        DECLARE_BUTTON(S3C2410_GPF(5),KEY_3,"BUTTON3",0),
        DECLARE_BUTTON(S3C2410_GPF(6),KEY_4,"BUTTON4",0),
};
*/
static struct gpio_keys_button at2440_buttons[] = {
	{
		.gpio		= S3C2410_GPF(3),		/* K1 */
		.code		= KEY_1,
		.desc		= "BUTTON1",
		.active_low	= 1,
	},
	{
		.gpio		= S3C2410_GPF(4),		/* K2 */
		.code		= KEY_2,
		.desc		= "BUTTON2",
		.active_low	= 1,
	},
	{
		.gpio		= S3C2410_GPF(5),		/* K3 */
		.code		= KEY_3,
		.desc		= "BUTTON3",
		.active_low	= 1,
	},
	{
		.gpio		= S3C2410_GPF(6),		/* K4 */
		.code		= KEY_4,
		.desc		= "BUTTON4",
		.active_low	= 1,
	},

};
static struct gpio_keys_platform_data at2440_buttons_data = {
        .buttons  = at2440_buttons,
        .nbuttons = ARRAY_SIZE(at2440_buttons),
};

struct platform_device s3c_device_buttons = {
        .name             = "gpio-keys",
        .id               = -1,
		.dev		= {
			.platform_data  = &at2440_buttons_data,
		}
};

/* AUDIO */

static struct s3c24xx_uda134x_platform_data at2440evb_audio_pins = {
	.l3_clk = S3C2410_GPB(4),
	.l3_mode = S3C2410_GPB(2),
	.l3_data = S3C2410_GPB(3),
	.model = UDA134X_UDA1341
};

static struct platform_device at2440evb_audio = {
	.name		= "s3c24xx_uda134x",
	.id		= 0,
	.dev		= {
		.platform_data	= &at2440evb_audio_pins,
	},
};

static struct uda134x_platform_data s3c24xx_uda134x = {
	.l3 = {
		.gpio_clk = S3C2410_GPB(4),
		.gpio_data = S3C2410_GPB(3),
		.gpio_mode = S3C2410_GPB(2),
		.use_gpios = 1,
		.data_hold = 1,
		.data_setup = 1,
		.clock_high = 1,
		.mode_hold = 1,
		.mode = 1,
		.mode_setup = 1,
	},
	.model = UDA134X_UDA1341,
};

static struct platform_device uda134x_codec = {
		.name = "uda134x-codec",
		.id = -1,
		.dev = {
			.platform_data	= &s3c24xx_uda134x,
		},
};

//touch screen
/*
struct platform_device s3c_device_ts = {
	.name		  = "s3c2410-ts",
	.id		  = -1,
};

static struct s3c2410_ts_mach_info at2440evb_ts_info = {
	.delay = 10000,
	.presc = 49,
	.oversampling_shift = 2,
};
*/
static struct platform_device *at2440evb_devices[] __initdata = {
	&s3c_device_ohci,
	&s3c_device_wdt,
	&s3c_device_i2c0,
	&s3c_device_rtc,
	&s3c_device_usbgadget,
	&at2440evb_device_eth,
	&s3c_device_buttons,
	&s3c_device_nand,
	&s3c_device_sdi,
	&s3c2440_device_dma,
	&s3c_device_adc,	
	&s3c_device_lcd,
	&s3c_device_iis,
	&uda134x_codec,
	&at2440evb_audio,
	&s3c_device_ts,
};

static void __init at2440evb_map_io(void)
{
	s3c24xx_init_io(at2440evb_iodesc, ARRAY_SIZE(at2440evb_iodesc));
	s3c24xx_init_uarts(at2440evb_uartcfgs, ARRAY_SIZE(at2440evb_uartcfgs));
	samsung_set_timer_source(SAMSUNG_PWM3, SAMSUNG_PWM4);
}

static void __init at2440evb_init_time(void)
{
	s3c2440_init_clocks(16934400);
	samsung_timer_init();
}

static void __init at2440evb_init(void)
{
	s3c24xx_fb_set_platdata(&at2440evb_fb_info);
	s3c24xx_mci_set_platdata(&at2440evb_mci_pdata);
	s3c_nand_set_platdata(&at2440evb_nand_info);
	s3c_i2c0_set_platdata(NULL);

	platform_add_devices(at2440evb_devices, ARRAY_SIZE(at2440evb_devices));
}


MACHINE_START(AT2440EVB, "AT2440EVB")
	.atag_offset	= 0x100,
	.map_io		= at2440evb_map_io,
	.init_machine	= at2440evb_init,
	.init_irq	= s3c2440_init_irq,
	.init_time	= at2440evb_init_time,
MACHINE_END
