import copy
import os
import re
from enum import Enum
from datetime import datetime
import json
from typing import List

from dotenv import load_dotenv
from newrcc.c_console import ctext
from newrcc.c_color import TextColor

load_dotenv()
VERSION_FILE_PATH = os.getenv("PROJECT_VERSION_FILE_PATH")
LOG_FILE_PATH = os.getenv("LOG_FILE_PATH")
UPDATE_CONFIG_PATH = os.getenv("UPDATE_CONFIG_PATH")
UPDATE_CONFIG_CACHE_PATH = os.getenv("UPDATE_CONFIG_CACHE_PATH")

OVERVIEW_MARKER = "<!-- Add Version Overview -->"
DETAILS_MARKER = "<!-- Add Version Details -->"

UPDATE_LOG_TEMPLATE = {
    "validate": False,
    "version": {
        "vs_desc": "...",
        "change_type": ["fix"]
    },
    "update_log": {
        "author": "RestRegular",
        "log_contents": [
            "..."
        ]
    },
    "annotation": {
        "annotations": [
            "Please ensure that the value of the 'validate' field is true."
        ],
        "increment_types": [
            "major",
            "minor",
            "patch"
        ],
        "change_types": [
            "Breaking", "break",    # Major - 不兼容的变更
            "Feature", "feat",      # Minor - 向后兼容的功能新增
            "BugFix", "fix",        # Patch - 向后兼容的问题修复
            "Optimize", "opt",      # Patch - 性能优化（不影响功能）
            "Deprecate", "depr",    # Minor - 标记即将移除的功能（仍保持兼容）
            "Refactor", "ref",      # Patch - 代码重构（不影响功能和接口）
            "Docs", "doc",          # Patch - 文档更新
            "Test", "test",         # Patch - 测试相关修改
        ]
    },
}

CHANGE_TYPE_MAP = {
    "rel": "Release",
    "feat": "Feature",
    "fix": "BugFix",
    "opt": "Optimize",
    "depr": "Deprecate",
    "ref": "Refactor",
}


class IncrementType(Enum):
    """版本号递增类型"""
    MAJOR = 0  # 主版本号，递增后，次版本号和补丁版本号会重置为0
    MINOR = 1  # 次版本号，递增后，补丁版本号会重置为0
    PATCH = 2  # 补丁版本号

    def __str__(self):
        return self.name

    @staticmethod
    def from_str(type_str: str) -> 'IncrementType':
        """从字符串转换为IncrementType枚举"""
        try:
            return getattr(IncrementType, type_str.upper())
        except KeyError:
            raise ValueError(f'无效的版本号自增类型：{type_str}')

    @staticmethod
    def from_int(type_int: int) -> 'IncrementType':
        """从整数转换为IncrementType枚举"""
        for type_ in IncrementType:
            if type_.value == type_int:
                return type_
        raise ValueError(f'无效的版本号自增类型：{type_int}')

    def increment(self, major: int, minor: int, patch: int) -> dict[str, int] | None:
        """根据递增类型更新版本号"""
        if self == IncrementType.MAJOR:
            return {'MAJOR': major + 1, 'MINOR': 0, 'PATCH': 0}
        elif self == IncrementType.MINOR:
            return {'MAJOR': major, 'MINOR': minor + 1, 'PATCH': 0}
        elif self == IncrementType.PATCH:
            return {'MAJOR': major, 'MINOR': minor, 'PATCH': patch + 1}
        return None

    @staticmethod
    def from_change_type(change_types: List[str]) -> 'IncrementType':
        INCREMENT_TYPE_MATCH_CHANGE_TYPE_MAP = {
            "Breaking": 0, # major
            "Feature": 1, # minor
            "BugFix": 2, # patch
            "Optimize": 2, # patch
            "Deprecate": 1, # minor
            "Refactor": 2, # patch
            "Docs": 2, # patch
            "Test": 2, # patch
        }
        max_inc_type = 2
        for ct in change_types:
            # 转换为标准变更类型
            if ct in CHANGE_TYPE_MAP.values():
                std_type = ct
            else:
                std_type = CHANGE_TYPE_MAP.get(ct, "BugFix")

            # 获取对应的版本递增类型值，默认为patch
            inc_type = INCREMENT_TYPE_MATCH_CHANGE_TYPE_MAP.get(std_type, 1)
            if inc_type < max_inc_type:
                max_inc_type = inc_type

        return IncrementType.from_int(max_inc_type)


def get_version_str(version: dict) -> str:
    return f'v{version["MAJOR"]}.{version["MINOR"]}.{version["PATCH"]}'


def parse_version_from_content(version_content: str) -> dict:
    """从版本文件中解析版本号"""
    version_pattern = re.compile(r'^\s*#define\s+VERSION_(\S+)\s+(\d+)\s*$', re.MULTILINE)
    matches = re.findall(version_pattern, version_content)

    if not matches:
        print(ctext('>>> 错误：未找到任何版本号定义，请检查版本文件格式', TextColor.RED))
        raise Exception('未找到任何版本号定义，请检查版本文件格式')

    version = {}
    for key, value in matches:
        try:
            version[key] = int(value)
        except ValueError:
            print(ctext(f'>>> 错误：版本号 {key} 的值 {value} 不是有效的整数', TextColor.RED))
            raise Exception(f'版本号 {key} 的值 {value} 不是有效的整数')

    required_versions = ['MAJOR', 'MINOR', 'PATCH']
    for ver in required_versions:
        if ver not in version:
            print(ctext(f'>>> 错误：未找到{ver}版本号，请检查版本文件格式', TextColor.RED))
            raise Exception(f'未找到{ver}版本号，请检查版本文件格式')
    return version


def parse_update_config(version: dict) -> tuple[dict, str, str]:
    """
    解析更新配置文件，提取版本更新相关信息并生成更新日志片段
    :param version: 原始版本号
    :return tuple[dict, str, str]: （新版本号，日志概要，详细日志）
    """
    date = datetime.now().strftime("%Y-%m-%d")
    time = datetime.now().strftime("%Y/%m/%d %H:%M")
    print(ctext(f">>> 正在处理更新配置，当前时间: {time}", TextColor.CYAN))

    try:
        with open(UPDATE_CONFIG_PATH, 'r', encoding='utf-8') as cf:
            update_config = json.load(cf)
        print(ctext(f">>> 成功读取配置文件: {UPDATE_CONFIG_PATH}", TextColor.GREEN))
    except FileNotFoundError:
        print(ctext(f">>> 错误: 配置文件不存在 - {UPDATE_CONFIG_PATH}", TextColor.RED))
        raise
    except json.JSONDecodeError:
        print(ctext(f">>> 错误: 配置文件格式无效，不是合法的JSON - {UPDATE_CONFIG_PATH}", TextColor.RED))
        raise
    except Exception as e:
        print(ctext(f">>> 错误: 读取配置文件失败: {str(e)}", TextColor.RED))
        raise

    required_fields = ['validate', 'version', 'update_log']
    missing_fields = [field for field in required_fields if field not in update_config]
    if missing_fields:
        error_msg = f"配置文件缺少必要字段: {', '.join(missing_fields)}"
        print(ctext(f">>> 错误: 配置文件格式错误 - {error_msg}", TextColor.RED))
        raise Exception(f"配置文件格式错误: {error_msg}")

    validate = update_config['validate']
    if not isinstance(validate, bool) or not validate:
        print(ctext(">>> 错误: 配置文件验证失败 - 'validate' 必须设为 True", TextColor.RED))
        raise Exception("配置文件验证失败: 'validate' 字段必须为 True")

    version_config = update_config['version']
    update_log = update_config['update_log']

    vs_change_type = version_config.get('change_type')
    vs_cg_tps = copy.copy(vs_change_type)
    valid_types = UPDATE_LOG_TEMPLATE['annotation']['change_types']
    if isinstance(vs_change_type, str):
        if vs_change_type not in valid_types:
            error_msg = f"无效的版本号变更类型: {vs_change_type}，允许的值为: {', '.join(valid_types)}"
            print(ctext(f">>> 错误: {error_msg}", TextColor.RED))
            raise Exception(error_msg)
        vs_change_type = f"[{vs_change_type}]"
    elif isinstance(vs_change_type, list):
        if not vs_change_type:
            error_msg = "版本号变更类型列表不能为空"
            print(ctext(f">>> 错误: {error_msg}", TextColor.RED))
            raise Exception(error_msg)
        invalid_types = [vct for vct in vs_change_type if vct not in valid_types]
        if invalid_types:
            error_msg = f"无效的版本号变更类型: {', '.join(invalid_types)}，允许的值为: {', '.join(valid_types)}"
            print(ctext(f">>> 错误: {error_msg}", TextColor.RED))
            raise Exception(error_msg)
        vs_change_type = ' '.join([f"[{CHANGE_TYPE_MAP[vct] if vct in CHANGE_TYPE_MAP else vct}]" for vct in vs_change_type if vct in valid_types])
    else:
        error_msg = f"无效的版本号变更类型：{type(vs_change_type)}"
        print(ctext(f">>> 错误: {error_msg}", TextColor.RED))
        raise Exception(error_msg)

    try:
        vs_incr_type = IncrementType.from_change_type(vs_cg_tps)
        print(ctext(f">>> 成功解析版本增量类型: {vs_incr_type}", TextColor.GREEN))
    except KeyError:
        print(ctext(">>> 错误: 配置文件缺少'increment_type'字段", TextColor.RED))
        raise
    except ValueError as e:
        print(ctext(f">>> 错误: 无效的版本增量类型: {str(e)}", TextColor.RED))
        raise

    version = vs_incr_type.increment(*version.values())

    vs_desc = version_config.get('vs_desc', '无描述')
    print(ctext(f">>> 版本更新描述: {vs_desc}", TextColor.CYAN))

    author = update_log.get('author', '未知作者')
    log_contents = update_log.get('log_contents', [])
    if not isinstance(log_contents, list):
        print(ctext(">>> 错误: 日志内容格式错误 - 'log_contents'必须是列表类型", TextColor.RED))
        raise Exception("日志内容格式错误: 'log_contents'必须是列表")
    if not log_contents:
        print(ctext(">>> 警告: 更新日志内容为空", TextColor.YELLOW))

    version_str = get_version_str(version)
    print(ctext(f">>> 处理的版本号: {version_str}", TextColor.CYAN))

    overview_row = (f"| **`{version_str}`** | {date} | {vs_desc} | **{vs_change_type}** | {author} |\n"
                    f"{OVERVIEW_MARKER}")

    details = (f"{DETAILS_MARKER}\n\n"
               f"---\n"
               f"### 版本号 `{version_str}` ({date})\n"
               f"- **类型**：**{vs_change_type}**\n"
               f"- **作者**：{author}\n"
               f"- **更新时间**：{time}\n"
               f"- **更新内容**：\n"
               f"{'\n'.join([f'  - {content}' for content in log_contents])}")

    try:
        with open(UPDATE_CONFIG_CACHE_PATH, 'w', encoding='utf-8') as cfc:
            json.dump(update_config, cfc, ensure_ascii=False, indent=2)
        print(ctext(f">>> 已缓存配置文件: {UPDATE_CONFIG_CACHE_PATH}", TextColor.GREEN))
        with open(UPDATE_CONFIG_PATH, 'w', encoding='utf-8') as cf:
            json.dump(UPDATE_LOG_TEMPLATE, cf, indent=2)
        print(ctext(f">>> 已重置配置文件: {UPDATE_CONFIG_PATH}", TextColor.GREEN))
    except Exception as e:
        print(ctext(f">>> 错误: 重置配置文件失败: {str(e)}", TextColor.RED))
        raise

    print(ctext(">>> 配置文件解析完成", TextColor.CYAN))
    return version, overview_row, details


def record_update_log(overview_row: str, details: str) -> None:
    """记录更新日志"""
    print(ctext(f'>>> 正在记录更新日志：{LOG_FILE_PATH}', TextColor.CYAN))
    with open(LOG_FILE_PATH, 'r', encoding='utf-8') as lf:
        print(ctext(f'>>> 正在读取更新日志内容...', TextColor.CYAN))
        log_content = lf.read()
    log_content = log_content.replace(OVERVIEW_MARKER, overview_row)
    log_content = log_content.replace(DETAILS_MARKER, details)
    with open(LOG_FILE_PATH, 'w', encoding='utf-8') as lf:
        lf.write(log_content)
        print(ctext(f'>>> 记录更新日志成功：{LOG_FILE_PATH}', TextColor.GREEN))


def auto_version_increment() -> bool:
    """将版本文件中的版本号自动递增"""
    try:
        with open(VERSION_FILE_PATH, 'r', encoding='gb18030') as vf:
            print(ctext(f'>>> 正在读取版本文件：{VERSION_FILE_PATH}', TextColor.CYAN))
            version_content = vf.read()
        version = parse_version_from_content(version_content)

        major_ver = version['MAJOR']
        minor_ver = version['MINOR']
        patch_ver = version['PATCH']
        print(ctext(f'>>> 当前版本号：v{major_ver}.{minor_ver}.{patch_ver}', TextColor.CYAN))

        new_version, overview_row, details = parse_update_config(version)
        record_update_log(overview_row, details)

        print(ctext(f'>>> 递增后版本号：v{new_version["MAJOR"]}.{new_version["MINOR"]}.{new_version["PATCH"]}', TextColor.GREEN))

        updated_content = version_content
        for key, value in new_version.items():
            pattern = re.compile(rf'^\s*#define\s+VERSION_{key}\s+\d+\s*$', re.MULTILINE)
            updated_content = pattern.sub(f'#define VERSION_{key} {value}', updated_content)

        with open(VERSION_FILE_PATH, 'w', encoding='gb18030') as vf:
            vf.write(updated_content)

        print(ctext(f'>>> 版本号已成功更新并保存：{VERSION_FILE_PATH}', TextColor.GREEN))
        return True

    except FileNotFoundError:
        print(ctext(f'>>> 错误：版本文件不存在 - {VERSION_FILE_PATH}', TextColor.RED))
    except PermissionError:
        print(ctext(f'>>> 错误：没有权限访问文件 - {VERSION_FILE_PATH}', TextColor.RED))
    except Exception as e:
        print(ctext(f'>>> 错误：处理版本文件时发生错误 - {str(e)}', TextColor.RED))

    return False


if __name__ == '__main__':
    result = auto_version_increment()
    exit(0 if result else -1)
