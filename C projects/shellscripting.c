#!/bin/tcsh

# 
if ($#argv < 2) then
    echo "Usage: deldirs name pathname"
    exit 1
endif

#
set dirname = $argv[1]
shift

#
foreach pathname($*)
# 
    if (!- d $pathname / $dirname) then
        echo "The directory entered: $dirname , does not exist"
        #continue
    else
        # 
        echo "Found directory $pathname/$dirname"
        # 
        echo - n "Do you want to delete this directory? (y/n) "
        set answer = $ <
        # If confirmed, delete all of the directory's contents
        if ($answer == "y") then
            echo "Deleting $pathname/$dirname..."
            rm - rf $pathname / $dirname
        endif
    endif
end
