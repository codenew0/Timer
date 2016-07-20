# Timer
A timer under linux and gtk3 environment
This project is under linux and gtk3.
If the version of your gtk is more than 3.14, you will get some warnings about GtkStutusIcon. 
Because it has been deprecated since version 3.14. But you can ignore these warning.

# Building
1. Put all these files into one folder
2. change your working directory to this folder
2. chmod +x make.sh
3. ./make.sh
4. ./main

# Usage
 Press "Add" button to add a timer set. Then input timing numbers into the first three entries.
 You can input up to 5 numbers into one entry. And all the numbers will be transformed to a time number.
 ex: you input 10, 68, 3600 into the 1st, 2nd, 3rd entry, you will get a "12:08:00" timer.
 The 4th entry is to remind you after the time ends. 
 Input some remarks to the 4th entry, you can hear a warning sound and get a notification with your remarks after the time ends.

