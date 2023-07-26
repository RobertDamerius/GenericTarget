![](doc/img/icon.svg)

# Generic Target

The Generic Target Toolbox can be used to run MATLAB/Simulink models (**>= R2022b**) on a target computer in realtime.
The target computer can be any computer that has at least one network interface and has a Linux operating system with
the PREEMPT_RT realtime patch installed. All Simulink blocks that allow code generation can be used. For low-level hardware
interfaces like network interfaces, the Generic Target Toolbox comes with its own blocks.

The directory structure of this repository is as follows.

| File / Directory   | Description                                                                                                    |
| :----------------- | :------------------------------------------------------------------------------------------------------------- |
| documentation      | contains the documentation that has been created with [SimpleDoc](https://github.com/RobertDamerius/SimpleDoc) |
| library            | contains the MATLAB/Simulink library "GenericTarget" (**>= R2022b**)                                           |
| packages           | contains the MATLAB package "GT"                                                                               |
| readme.html        | forwards to the documentation                                                                                  |
| install.m          | MATLAB script to install the Generic Target Toolbox                                                            |
| LICENSE            | license information                                                                                            |
| README.md          | this file                                                                                                      |


> **Note**<br>
> The toolbox is currently in the first stage of development and testing.

### Revision History
| Date        | Version  | Description                                                                                                                                                                   |
| :---------- | :------- | :---------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 2021-01-26  | 1.0      | Initial release                                                                                                                                                               |
| 2021-02-08  | 1.1      | File logging has been updated. Bus signals can now be recorded directly (scalar doubles only).                                                                                |
| 2021-03-19  | 1.2      | File logging has been extended to complete bus objects with multi-dimensional signals of common data types.                                                                   |
| 2021-05-31  | 1.3      | Custom code directories can now be included during code generation.                                                                                                           |
| 2022-05-18  | 1.4      | Updated interface code generation to support newer MATLAB versions.                                                                                                           |
| 2022-10-10  | 1.5      | Model and simulation time are renamed to hardware and software time, respectively.                                                                                            |
| 2022-10-11  | 1.6      | CPU overload is replaced by task overload. Missed ticks of the base timer are interpreted as CPU overloads.                                                                   |
| 2023-02-28  | 1.7      | Lowest valid thread priority is set to 1.                                                                                                                                     |
| 2023-03-23  | 1.8      | Default port of application socket is set to 44000.                                                                                                                           |
| 2023-05-24  | 2.0      | Added automatic network initialization, added protocol files, added backtrace for DEBUG mode, updated timing blocks, added prebuild driver blocks for Windows and Linux.      |
| 2023-05-26  | 2.1      | Renamed target execution time to model execution time. Updated directory names.                                                                                               |
| 2023-07-26  | 2.2      | Updated deployment procedure and target directory structure. Data recordings can be named. Additional compiler flags can be set. Target can be started on specific CPU cores. |


## How To Start
Look at the HTML-based documentation by opening the ``readme.html`` file with a browser. There you will find a more
detailed description of the Generic Target Toolbox as well as instructions on how to prepare the target computer and create
your first realtime application.
