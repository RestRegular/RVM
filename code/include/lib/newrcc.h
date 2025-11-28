//
// Created by RestRegular on 2025/6/28
//

#ifndef COLORFUL_TEXT_HPP
#define COLORFUL_TEXT_HPP

#include <string>
#include <utility>
#include <vector>
#include <stdexcept>
#include <tuple>

namespace cc {

    extern bool enableColorfulOutput;

    const std::string Reset = "\033[0m";

    class Color {
    protected:
        std::string colorcode;

    public:
        explicit Color(std::string  color) : colorcode(std::move(color)) {}

        Color(int r, int g, int b, const std::string& type) {
            if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255) {
                throw std::invalid_argument("RGB values must be between 0 and 255");
            }
            colorcode = "\033[" + type + ";2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m";
        }

        virtual ~Color() = default;

        [[nodiscard]] std::string str() const {
            return colorcode;
        }
    };

    class TextColor : public Color {
    public:
        static const std::string RED;
        static const std::string GREEN;
        static const std::string YELLOW;
        static const std::string BLUE;
        static const std::string PURPLE;
        static const std::string CYAN;
        static const std::string GRAY;
        static const std::string DARK_GRAY;
        static const std::string LIGHT_RED;
        static const std::string LIGHT_GREEN;
        static const std::string LIGHT_YELLOW;
        static const std::string LIGHT_BLUE;
        static const std::string LIGHT_PURPLE;
        static const std::string LIGHT_CRAY;
        static const std::string LIGHT_GRAY;

        explicit TextColor(const std::string& color) : Color(color) {}
        TextColor(int r, int g, int b) : Color(r, g, b, "38") {}
    };

    class BackgroundColor : public Color {
    public:
        static const std::string BLACK;
        static const std::string RED;
        static const std::string GREEN;
        static const std::string YELLOW;
        static const std::string BLUE;
        static const std::string PURPLE;
        static const std::string CYAN;
        static const std::string GRAY;

        explicit BackgroundColor(const std::string& color) : Color(color) {}
        BackgroundColor(int r, int g, int b) : Color(r, g, b, "48") {}
    };

    class Decoration {
    private:
        std::string decoration;

    public:
        static const std::string BOLD;
        static const std::string ITALIC;
        static const std::string LIGHT_UNDERLINE;
        static const std::string REVERSE;
        static const std::string THROUGH_LINE;
        static const std::string BOLD_UNDERLINE;

        explicit Decoration(std::string  deco) : decoration(std::move(deco)) {}

        [[nodiscard]] std::string str() const {
            return decoration;
        }
    };

    std::string colorfulText(const std::string &text, const std::string &color = "",
                             const std::vector<std::string> &decorations = {},
                             bool reset = true);

    std::string colorfulText(const std::string &text, const Color &color,
                             const std::vector<std::string> &decorations = {},
                             bool reset = true);

    std::string colorfulText(const std::string &text, const std::pair<TextColor, BackgroundColor> &colors,
                             const std::vector<std::string> &decorations = {}, bool reset = true);

    std::string colorfulText(const std::string &text,
                             const std::pair<std::tuple<int, int, int>,
                                     std::tuple<int, int, int>> &colors,
                             const std::vector<std::string> &decorations = {},
                             bool reset = true);

} // namespace cc

#endif // COLORFUL_TEXT_HPP    