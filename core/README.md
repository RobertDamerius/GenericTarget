## How To Launch
This directory contains the built software as well as source files, protocol files and data recordings. To start the application manually, run the following command in this directory:
```
sudo ./$TARGET_README_PRODUCT_NAME$
```

If the output should be printed to the console instead of being redirected into a text file, run
```
sudo ./$TARGET_README_PRODUCT_NAME$ --console
```

If the application is to be started without waiting for termination in a console window, execute the following command:
```
sudo nohup ./$TARGET_README_PRODUCT_NAME$ &
```

To stop a possibly running application, run
```
sudo ./$TARGET_README_PRODUCT_NAME$ --stop
```

## How To Recompile
To recompile the generic target application, run
```
make clean && make -j8
```

## How To Autostart
If you want to autostart the application via a cron job, edit the crontab via
```
sudo crontab -e
```
and add the line
```
@reboot /home/UserName/PathToApplication/$TARGET_README_PRODUCT_NAME$ >/dev/null 2>&1
```
where UserName and PathToApplication have to be modified.
If the application should be started on specific cpu cores, e.g. 0,1,2,3, modify the line to
```
@reboot taskset 0x0f /home/UserName/PathToApplication/$TARGET_README_PRODUCT_NAME$ >/dev/null 2>&1
```
