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
const COLORREF C_MENU_NARDS_GREY = RGB(240, 240, 24);
const COLORREF C_MENU_TEXT_WHITE = RGB(230, 230, 230);
const COLORREF C_MENU_GREEN = RGB(0, 135, 0);
const COLORREF C_PTL_GREEN = RGB(3, 102, 0);
const COLORREF C_WX_BLUE = RGB(0, 32, 120);
const COLORREF C_PPS_TBS_PINK = RGB(244, 186, 255);

// Math
const double PI = 3.14159;

// Tag Settings
const int TAG_MAX_X_OFFSET = 60;
const int TAG_MAX_Y_OFFSET = 45;
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
const int BUTTON_MENU_QUICK_LOOK = 210;
const int BUTTON_MENU_EXT_ALT = 211;

const int BUTTON_MENU_PTL_CLOSE = 212;
const int BUTTON_MENU_PTL_CLEAR_ALL = 213;
const int BUTTON_MENU_PTL_ALL_ON = 214;
const int BUTTON_MENU_PTL_OPTIONS = 215;
const int BUTTON_MENU_HALO_CLOSE = 216;
const int BUTTON_MENU_HALO_CLEAR_ALL = 217;
const int BUTTON_MENU_HALO_MOUSE = 218;
const int BUTTON_MENU_HALO_TOOL = 219;
const int BUTTON_MENU_WX_HIGH = 220;
const int BUTTON_MENU_WX_ALL = 221;
const int BUTON_MENU_DEST_APRT = 222;
const int BUTTON_MENU_CLOSE_DEST = 223;
const int BUTTON_MENU_CLEAR_DEST = 224;
const int BUTTON_MENU_DEST_1 = 225;
const int BUTTON_MENU_DEST_2 = 226;
const int BUTTON_MENU_DEST_3 = 227;
const int BUTTON_MENU_DEST_4 = 228;
const int BUTTON_MENU_DEST_5 = 229;
const int BUTTON_MENU_DEST_ICAO = 230;
const int BUTTON_MENU_DEST_DIST = 231;
const int BUTTON_MENU_DEST_EST = 232;
const int BUTTON_MENU_DEST_VFR = 233;

const int BUTTON_MENU_QL_CJS = 234;
const int BUTTON_MENU_PTL_WB = 235;
const int BUTTON_MENU_PTL_EB = 236;


const int BUTTON_MENU_RMB_MENU = 240;
const int BUTTON_MENU_RMB_MENU_SECONDARY = 241;

const int BUTTON_MENU_SETUP = 254;

const int BUTTON_MENU_TBS_HDG = 250;
const int BUTTON_MENU_TBS_MIXED = 251;
const int BUTTON_MENU_CRDA = 252;
const int BUTTON_MENU_CRDA_CLOSE = 253;

// Menu Modules
const int MODULE_1_X = 0;
const int MODULE_1_Y = 2;
const int MODULE_2_X = 300;
const int MODULE_2_Y = 2;

// Menu functions
const int FUNCTION_ALT_FILT_LOW = 301;
const int FUNCTION_ALT_FILT_HIGH = 302;
const int FUNCTION_ALT_FILT_SAVE = 303;

const int FUNCTION_DEST_ICAO_1 = 304;
const int FUNCTION_DEST_ICAO_2 = 305;
const int FUNCTION_DEST_ICAO_3 = 306;
const int FUNCTION_DEST_ICAO_4 = 307;
const int FUNCTION_DEST_ICAO_5 = 308;

const int FUNCTION_RMB_POPUP = 309;

const int FUNCTION_TBS_HDG = 310;
const int TBS_FOLLOWER_TOGGLE = 311;

const int BUTTON_MENU_CPDLC = 312;

// Radar Background
const int SCREEN_BACKGROUND = 501;

const int FREE_TEXT = 1101;

// AC lists
const int LIST_OFF_SCREEN = 8000;
const int LIST_TIME_ATIS = 8001;
const int LIST_MESSAGES = 8002;


const int LIST_ITEM_SIMPLE_STRING = 8100;

// Module 2 : distances relative to module origin
const int HALO_TOOL_X = 0;
const int HALO_TOOL_Y = 0;

// Appwindows
const int WINDOW_TITLE_BAR = 3000;


const int WINDOW_FLIGHT_PLAN = 6000;
const int WINDOW_CTRL_REMARKS = 6001;
const int WINDOW_LIST_BOX_ELEMENT = 6002;
const int WINDOW_TEXT_FIELD = 6003;
const int WINDOW_HANDOFF_EXT_CJS = 6004;
const int WINDOW_POINT_OUT = 6005;
const int HIGHLIGHT_POINT_OUT_ACCEPT = 6006;
const int WINDOW_DIRECT_TO = 6007;
const int WINDOW_SCROLL_ARROW_UP = 6008;
const int WINDOW_SCROLL_ARROW_DOWN = 6009;
const int WINDOW_FREE_TEXT = 6010;
const int WINDOW_CPDLC = 6011;
const int WINDOW_CPDLC_EDITOR = 6012;


// Textfields
const int TEXTFIELD_CPDLC_MESSAGE = 6050;
const int TEXTFIELD_CPDLC_PENDING_UPLINK = 6051;