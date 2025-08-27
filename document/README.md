# RVM

<img src="https://github.com/user-attachments/assets/8ea29c7a-2f91-4e88-a389-97cfbe8850f4"></img>

**RVM**��Rio Virtual Machine������������ RA �����ļ��� RSI �����ļ���

# Ŀ¼

1. **[���ܸ���](#1-���ܸ���)** 
2. **[������������](#2-������������)**
    - **[������](#21-������)**
    - **[���벽��](#22-���벽��)**
3. **[ʹ�÷���](#3-ʹ�÷���)**
4. **[����˵��](#4-����˵��)**
    - **[��־����](#41-��־����)**
    - **[ѡ�����](#42-ѡ�����)**
    - **[λ�ò���](#43-λ�ò���)**
5. **[����ṹ](#5-����ṹ)**
6. **[������](#6-������)**
7. **[RA ���Թ淶](#7-ra-���Թ淶)**

## 1. ���ܸ���

RVM ��һ�����������Ҫ�������� RA �����ļ��� RSI �����ļ�����֧�ֶ��ֲ���������롢���С��汾���ȣ����ṩ����ϸ�Ĳ�������ѡ������û�����������в�����

## 2. ������������

### 2.1 ������

- CMake �汾 3.29 �����
- C++ 20 ��׼֧��

### 2.2 ���벽��

- **��¡����ֿ⵽���أ�**
    ```bash
    git clone https://github.com/RestRegular/RVM.git
    cd RVM
    ```
- **��������Ŀ¼�����룺**
    ```bash
    mkdir build
    cd build
    ```
- **ʹ�� CMake ���ɹ����ļ���**
    ```bash
    cmake ..
    ```
- **������Ŀ��**
    ```bash
    cmake --build .
    ```

- **���ߵķ�����**

  ���ڿ�����ʱ��ʹ���� bat ��д��һ������ű���������ֱ�����У������ʹ�� CMake �Ļ�����ֱ��ʹ���������ű����������£�
    ```bash
    # file: execute.bat
    @echo off
    cls
    color 0A
    set COMMAND_PREFIX="^>^>^>"
    set CMAKE_PATH=D:\soft\Clion\CLion-2024.2.3\bin\cmake\win\x64\bin\cmake.exe # �뽫��·���滻Ϊ��� CMake ·��

    set PROJECT_TARGET=RVM

    set PROJECT_DIR=D:\ClionProjects\%PROJECT_TARGET% # �뽫��·���滻Ϊ�����Ŀ·��
    set BUILD_DIR=%PROJECT_DIR%\cmake-build-debug # �������ĿĿ¼�´���һ����Ϊ cmake-build-debug ��Ŀ¼�����������ƣ�ע���޸� BUILD_DIR ������
    set EXECUTABLE_PATH=%BUILD_DIR%\%PROJECT_TARGET%.exe
    echo %COMMAND_PREFIX:"=% Building project...
    "%CMAKE_PATH%" --build %BUILD_DIR% --target %PROJECT_TARGET% -j 10 # ����ʹ���� -j 10 ����������Ը����Լ���������е���
    color 07
    cls
    echo %COMMAND_PREFIX:"=% %EXECUTABLE_PATH% %*
    %EXECUTABLE_PATH% %*
    ```
    **�������**
    `execute.bat [options]`

## 3. ʹ�÷���

������ɺ󣬿��� build Ŀ¼���ҵ����ɵĿ�ִ���ļ� RVM��ʹ�������������г���

```bash
RVM.exe [options]
```

## 4. ����˵��

1. **��־����**
   - `--help (-h)`����ʾ������Ϣ���˳���
   - `--version (-v)`����ʾ�汾��Ϣ���˳���
   - `--vs-check (-vc)`����ʾ����汾��ָ�� RSI �ļ��İ汾��Ϣ���˳���
   - `--run (-r)`������ָ���Ŀ�ִ���ļ���
   - `--comp (-c)`������ָ����Դ�ļ���
   - `--time-info (-ti)`������ִ��ʱ����Ϣ�����
   - `--debug (-d, -db)`�����õ���ģʽ��
   - `--rvm-work-directory (-rwd)`��ָ�� RVM Ŀ¼Ϊ����Ĺ���Ŀ¼��
   - `--precomp-link (-pcl, -pl)`��ָ��Ԥ���� RSI �ļ���·����

2. **ѡ�����**
   - `--help-option (-ho)`��ָ������ѡ������ơ�
   - `--comp-level (-cl)`��ָ�����뼶��
   - `--target (-tar, -t)`��ָ��������Ŀ���ļ�·����
   - `--archive (-arc, -a)`��ָ�������ļ������·����
   - `--working-dir (-wd)`��ָ�������Ĺ���Ŀ¼��
   - `--output-redirect (-or)`��ָ������ض����λ�á�
   - `--precomp-link-dir (-pcld, -pld)`��ָ��Ԥ���� RSI �ļ���·����

3. **�����������ϵ**
   - `--run` �� `--comp` ���⡣
   - `--rvm-work-directory` �� `--working-dir` ���⡣
   - `--help`��`--version` �� `--run`��`--comp`��`--debug` ��ѡ��⡣
   - `--help-option` ������ `--help`��
   - `--run` �� `--comp` ������ `--target`��
   - `--debug` ������ `--run`��
   - `--vs-check` ������ `--target`��

## 5. ����ṹ

��Ŀ��Ҫ�����¼���������ɣ�

- `rvm_main.cpp`����������ļ��������ʼ��������������ִ����Ӧ�Ĳ�����
- `ra_base.cpp/h`��������͹��ߵ�ʵ�֣��� `RVM_IO`��`RVM_Error` �ȡ�
- `ra_core.cpp/h`�����Ĺ��ܵ�ʵ�֣����ļ��������ڴ����ȡ�
- `ra_parser.cpp/h`�������������ʵ�֣���������Ԥ����ָ������ȡ�
- `ra_utils.cpp/h`�����ߺ�����ʵ�֣����ļ���ȡ���ַ�������ȡ�
- `rvm_ris.cpp/h`��ָ���ʵ�֣���˷�ָ��ļ���ȡָ��ȡ�

## 6. ������

���������й����лᲶ�񲢴�������쳣������������﷨�����ļ���ȡ����ȡ�������Ϣ�����������̨��ָ��������ļ��У������û����е��Ժ��Ų����⡣

## 7. RA ���Թ淶
��ϸ�� RA ���Թ淶�ĵ�����Ŀ�е� `document/RALanguage.md` �ļ���