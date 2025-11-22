//
// Created by RestRegular on 2025/6/28
//

#include "../../include/lib/newrcc.h"

namespace cc {

    bool enableColorfulOutput = true;

    // TextColor静态常量定义
    constexpr std::string TextColor::RED = "\033[31m";
    constexpr std::string TextColor::GREEN = "\033[32m";
    constexpr std::string TextColor::YELLOW = "\033[33m";
    constexpr std::string TextColor::BLUE = "\033[34m";
    constexpr std::string TextColor::PURPLE = "\033[35m";
    constexpr std::string TextColor::CYAN = "\033[36m";
    constexpr std::string TextColor::GRAY = "\033[37m";
    constexpr std::string TextColor::DARK_GRAY = "\033[90m";
    constexpr std::string TextColor::LIGHT_RED = "\033[91m";
    constexpr std::string TextColor::LIGHT_GREEN = "\033[92m";
    constexpr std::string TextColor::LIGHT_YELLOW = "\033[93m";
    constexpr std::string TextColor::LIGHT_BLUE = "\033[94m";
    constexpr std::string TextColor::LIGHT_PURPLE = "\033[95m";
    constexpr std::string TextColor::LIGHT_CRAY = "\033[96m";
    constexpr std::string TextColor::LIGHT_GRAY = "\033[97m";

    // BackgroundColor静态常量定义
    constexpr std::string BackgroundColor::BLACK = "\033[40m";
    constexpr std::string BackgroundColor::RED = "\033[41m";
    constexpr std::string BackgroundColor::GREEN = "\033[42m";
    constexpr std::string BackgroundColor::YELLOW = "\033[43m";
    constexpr std::string BackgroundColor::BLUE = "\033[44m";
    constexpr std::string BackgroundColor::PURPLE = "\033[45m";
    constexpr std::string BackgroundColor::CYAN = "\033[46m";
    constexpr std::string BackgroundColor::GRAY = "\033[47m";

    // Decoration静态常量定义
    constexpr std::string Decoration::BOLD = "\033[1m";
    constexpr std::string Decoration::ITALIC = "\033[3m";
    constexpr std::string Decoration::LIGHT_UNDERLINE = "\033[4m";
    constexpr std::string Decoration::REVERSE = "\033[7m";
    constexpr std::string Decoration::THROUGH_LINE = "\033[9m";
    constexpr std::string Decoration::BOLD_UNDERLINE = "\033[21m";

    // 彩色文本函数实现
    std::string
    colorfulText(const std::string &text, const std::string &color, const std::vector<std::string> &decorations,
                 const bool reset) {
        std::string result = text;
        if (enableColorfulOutput) {
            for (const auto &deco: decorations) {
                result.insert(0, deco);
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