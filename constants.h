#pragma once

#include <string>
#include "EuroScopePlugIn.h"

using namespace std;
using namespace EuroScopePlugIn;


// Colours for the plugin
const COLORREF C_WHITE = RGB(255, 255, 255);
const COLORREF C_PPS_ORANGE = RGB(242, 120, 57);
const COLORREF C_PPS_YELLOW = RGB(202, 205, 169);
const COLORREF C_PPS_MAGENTA = RGB(197, 38, 212);
const COLORREF C_PPS_RED = RGB(209, 39, 27);
const COLORREF C_MENU_GREY1 = RGB(40, 40, 40);
const COLORREF C_MENU_GREY2 = RGB(55, 55, 55);
const COLORREF C_MENU_GREY3 = RGB(66, 66, 66);
const COLORREF C_MENU_GREY4 = RGB(140, 140, 140);
const COLORREF C_MENU_TEXT_WHITE = RGB(230, 230, 230);
const COLORREF C_PTL_GREEN = RGB(3, 102, 0);

// Math
const double PI = 3.14159;

// Tag Settings
const int TAG_MAX_X_OFFSET = 45;
const int TAG_MAX_Y_OFFSET = 30;
const int TAG_WIDTH = 70;
const int TAG_HEIGHT = 25;

// Tag Items
const int TAG_ITEM_PLANE_HALO = 1;
const int AIRCRAFT_SYMBOL = 200;
const int AIRCRAFT_CJS = 400;

const int TAG_ITEM_FP_CS = 401;
const int TAG_ITEM_FP_FINAL_ALTITUDE = 402;
const int TAG_ALT = 403;

const int BUTTON_MENU = 201;
const int BUTTON_MENU_HALO_OPTIONS = 202;
const int BUTTON_MENU_ALT_FILT_OPT = 203;
const int BUTTON_MENU_ALT_FILT_ON = 204;
const int BUTTON_MENU_ALT_FILT_SAVE = 205;
const int BUTTON_MENU_PTL_TOOL = 206;
const int BUTTON_MENU_RELOCATE = 207;
const int BUTTON_MENU_EXTRAP_FP = 208;
const int BUTTON_MENU_OVRD_ALL = 209;

// Menu Modules
const int MODULE_1_X = 0;
const int MODULE_1_Y = 2;
const int MODULE_2_X = 300;
const int MODULE_2_Y = 2;

// Menu functions
const int FUNCTION_ALT_FILT_LOW = 301;
const int FUNCTION_ALT_FILT_HIGH = 302;
const int FUNCTION_ALT_FILT_SAVE = 303;

// Radar Background
const int SCREEN_BACKGROUND = 501;

const int ADD_FREE_TEXT = 1101;
const int DELETE_FREE_TEXT = 1102;
const int DELETE_ALL_FREE_TEXT = 1103;


// Module 2 : distances relative to module origin
const int HALO_TOOL_X = 0;
const int HALO_TOOL_Y = 0;