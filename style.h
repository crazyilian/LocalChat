#ifndef LOCAL_CHAT_STYLE_H
#define LOCAL_CHAT_STYLE_H

#include "global_variables.h"

namespace style {
    string rgb(unsigned char red, unsigned char green, unsigned char blue) {
        return "\033[38;2;" + to_string(red) + ";" + to_string(green) + ";" + to_string(blue) + "m";
    }
    const string red = rgb(255,0,0),      light_red = rgb(255,123,123),    dark_red = rgb(155,0,0);
    const string green = rgb(0,255,0),    light_green = rgb(131,255,131),  dark_green = rgb(0,128,0);
    const string blue = rgb(0,0,255),     light_blue = rgb(101,101,255),   dark_blue = rgb(0,0,140);
    const string cyan = rgb(0,255,255),   light_cyan = rgb(141,255,255),   dark_cyan = rgb(0,161,161);
    const string pink = rgb(255,0,255),   light_pink = rgb(255,130,255),   dark_pink = rgb(147,0,147);
    const string purple = rgb(175,0,255), light_purple = rgb(211,114,255), dark_purple = rgb(90,0,132);
    const string brown = "\033[0;33m",    orange = "",                     dark_brown = "";
    const string yellow = rgb(255,255,0), light_yellow = rgb(255,255,164), dark_yellow = rgb(159,159,0);
    const string gray = "",               light_gray = "\033[0;37m",       dark_gray = "\033[1;30m";
    const string black = rgb(0,0,0),      white = rgb(255,255,255);

    const string cursive = "\033[3m", bold = "\033[1m";
    const string underline = "\033[4m", upperline = "\033[53m", cross = "\033[9m";;
    const string blink = "\033[5m", hide = "\033[8m";
    const string normal = "\033[0m", no_color = "\033[39m";
    const string fade = "\033[2m", super = underline + upperline + blink;
    std::vector<string> colors_for_nick {light_red, light_blue, light_purple, dark_green, light_green,
                                         light_cyan, dark_cyan};
};

#endif
