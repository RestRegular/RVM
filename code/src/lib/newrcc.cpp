//
// Created by RestRegular on 2025/6/28
//

#include "../../include/lib/newrcc.h"

namespace cc {

    bool enableColorfulOutput = true;

    // TextColor静态常量定义
    const std::string TextColor::RED = "\033[31m";
    const std::string TextColor::GREEN = "\033[32m";
    const std::string TextColor::YELLOW = "\033[33m";
    const std::string TextColor::BLUE = "\033[34m";
    const std::string TextColor::PURPLE = "\033[35m";
    const std::string TextColor::CYAN = "\033[36m";
    const std::string TextColor::GRAY = "\033[37m";
    const std::string TextColor::DARK_GRAY = "\033[90m";
    const std::string TextColor::LIGHT_RED = "\033[91m";
    const std::string TextColor::LIGHT_GREEN = "\033[92m";
    const std::string TextColor::LIGHT_YELLOW = "\033[93m";
    const std::string TextColor::LIGHT_BLUE = "\033[94m";
    const std::string TextColor::LIGHT_PURPLE = "\033[95m";
    const std::string TextColor::LIGHT_CRAY = "\033[96m";
    const std::string TextColor::LIGHT_GRAY = "\033[97m";

    // BackgroundColor静态常量定义
    const std::string BackgroundColor::BLACK = "\033[40m";
    const std::string BackgroundColor::RED = "\033[41m";
    const std::string BackgroundColor::GREEN = "\033[42m";
    const std::string BackgroundColor::YELLOW = "\033[43m";
    const std::string BackgroundColor::BLUE = "\033[44m";
    const std::string BackgroundColor::PURPLE = "\033[45m";
    const std::string BackgroundColor::CYAN = "\033[46m";
    const std::string BackgroundColor::GRAY = "\033[47m";

    // Decoration静态常量定义
    const std::string Decoration::BOLD = "\033[1m";
    const std::string Decoration::ITALIC = "\033[3m";
    const std::string Decoration::LIGHT_UNDERLINE = "\033[4m";
    const std::string Decoration::REVERSE = "\033[7m";
    const std::string Decoration::THROUGH_LINE = "\033[9m";
    const std::string Decoration::BOLD_UNDERLINE = "\033[21m";

    // 彩色文本函数实现
    std::string
    colorfulText(const std::string &text, const std::string &color, const std::vector<std::string> &decorations,
                 bool reset) {
        std::string result = text;
        if (enableColorfulOutput) {
            for (const auto &deco: decorations) {
                result = deco + result;
            }
            if (!color.empty()) {
                result = color + result;
                if (reset) {
                    result += Reset;
                }
            }
        }
        return result;
    }

    std::string
    colorfulText(const std::string &text, const Color &color, const std::vector<std::string> &decorations, bool reset) {
        return colorfulText(text, color.str(), decorations, reset);
    }

    std::string colorfulText(const std::string &text, const std::pair<TextColor, BackgroundColor> &colors,
                             const std::vector<std::string> &decorations, bool reset) {
        std::string result = text;
        if (enableColorfulOutput) {
            for (const auto &deco: decorations) {
                result = deco + result;
            }
            result = colors.first.str() + colors.second.str() + result;
            if (reset) {
                result += Reset;
            }
        }
        return result;
    }

    std::string
    colorfulText(const std::string &text, const std::pair<std::tuple<int, int, int>, std::tuple<int, int, int>> &colors,
                 const std::vector<std::string> &decorations, bool reset) {
        if (enableColorfulOutput) {
            auto [r, g, b] = colors.first;
            auto [r2, g2, b2] = colors.second;
            TextColor textColor(r, g, b);
            BackgroundColor bgColor(r2, g2, b2);
            return colorfulText(text, std::make_pair(textColor, bgColor), decorations, reset);
        } else {
            return text;
        }
    }

} // namespace cc    