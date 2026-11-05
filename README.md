# Generic Target

<div align="center"><img src="documentation/img/icon.svg" alt="Generic Target Logo" width="20%"><br><br></div>

Generic Target deploys MATLAB/Simulink models as real-time applications on Linux. It can run on any hardware capable of running Linux; real-time operation requires a **PREEMPT_RT** kernel. The toolbox is designed for applications that communicate with external devices over network interfaces, with support for protocols such as UDP and TCP.

## 📋 Requirements

### Host Computer

- MATLAB R2025b or newer.
- Simulink and Embedded Coder.
- SSH/SCP client for deploying to a remote target.
- A C/C++ compiler configured in MATLAB with `mex -setup` to build or rebuild driver blocks.

### Target Computer

- Linux. A Linux desktop or Windows with WSL can be used for testing.
- A **PREEMPT_RT** kernel for real-time operation.
- **g++ 13 or newer**, with support for the **C++20** standard.

## ✨ Features

- **Hardware agnostic**: Runs in Linux userspace and supports Linux-compatible hardware.
- **Multithreading**: Schedules tasks with multiple sample rates using thread priorities.
- **UDP communication**: Provides direct socket access, sender addresses, dynamic multicast joining, and socket error handling.
- **TCP communication**: Manages connection lifecycle, disconnections, and automatic reconnection.
- **Timing**: Provides system time in local, UTC, and UNIX timestamp formats.
- **Data recording**: Writes data continuously to reduce loss in case of system failure.
- **System monitoring**: Exposes thermal zones, CPU and task overloads, and task execution time.

## 📦 Install

Clone this repository or download the [latest release](https://github.com/RobertDamerius/GenericTarget/releases/latest). In MATLAB, run `install.m` to add the toolbox to the MATLAB path. For compatibility with your host system, rebuilding the driver blocks during installation is recommended:

```matlab
GT.BuildDrivers()
```

For installation through MATLAB projects, see the documentation.

## 🚀 Get Started

Open [readme.html](readme.html) in a browser for target setup instructions and a guide to creating and deploying an application.

## 📖 Documentation

The offline HTML documentation covers setup, configuration and deployment. Open [readme.html](readme.html) in a browser, or launch it from MATLAB:

```matlab
GT.OpenDocumentation()
```

## 📁 Repository Overview

| File / Directory    | Description                                 |
| :------------------ | :------------------------------------------ |
| `core/`             | Generic Target Core source and templates    |
| `documentation/`    | offline HTML documentation and source files |
| `library/`          | MATLAB/Simulink library                     |
| `packages/`         | MATLAB `GT` package                         |
| `resources/`        | MATLAB project definition files             |
| `GenericTarget.prj` | MATLAB project                              |
| `install.m`         | adds the toolbox to the MATLAB path         |
| `LICENSE`           | license information                         |
| `readme.html`       | entry point for the HTML documentation      |
| `README.md`         | this file                                   |

## 📄 License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.
