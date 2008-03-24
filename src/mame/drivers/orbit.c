/***************************************************************************

Atari Orbit Driver

  game 0 = beginner slow
  game 1 = beginner medium
  game 2 = beginner fast
  game 3 = intermediate slow
  game 4 = intermediate fast
  game 5 = expert fast shells only
  game 6 = expert slow
  game 7 = expert medium
  game 8 = expert fast
  game 9 = super expert

  Flip screen DIP doesn't work because it's not supported by the game.

***************************************************************************/

#include "driver.h"
#include "orbit.h"
#include "sound/discrete.h"


#define MASTER_CLOCK		XTAL_12_096MHz


static UINT8 orbit_misc_flags;



/*************************************
 *
 *  Interrupts and timing
 *
 *************************************/

static TIMER_CALLBACK( nmi_32v )
{
	int scanline = param;
	int nmistate = (scanline & 32) && (orbit_misc_flags & 4);
	cpunum_set_input_line(machine, 0, INPUT_LINE_NMI, nmistate ? ASSERT_LINE : CLEAR_LINE);
}


static TIMER_CALLBACK( irq_off )
{
	cpunum_set_input_line(machine, 0, 0, CLEAR_LINE);
}


static INTERRUPT_GEN( orbit_interrupt )
{
	cpunum_set_input_line(machine, 0, 0, ASSERT_LINE);
	timer_set(video_screen_get_time_until_vblank_end(machine->primary_screen), NULL, 0, irq_off);
}



/*************************************
 *
 *  Bit flags
 *
 *************************************/

static void update_misc_flags(running_machine *machine, UINT8 val)
{
	orbit_misc_flags = val;

	/* BIT0 => UNUSED       */
	/* BIT1 => LOCKOUT      */
	/* BIT2 => NMI ENABLE   */
	/* BIT3 => HEAT RST LED */
	/* BIT4 => PANEL BUS OC */
	/* BIT5 => PANEL STROBE */
	/* BIT6 => HYPER LED    */
	/* BIT7 => WARNING SND  */

	discrete_sound_w(machine, ORBIT_WARNING_EN, orbit_misc_flags & 0x80);

	set_led_status(0, orbit_misc_flags & 0x08);
	set_led_status(1, orbit_misc_flags & 0x40);

	coin_lockout_w(0, !(orbit_misc_flags & 0x02));
	coin_lockout_w(1, !(orbit_misc_flags & 0x02));
}


static WRITE8_HANDLER( orbit_misc_w )
{
	UINT8 bit = offset >> 1;

	if (offset & 1)
		update_misc_flags(machine, orbit_misc_flags | (1 << bit));
	else
		update_misc_flags(machine, orbit_misc_flags & ~(1 << bit));
}



/*************************************
 *
 *  Machine setup
 *
 *************************************/

static MACHINE_RESET( orbit )
{
	update_misc_flags(machine, 0);
}



/*************************************
 *
 *  Address maps
 *
 *************************************/

static ADDRESS_MAP_START( orbit_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x00ff) AM_MIRROR(0x0700) AM_RAM
	AM_RANGE(0x0800, 0x0800) AM_MIRROR(0x07ff) AM_READ(input_port_0_r)
	AM_RANGE(0x1000, 0x1000) AM_MIRROR(0x07ff) AM_READ(input_port_1_r)
	AM_RANGE(0x1800, 0x1800) AM_MIRROR(0x07ff) AM_READ(input_port_2_r)
	AM_RANGE(0x2000, 0x2000) AM_MIRROR(0x07ff) AM_READ(input_port_3_r)
	AM_RANGE(0x2800, 0x2800) AM_MIRROR(0x07ff) AM_READ(input_port_4_r)
	AM_RANGE(0x3000, 0x33bf) AM_MIRROR(0x0400) AM_READWRITE(SMH_RAM, orbit_playfield_w) AM_BASE(&orbit_playfield_ram)
	AM_RANGE(0x33c0, 0x33ff) AM_MIRROR(0x0400) AM_RAM AM_BASE(&orbit_sprite_ram)
	AM_RANGE(0x3800, 0x3800) AM_MIRROR(0x00ff) AM_WRITE(orbit_note_w)
	AM_RANGE(0x3900, 0x3900) AM_MIRROR(0x00ff) AM_WRITE(orbit_noise_amp_w)
	AM_RANGE(0x3a00, 0x3a00) AM_MIRROR(0x00ff) AM_WRITE(orbit_note_amp_w)
	AM_RANGE(0x3c00, 0x3c0f) AM_MIRROR(0x00f0) AM_WRITE(orbit_misc_w)
	AM_RANGE(0x3e00, 0x3e00) AM_MIRROR(0x00ff) AM_WRITE(orbit_noise_rst_w)
	AM_RANGE(0x3f00, 0x3f00) AM_MIRROR(0x00ff) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x6000, 0x7fff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( orbit )
	PORT_START /* 0800 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) /* actually buttons */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START /* 1000 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) /* actually buttons */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START /* 1800 */
	PORT_DIPNAME( 0x07, 0x00, "Play Time Per Credit" )
	PORT_DIPSETTING( 0x00, "0:30" )
	PORT_DIPSETTING( 0x01, "1:00" )
	PORT_DIPSETTING( 0x02, "1:30" )
	PORT_DIPSETTING( 0x03, "2:00" )
	PORT_DIPSETTING( 0x04, "2:30" )
	PORT_DIPSETTING( 0x05, "3:00" )
	PORT_DIPSETTING( 0x06, "3:30" )
	PORT_DIPSETTING( 0x07, "4:00" )
	PORT_DIPNAME( 0x18, 0x00, DEF_STR( Language ) )
	PORT_DIPSETTING( 0x00, DEF_STR( English ) )
	PORT_DIPSETTING( 0x08, DEF_STR( Spanish ) )
	PORT_DIPSETTING( 0x10, DEF_STR( French ) )
	PORT_DIPSETTING( 0x18, DEF_STR( German ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Free_Play ))
	PORT_DIPSETTING( 0x00, DEF_STR( Off ))
	PORT_DIPSETTING( 0x20, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown )) /* probably unused */
	PORT_DIPSETTING( 0x00, DEF_STR( Off ))
	PORT_DIPSETTING( 0x40, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown )) /* probably unused */
	PORT_DIPSETTING( 0x00, DEF_STR( Off ))
	PORT_DIPSETTING( 0x80, DEF_STR( On ))

	PORT_START /* 2000 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Game Reset") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Game 9") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Game 8") PORT_CODE(KEYCODE_8_PAD)
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Flip_Screen ))
	PORT_DIPSETTING( 0x00, DEF_STR( Off ))
	PORT_DIPSETTING( 0x08, DEF_STR( On ))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Heat Reset") PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_DIPNAME( 0x20, 0x20, "NEXT TEST" ) /* should be off */
	PORT_DIPSETTING( 0x20, DEF_STR( Off ))
	PORT_DIPSETTING( 0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x40, "DIAG TEST" ) /* should be off */
	PORT_DIPSETTING( 0x40, DEF_STR( Off ))
	PORT_DIPSETTING( 0x00, DEF_STR( On ))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_VBLANK )

	PORT_START /* 2800 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Game 7 / Strong Gravity") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Game 6 / Stars") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Game 5 / Unlimited Supplies") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Game 4 / Space Stations") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Game 3 / Black Hole") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Game 2 / Zero Gravity") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Game 1 / Negative Gravity") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Game 0 / Bounce Back") PORT_CODE(KEYCODE_0_PAD)
INPUT_PORTS_END



/*************************************
 *
 *  Graphics layouts
 *
 *************************************/

static const gfx_layout orbit_full_sprite_layout =
{
	8, 32,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP8(0,1) },
	{ STEP32(0,8) },
	0x100
};


static const gfx_layout orbit_upper_sprite_layout =
{
	8, 16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP8(0,1) },
	{ STEP16(0,8) },
	0x100
};


static const gfx_layout orbit_lower_sprite_layout =
{
	8, 16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP8(0,1) },
	{ STEP16(0x80,8) },
	0x100
};


static const gfx_layout orbit_tile_layout =
{
	8, 8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	0x40
};


static GFXDECODE_START( orbit )
	GFXDECODE_ENTRY( REGION_GFX1, 0, orbit_full_sprite_layout, 0, 1 )
	GFXDECODE_ENTRY( REGION_GFX1, 0, orbit_upper_sprite_layout, 0, 1 )
	GFXDECODE_ENTRY( REGION_GFX1, 0, orbit_lower_sprite_layout, 0, 1 )
	GFXDECODE_SCALE( REGION_GFX2, 0, orbit_tile_layout, 0, 1, 2, 2 )
GFXDECODE_END



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static MACHINE_DRIVER_START( orbit )

	/* basic machine hardware */
	MDRV_CPU_ADD(M6800, MASTER_CLOCK / 16)
	MDRV_CPU_PROGRAM_MAP(orbit_map, 0)
	MDRV_CPU_VBLANK_INT("main", orbit_interrupt)

	MDRV_TIMER_ADD_SCANLINE("32V", nmi_32v, "main", 0, 32)

	MDRV_MACHINE_RESET(orbit)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_RAW_PARAMS(MASTER_CLOCK*2, 384*2, 0, 256*2, 261*2, 0, 240*2)

	MDRV_GFXDECODE(orbit)
	MDRV_PALETTE_LENGTH(2)
	MDRV_PALETTE_INIT(black_and_white)
	MDRV_VIDEO_START(orbit)
	MDRV_VIDEO_UPDATE(orbit)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD_TAG("discrete", DISCRETE, 0)
	MDRV_SOUND_CONFIG_DISCRETE(orbit)
	MDRV_SOUND_ROUTE(0, "left", 1.0)
	MDRV_SOUND_ROUTE(1, "right", 1.0)
MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( orbit )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD_NIB_LOW ( "033701.h2", 0x6800, 0x400, CRC(6de43b85) SHA1(1643972f45d3a0dd6540158c575cd84cee2b0c9a) )
	ROM_LOAD_NIB_HIGH( "033693.l2", 0x6800, 0x400, CRC(8878409e) SHA1(a14e0161705bbc230f0aec1837ebc41d62178368) )
	ROM_LOAD_NIB_LOW ( "033702.h1", 0x6C00, 0x400, CRC(8166bdcb) SHA1(b7ae6cd46b4aff6e1e1ec9273cf068dec4a8cd46) )
	ROM_LOAD_NIB_HIGH( "033694.l1", 0x6C00, 0x400, CRC(5337a8ee) SHA1(1606bfa652bb5253c387f11c96d77d7a84983344) )
	ROM_LOAD_NIB_LOW ( "033699.f2", 0x7000, 0x400, CRC(b498b36f) SHA1(5d150af193196fccd7c20ba731a020a9ae75e516) )
	ROM_LOAD_NIB_HIGH( "033691.m2", 0x7000, 0x400, CRC(6cbabb21) SHA1(fffb3f7be73c72b4775d8cdfe174c75ae4389cba) )
	ROM_LOAD_NIB_LOW ( "033700.f1", 0x7400, 0x400, CRC(9807c922) SHA1(b6b62530b24d967104f632540ef98f2b4780c3ed) )
	ROM_LOAD_NIB_HIGH( "033692.m1", 0x7400, 0x400, CRC(96167d1b) SHA1(6f272b2f1b30aa94f51ea5710f4114bfdea19f2c) )
	ROM_LOAD_NIB_LOW ( "033697.e2", 0x7800, 0x400, CRC(19ccf0dc) SHA1(7d12c4985bd0a25ef518246faf2849e5a0cf600b) )
	ROM_LOAD_NIB_HIGH( "033689.n2", 0x7800, 0x400, CRC(ea3b70c1) SHA1(5e985fed057f362deaeb5e4049c4e8c1d449d6e1) )
	ROM_LOAD_NIB_LOW ( "033698.e1", 0x7C00, 0x400, CRC(356a7c32) SHA1(a3496c0f9d9f3e2e0b452cdc0e908dc93d179990) )
	ROM_RELOAD(                     0xFC00, 0x400 )
	ROM_LOAD_NIB_HIGH( "033690.n1", 0x7C00, 0x400, CRC(f756ebd4) SHA1(4e473541b712078c6a81901714a6243de348e543) )
	ROM_RELOAD(                     0xFC00, 0x400 )

	ROM_REGION( 0x1000, REGION_GFX1, ROMREGION_DISPOSE )  /* sprites */
	ROM_LOAD( "033712.b7", 0x0000, 0x800, CRC(cfd43bf2) SHA1(dbca0da6ed355aac921bae5adeef2f384f5fa2c3) )
	ROM_LOAD( "033713.d7", 0x0800, 0x800, CRC(5ac89f4d) SHA1(747889b33cd83510a640e68fb4581a3e881c43a3) )

	ROM_REGION( 0x200, REGION_GFX2, ROMREGION_DISPOSE )   /* tiles */
	ROM_LOAD( "033711.a7", 0x0000, 0x200, CRC(9987174a) SHA1(d2117b6e6d64c29aef8ad8c94256baea493bce5c) )

	ROM_REGION( 0x100, REGION_PROMS, 0 )                  /* sync, unused */
	ROM_LOAD( "033688.p6", 0x0000, 0x100, CRC(ee66ddba) SHA1(5b9ae4cbf019375c8d54528b69280413c641c4f2) )
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1978, orbit, 0, orbit, orbit, 0, 0, "Atari", "Orbit", 0 )
