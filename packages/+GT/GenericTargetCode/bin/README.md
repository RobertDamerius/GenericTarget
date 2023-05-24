## How To Launch
This directory contains the built software as well as protocol files and data recordings. To start the application manually, run the following command:
```
sudo ./GenericTarget
```

If the output should be printed to the console instead of being redirected into a text file, run
```
sudo ./GenericTarget --console
```

If the application is to be started without waiting for termination, the following command can be executed.
```
sudo nohup ./GenericTarget &
```

To stop a possibly running application, run
```
sudo ./GenericTarget --stop
```

