## How To Launch
This directory contains the built software as well as source files, protocol files and data recordings. To start the application manually, run the following command:
```
sudo ./$TARGET_README_PRODUCT_NAME$
```

If the output should be printed to the console instead of being redirected into a text file, run
```
sudo ./$TARGET_README_PRODUCT_NAME$ --console
```

If the application is to be started without waiting for termination, the following command can be executed.
```
sudo nohup ./$TARGET_README_PRODUCT_NAME$ &
```

To stop a possibly running application, run
```
sudo ./$TARGET_README_PRODUCT_NAME$ --stop
```

To recompile the generic target application, run
```
make clean && make -j8
```
