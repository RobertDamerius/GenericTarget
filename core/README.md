## How To Launch

This directory contains the built software as well as source files and data recordings. To start the application manually, run the following command in this directory:

```bash
sudo ./$TARGET_README_PRODUCT_NAME$
```

To redirect the output to a text file, run

```bash
sudo ./$TARGET_README_PRODUCT_NAME$ >> out.txt 2>&1
```

If the application is to be started without waiting for termination in a console window, execute the following command:

```bash
sudo nohup ./$TARGET_README_PRODUCT_NAME$ >> out.txt 2>&1 &
```

## How To Stop

To stop a possibly running application, run

```bash
sudo ss -lpx src '@$TARGET_SOCKET_NAME$' | grep -o 'pid=[0-9]\+' | cut -d= -f2 | xargs -r sudo kill -15
```

## How To Recompile

To recompile the generic target application, run

```bash
make clean && make -j8
```

## How To Autostart

To autostart the application create a `systemd` service

```bash
sudo nano /etc/systemd/system/generictarget.service
```

and paste the following configuration

```ini
[Unit]
Description=GenericTarget
After=network.target remote-fs.target

[Service]
Type=simple
User=root

# Optional: Add a brief startup delay if the boot storm causes overloads
ExecStartPre=/bin/sleep 5

# Ensure log file exists and is owned by your regular user before starting
ExecStartPre=/bin/touch /home/UserName/PathToApplication/out.txt
ExecStartPre=/bin/chown UserName:UserName /home/UserName/PathToApplication/out.txt

# Run application
ExecStart=/usr/bin/taskset 0x0f /home/UserName/PathToApplication/$TARGET_README_PRODUCT_NAME$

# Systemd native logging (append mode)
StandardOutput=append:/home/UserName/PathToApplication/out.txt
StandardError=inherit

# Restart behavior in case of crashes
Restart=no

[Install]
WantedBy=multi-user.target
```

where `UserName`, `PathToApplication` and possibly `taskset 0x0f` have to be modified.
Reload systemd, enable the service to start automatically on boot:

```bash
sudo systemctl daemon-reload
sudo systemctl enable generictarget.service
sudo systemctl start generictarget.service
```
