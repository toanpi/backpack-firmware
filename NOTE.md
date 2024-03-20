source third_party/pigweed/bootstrap.sh 

west build -p -b qemu_cortex_m3 -t run app/

# From the root of the zephyr repository
west build -b 96b_nitrogen samples/hello_world

west build -p -b decawave_dwm1001_dev app/

west build -p -b decawave_dwm1001_dev -t run app/

