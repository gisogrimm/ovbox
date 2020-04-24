# get hostname:
thisbox=$(hostname)
test -e cfg/$thisbox && . cfg/$thisbox

# extract box number:
thisboxno=$(echo $thisbox|sed 's/[^[0-9]]*//g')

export thisbox
export thisboxno
