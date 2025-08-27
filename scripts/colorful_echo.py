import sys

from newrcc.CConsole import colorfulText
from newrcc.CColor import TextColor


if __name__ == '__main__':
    color = TextColor.BLUE
    if sys.argv[1] == 'b':
        color = TextColor.BLUE
    elif sys.argv[1] == 'g':
        color = TextColor.GREEN
    elif sys.argv[1] == 'r':
        color = TextColor.RED
    elif sys.argv[1] == 'y':
        color = TextColor.YELLOW
    elif sys.argv[1] == 'p':
        color = TextColor.PURPLE
    elif sys.argv[1] == 'c':
        color = TextColor.CYAN
    elif sys.argv[1] == 'w':
        color = TextColor.WHITE
    elif sys.argv[1] == 'G':
        color = TextColor.GRAY
    else:
        print(colorfulText(f'Invalid color: {sys.argv[1]}', TextColor.RED))
        sys.exit(-1)
    print(colorfulText(' '.join(sys.argv[2:]).replace('\\n', '\n'), color))
