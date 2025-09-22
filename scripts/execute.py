import os
import sys
import subprocess
from typing import Union
import shutil

from dotenv import load_dotenv
from newrcc.c_console import ctext
from newrcc.c_color import TextColor, RESET

from auto_version_increment import parse_version_from_content, get_version_str


def log(msg: str, color: Union[TextColor, str] = TextColor.CYAN, start: str = "", end: str = "\n"):
    print(ctext(f"{start}>>> {msg}{end}", color), end="")


def error(error_msg: str, start: str = "", end: str = "\n"):
    print(ctext(f"{start}>>> 错误: {error_msg}{end}", TextColor.RED))
    sys.exit(-1)


def save_project_version_release(version: dict):
    executable_path = os.getenv("PROJECT_EXECUTABLE_PATH")
    release_dir = os.getenv("PROJECT_RELEASE_DIR")
    # 检查环境变量是否设置
    if not executable_path:
        raise Exception("未设置PROJECT_EXECUTABLE_PATH环境变量")

    if not release_dir:
        raise Exception("未设置PROJECT_RELEASE_DIR环境变量")

    major_dir = os.path.join(release_dir, f"v{version['MAJOR']}")
    if not os.path.exists(major_dir):
        os.makedirs(major_dir, exist_ok=True)
    minor_dir = os.path.join(major_dir, f"v{version['MAJOR']}.{version['MINOR']}")
    if not os.path.exists(minor_dir):
        os.makedirs(minor_dir, exist_ok=True)
    patch_dir = os.path.join(minor_dir, f"v{version['MAJOR']}.{version['MINOR']}.{version['PATCH']}")
    if not os.path.exists(patch_dir):
        os.makedirs(patch_dir, exist_ok=True)
    dest_dir = os.path.join(release_dir, major_dir, minor_dir, patch_dir)
    if not os.path.exists(dest_dir):
        os.makedirs(dest_dir, exist_ok=True)
    file_name = os.path.basename(executable_path)
    destination_path = os.path.join(dest_dir, file_name)
    try:
        shutil.copy2(executable_path, destination_path)
        log(f"已保存版本 {get_version_str(version)} release: {destination_path}")
    except Exception as e:
        error(f"保存版本 {version} release 失败：{e}")


def run(auto_increment_version: str = "true", *args, **kwargs) -> None:
    auto_increment_version = auto_increment_version == "true"
    log('正在加载环境变量...')
    succeed = load_dotenv()
    if succeed:
        log('加载环境变量成功！', TextColor.GREEN)
    else:
        error('加载环境变量失败！')

    if auto_increment_version and os.getenv("RUN_VERSION_INCREMENT_SCRIPT", "true") == 'true':
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
        if auto_increment_version:
            with open(os.getenv("PROJECT_VERSION_FILE_PATH"), 'r', encoding='gb18030') as vf:
                print(ctext(f'>>> 正在读取版本文件：{os.getenv("PROJECT_VERSION_FILE_PATH")}', TextColor.CYAN))
                version_content = vf.read()
            version = parse_version_from_content(version_content)
            save_project_version_release(version)
    except subprocess.CalledProcessError as e:
        error(f'构建项目失败：{e}')
    except FileNotFoundError as e:
        error(f'构建项目失败：{e}')
    except Exception as e:
        error(f'错误：{e}')

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
    run(*sys.argv[1:])
