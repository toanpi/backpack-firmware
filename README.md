# SenSeeAir Firmware Development

## Environment
Update project dependencies:
```shell
west update
```

Change directory to the project:
```shell
cd app/backpack-uwb-node 
```

## Building the project

Rebuild the project:
```shell
west build -b decawave_dwm1001_dev  -p
```

Build the project:
```shell
west build -b decawave_dwm1001_dev
```

After building the project, you can flash the firmware to the board:
```shell
west flash
```
