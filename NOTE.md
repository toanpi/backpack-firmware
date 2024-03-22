git clone https://pigweed.googlesource.com/pigweed/pigweed third_party/pigweed

source third_party/pigweed/bootstrap.sh 

source ${PW_ROOT}/activate.sh

source third_party/pigweed/activate.sh

source ~/working/project/backpack/backpack-firmware/third_party/pigweed/activate.sh

pw package install zephyr

west build -p -b qemu_cortex_m3 -t run app/

# From the root of the zephyr repository
west build -b 96b_nitrogen samples/hello_world


west build -p -b decawave_dwm1001_dev app/


west build -p -b decawave_dwm1001_dev app/uwb-node/


