wmdp introduction:
    This is a embedded system develop platform created by WM team.

wmdp advantage:
    1, friendly configurable interface
        the source code is managed with "SCONS" with the help of python.
    2, Object-Oriented Style
        i want to bring in "OOP" to this platform, such as we can use "oop" to develop device driver, just like linux device driver.
        ......

arch principal:
    1) the whole software is divided into 5 layers, application, modules, main, driver and cpulib.
    2) Each layer can and can only call the layers below it and the partners in the same layer.
        For example, lib_basic is located at the bottom layer, it could be called by every layer.

folders description:
    /doc        documents
    /include    header files
    /drivers    common useful routines and device independed driver
    /components modules such as tcp/ip, fatfs, finsh shell
    /libcpu     device drivers
    /boards     user application layer, related to real board
    /main       os schedule


