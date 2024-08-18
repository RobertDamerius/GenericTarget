![](documentation/img/icon.svg)

# Generic Target

The Generic Target Toolbox can be used to run MATLAB/Simulink models (**>= R2024a**) on a target computer in realtime.
The target computer can be any computer that has at least one network interface and has a Linux operating system with
the PREEMPT_RT realtime patch installed. All Simulink blocks that allow code generation can be used. For low-level hardware
interfaces like network interfaces, the Generic Target Toolbox comes with its own blocks.

The directory structure of this repository is as follows.

| File / Directory   | Description                                                                                                    |
| :----------------- | :------------------------------------------------------------------------------------------------------------- |
| documentation      | contains the documentation that has been created with [SimpleDoc](https://github.com/RobertDamerius/SimpleDoc) |
| library            | contains the MATLAB/Simulink library "GenericTarget" (**>= R2024a**)                                           |
| packages           | contains the MATLAB package "GT"                                                                               |
| resources          | contains project definition files for the MATLAB/Simulink project                                              |
| GenericTarget.prj  | Simulink project file                                                                                          |
| install.m          | MATLAB script to add the Generic Target Toolbox to the MATLAB path                                             |
| LICENSE            | license information                                                                                            |
| readme.html        | forwards to the documentation                                                                                  |
| README.md          | this file                                                                                                      |


> **Note**<br>
> The toolbox is currently in the stage of development and testing.

### Major Changes
| Date / Version  | Description                                                                                                                                                                   |
| :-------------- | :---------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 2021-01-26      | Initial release                                                                                                                                                               |
| 2021-02-08      | File logging has been updated. Bus signals can now be recorded directly (scalar doubles only).                                                                                |
| 2021-03-19      | File logging has been extended to complete bus objects with multi-dimensional signals of common data types.                                                                   |
| 2021-05-31      | Custom code directories can now be included during code generation.                                                                                                           |
| 2022-05-18      | Updated interface code generation to support newer MATLAB versions.                                                                                                           |
| 2022-10-10      | Model and simulation time are renamed to hardware and software time, respectively.                                                                                            |
| 2022-10-11      | CPU overload is replaced by task overload. Missed ticks of the base timer are interpreted as CPU overloads.                                                                   |
| 2023-02-28      | Lowest valid thread priority is set to 1.                                                                                                                                     |
| 2023-03-23      | Default port of application socket is set to 44000.                                                                                                                           |
| 2023-05-24      | Added automatic network initialization, added protocol files, added backtrace for DEBUG mode, updated timing blocks, added prebuild driver blocks for Windows and Linux.      |
| 2023-05-26      | Renamed target execution time to model execution time. Updated directory names.                                                                                               |
| 2023-07-26      | Updated deployment procedure and target directory structure. Data recordings can be named. Additional compiler flags can be set. Target can be started on specific CPU cores. |
| 2023-10-04      | Added support for UDP broadcast. Network interface names can now be used to specify multicast interfaces.                                                                     |
| 2023-11-21      | Enabled option supportsMultipleExecInstances for simulink blocks. Added prefixes to prevent name collision with user code.                                                    |
| 2024-03-19      | Added option to bind network devices for UDP unicast. Updated the block interface for UDP unicast and multicast blocks. Updated library to MATLAB R2023b.                     |
| 2024-05-20      | Time source of timestamps for UDP receive blocks can be selected via the block mask.                                                                                          |
| 2024-05-24      | Added support for periodic partitions and sample offset. Added stop execution block. Task name is now used for task execution time and task overload blocks.                  |
| 2024-06-08      | Model step functions are not executed in a delayed manner after a task overload but are called at their next possible time slot.                                              |

## Getting Started
Look at the HTML-based documentation by opening the ``readme.html`` file with a browser. There you will find a more
detailed description of the Generic Target Toolbox as well as instructions on how to prepare the target computer and create
your first realtime application.
