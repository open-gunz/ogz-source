#pragma once

// for future use: make it possible to reload when using last magazine
#define RELOADALWAYS

// controls whether it's possible to launch the client only once.
//#define ONECLIENT

// Controls whether voicechat is enabled, and whether libraries this feature
// depends on (portaudio, Opus) are linked into the Gunz executable.
#define VOICECHAT

// Controls whether portals can be created.
#define PORTAL

// Controls whether rockets can be reflected by blocking them.
//#define REFLECT_ROCKETS

// Controls whether attacks can be blocked in the guard_start animation.
//#define GUARD_START_CAN_BLOCK

// Controls whether aiming at a player will change the crosshair to a different pick bitmap.
// This allows for the creation of extremely simple triggerbots, so it's off by default.
//#define CROSSHAIR_PICK

// Controls whether the /timescale command can be used in developer mode.
#define TIMESCALE

// Controls whether the locator IP(s) are loaded from config.xml or system.mrs/system.xml
#define LOAD_LOCATOR_FROM_CONFIG_XML

// The interval between client transmission of MC_PEER_BASICINFO packets in milliseconds.
// Basicinfo packets contain several core pieces of physical information:
// Position, direction, velocity, animation indices and selected item slot.
#define BASICINFO_INTERVAL 33 // Milliseconds

// Controls whether your camera direction is locked to a small 90 degree area
// when dashing, hanging, etc. See ZMyCharacter::IsDirLocked for a full list.
#define NO_DIRECTION_LOCK

// Default field of view in radians.
#define DEFAULT_FOV		1.22173048f // 70 degrees in radians

// Default distance from the camera to the near plane in the projection transform.
// 5.0f in vanilla Gunz
#define DEFAULT_NEAR_Z	5.0f

// Default distance from the camera to the far plane in the projection transform.
// 10000.0f in vanilla Gunz
#define DEFAULT_FAR_Z	100000.0f

// Controls whether only MRS files will be loaded in release mode
//#define ONLY_LOAD_MRS_FILES

// Controls whether every map in the maps folder will be added
// to the ingame maps list regardless of whether it's in g_MapDesc or not
#define ADD_ALL_MAPS

// Controls whether the new ingame chat is used
#define NEW_CHAT

// Controls whether the slash decal shows up
// immediately or 250 ms delayed.
// It's delayed in vanilla Gunz.
#define INSTANT_SLASH_DECAL

// Prevents /<command> where <command> isn't a recognized command
// from showing up in chat.
#define HIDE_UNRECOGNIZED_COMMAND_INPUTS

// Controls whether the Gunz window in the Windows taskbar
// flashes on respawn and whisper, respectively
#define FLASH_WINDOW_ON_RESPAWN
#define FLASH_WINDOW_ON_WHISPER

// Controls whether the HP and AP of the player are drawn
// as numbers on top of the health gauges in the UI
#define DRAW_HPAP_NUMBERS

// Controls whether FOV can be changed (outside of debug mode).
#define ENABLE_FOV_OPTION

// Controls whether clans can be created with no sponsors.
#define SOLO_CLAN_CREATION

// Controls whether the /suicide command kills you instantly or after a delay.
#define DELAYED_SUICIDE

// Controls whether guard_cancel registers as a block on the attacking client.
#define GUARD_CANCEL_FIX

// Controls whether other players' view of a player's equipment in a stage updates when they change
// equipment in the inventory (backported from 1.5).
#define UPDATE_STAGE_EQUIP_LOOK

// Controls whether the equipment on the stage charviewer is updated immediately.
#define UPDATE_STAGE_CHARVIEWER

// Controls whether the hit sound (fx_myhit) stacks when hitting multiple enemies at once.
#define DONT_STACK_HITSOUNDS

// Controls whether the game saves resourses when the main window is out of focus
//#define GAME_FOCUS_CHECK