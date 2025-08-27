import os
import sys
import subprocess
from typing import Union

from dotenv import load_dotenv
from newrcc.CConsole import colorfulText
from newrcc.CColor import TextColor, RESET


def log(msg: str, color: Union[TextColor, str] = TextColor.CYAN, start: str = "", end: str = "\n"):
    print(colorfulText(f"{start}>>> {msg}{end}", color), end="")


def error(error_msg: str, start: str = "", end: str = "\n"):
    print(colorfulText(f"{start}>>> 错误: {error_msg}{end}", TextColor.RED))
    sys.exit(-1)


def load_env() -> None:
    log('正在加载环境变量...')
    succeed = load_dotenv()
    if succeed:
        log('加载环境变量成功！', TextColor.GREEN)
    else:
        error('加载环境变量失败！')

    if os.getenv("RUN_VERSION_INCREMENT_SCRIPT", "true") == 'true':
        # 检查版本更新脚本是否存在
        vs_script_path = os.getenv("VERSION_SCRIPT_PATH")
        log(f'正在检查版本更新脚本：{vs_script_path}')
        if not os.path.exists(vs_script_path):
            error('版本更新脚本不存在！')
        # 执行版本更新脚本
        log('正在执行版本更新脚本...')
        try:
            subprocess.run([sys.executable, vs_script_path], check=True)
            log('版本更新成功！', TextColor.GREEN)
        except subprocess.CalledProcessError as e:
            error(f'版本更新失败：{e}')

    # 检查 cmake 路径是否存在
    cmake_path = os.getenv("CMAKE_PATH")
    log(f'正在检查 cmake 路径：{cmake_path}')
    if not os.path.exists(cmake_path):
        error('cmake 路径不存在！')
    # 构建项目
    cmake_command = os.getenv("CMAKE_COMMAND").split(' ')
    executable_path = os.getenv("PROJECT_EXECUTABLE_PATH")
    try:
        subprocess.run(cmake_command, check=True)
        if not os.path.exists(executable_path):
            raise FileNotFoundError(f'生成的可执行文件不存在：{executable_path}')
        log('构建项目成功！', TextColor.GREEN)
    except subprocess.CalledProcessError as e:
        error(f'构建项目失败：{e}')
    except FileNotFoundError as e:
        error(f'构建项目失败：{e}')

    # 获取运行参数
    run_args = os.getenv("RUN_ARGS", None)
    if not run_args:
        log("请输入运行参数：", TextColor.YELLOW)
        run_args = [arg.strip(' ') for arg in input(f"{TextColor.YELLOW}<<< ").split(" ")]
        print(end=RESET)
    else:
        run_args = run_args.split(' ')

    # 运行项目
    log('正在运行项目...')
    command = [executable_path, *run_args]
    log(' '.join(command), TextColor.BLUE)
    try:
        subprocess.run(command, check=True)
        log('运行项目成功！', TextColor.GREEN, start="\n")
    except subprocess.CalledProcessError as e:
        error(f'运行项目失败：{e}', start="\n")

    log('程序执行完毕')


if __name__ == '__main__':
    load_env()
