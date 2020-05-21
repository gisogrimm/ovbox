# get device mac address:
test -e cfg/devicename || ./udpmirror/getmacaddr > cfg/devicename
devicename=$(cat cfg/devicename)
test -z "${devicename}" && ./udpmirror/getmacaddr > cfg/devicename
devicename=$(cat cfg/devicename)

# get hostname:
thisbox=$(hostname)

# read host-specific configuration files:
test -e cfg/$thisbox && . cfg/$thisbox

# extract box number:
thisboxno=$(echo $thisbox|sed 's/[^[0-9]]*//g')

export thisbox
export thisboxno
export devicename
