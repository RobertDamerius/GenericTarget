![](documentation/img/icon.svg)

# Generic Target

The **GenericTarget** project is designed to facilitate the deployment of MATLAB/Simulink models to linux-based real-time systems.
For applications that rely solely on network interfaces, it provides a library and tools to build hardware-agnostic real-time applications with MATLAB/Simulink.
Any computer can serve as a target system as long as it runs a Linux operating system with the **PREEMPT_RT** patch.


## Features
- **Hardware Agnostic**: The target applications runs entirely in the userspace, making it available to all platforms that are supported by Linux.
- **Cross-Platform**: While focused on Linux, it's possible to deploy a target application to different platforms with minor adjustments.
- **Multi-Threading**: Automatically schedules tasks with multiple sampling rates, running them in parallel with appropriate thread priorities.
- **UDP Communication**: Offers direct socket access beyond Simulink UDP blocks, allowing retrieval of sender addresses, dynamic multicast joining, socket error handling, and more.
- **Timing**: Provides real-time access to system time in multiple formats, including local time, UTC and UNIX timestamp.
- **Reliable Data Recording**: Continuously saves data to minimize loss in case of a system failure.
- **System Monitoring**: Provides access to thermal zones, CPU and task overloads and task execution time for performance analysis.


## Download and Installation
Clone this repository or download the [latest release](https://github.com/RobertDamerius/GenericTarget/releases/latest).
Run `install.m` in MATLAB to add the toolbox to MATLAB's path.
For optimal compatibility with your host system, it is recommended to **rebuild all driver blocks** during the installation.

You can always rebuild driver blocks by running:

```
GT.BuildDrivers()
```

For alternative installation methods using Simulink projects, refer to the documentation.


## Getting Started
Open [readme.html](readme.html) in a web browser to access the HTML documentation.
It provides a detailed overview of the **Generic Target Toolbox**, including setup instructions for the target computer and a guide to creating your first real-time application.


## Documentation
A comprehensive HTML-based documentation for offline use is available within this repository, including:

- Detailed setup instructions
- Configuration examples for real-time applications
- Best practices for optimizing performance

To access the documentation, open the [readme.html](readme.html) file with a browser.
Once you installed the Generic Target Toolbox in MATLAB, you can also open the documentation by running the following command in the MATLAB command window:

```
GT.OpenDocumentation()
```


## Repository Structure
The structure of this repository is as follows.

| File / Directory   | Description                                                          |
| :----------------- | :------------------------------------------------------------------- |
| core               | contains source code and template files for the Generic Target Core  |
| documentation      | contains the HTML-based documentation                                |
| library            | contains the MATLAB/Simulink library "GenericTarget" (**>= R2025a**) |
| packages           | contains the MATLAB package "GT"                                     |
| resources          | contains project definition files for the MATLAB/Simulink project    |
| GenericTarget.prj  | Simulink project file                                                |
| install.m          | MATLAB script to add the Generic Target Toolbox to the MATLAB path   |
| LICENSE            | license information                                                  |
| readme.html        | forwards to the documentation                                        |
| README.md          | this file                                                            |


## License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

