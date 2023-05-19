# GO

This programme was written in 1991, and finds files or directories.

GO uses my DOS windowing library to present the file and diretories that are found, and the up and down arrow keys are used to move through the list. Pressing enter takes you to the selection.

I have included all the original source code here, and it should be possible to compile and link with my [windowing library](https://github.com/rhulse/dos-asm-windowing-library).

It was originally distributed through a BBS. Kids, ask you parents what that is.

The idea was based on code written by Michael Holmes & Bob Flanders, and published in PC Magazine, and that is included as `cdxx.c` in this repository.

**Note that CDXX.C is copyright, and is **NOT** licensed under the MIT license.**

## Original Documentation

Files in this zip.

- GO.EXE -- The Go executable
- GO.DOC -- This file

GO may not be sold nor may it be included with any package that is sold without the permision of the author.

GO may be freely distributed, so long as ALL files, that is GO.EXE, GO.DOC, are provided in unaltered form and only a minimal fee is charged.

This utility is designed to find a matching directory or filename within a directory structure and then do a CD to it. For example say you are in the ROOT (`C:\`) and want to move to a dir called `c:\wordstar\files` then just type GO FILES and you will be there. If there are multiple subdirectories or files you will be given a display of matching files and directories to choose from. Use the up and down arrows to select the file or directory you require and press ENTER.

Pressing ESCape returns you to the directory you started from.

It will also move to a directory in which a file you want resides. If you are looking for a file type GO [filename]. If a DOS wildcard is used then GO will look at filenames only otherwise it will still look at directories as well. You can also use the -F (/F) switch to achieve the same effect. To search directory names only then put a \ before the name.

`GO \DOS`

GO will check to see if any sub-directory from where your are now or any directory from the root , match first. If you wish to check the whole disk use the /s ( search all ) switch

REMEMBER THAT GO LOOKS FOR ANY MATCHING DIRECTORY OR FILENAME

Correct Usage : GO [d:][\\]pathname [-switch]

The following are legal switches.
Only one can be used at a time.

`-F`
GO only searches for matching files

`-H`
GO includes hidden and system files in the search.

`-O`
GO looks for One ( the first it comes to ) matching string only ( for use in batch files )

`-C`
GO lets you check multiple matches if they occur in one directory only

`-S`
GO is forced to search the whole disk. It will not check sub-directories from where you are or the root first as it normally does.

### Examples

`GO _.exe`
Lists all the \_.exe files with thier directories

`GO g*.*`
Lists all files starting with g

`GO c:`
Lists all available directories on C: drive

`GO \foo`
Does a CD to the directory foo

`GO foo`
lists all foo.\_ files and \foo directories

`GO _.sys /h`
List all \_.sys files ( including system and hidden ones )

`GO foo /f`
Looks only for files that match foo

`GO _.bak`
/o Only looks for the first match and does a CD to it.

`GO d:help`
Looks for help on d: drive

`GO compress /c`
Will show all compress.\* files even if they are in the same directory.

`GO typing /s`
Ignore the sub-directory c:\typing if it exists and search the whole disk. If nothing else is found a CD to typing will be done.

Typing `GO` on its own gets help.

I will try and make other improvments as time allows. If any bugs pop up please let me know.

Thank you to those who have given suggestions...they will be incorporated in future releases.

Suggestions can be left for me on ANDREWS FOLLY 339-126 or 339-887

Version 2 posted on ANDREWS FOLLY, Wellington, New Zealand
Node 1 (00644-339-126)
Node 2 (00644-339-887)

### Version History

#### Ver. 2.21

- GO now will check the CURRENT , PARENT and ROOT directories for a matching DIRECTORY before searching. This can be overriden with the /s switch ( SEARCH ALL ) or file switch ( FILE search only )..

#### Ver. 2.20

- Direct sceen writes have been sped up significantly. ( up to 200 % !! )
- Now uses snow free screen writes on all CGA cards.
- Small bug fix in screen routine.
- Changes to a text mode if a graphics mode is active when run.
- Search code tightened..resulting in faster searches on slower machines.
- Cursor left on screen to show DOS critical error messages. GO is aware of the errors and exits properly if one is not resolved.
- Go is now DESQview aware.

Recompiled with Microsoft C ver 5.1 by Fermanagh Consultants NZ Ltd.
WN , New Zealand.
Phone (04) 764-538

#### Ver. 2.04

- If there were 5 matching strings in only one directory they were all displayed.
- GO now does not show multiple matches if they are all in the same directory.
- If you still want to check a directory before you GO there, add the /c ( choose ) switch.
- /s forces GO to scan the whole disk. This is to stop GO doing a CD to c:\typing when you want to find the file c:\utils\mine\typing.exe

#### Ver. 2.03

- Occasional problem with search when specifying a drive.
  Related to size of target......fixed.

#### Ver. 2.02

- A couple of complaints from users with BIG hard disks that GO locks up.
  GO used some statically allocated buffers. I had not expected for users to type
  GO _._ or \*.exe ...especially on BIG hard disks.
  GO now dynamically allocates memory for ALL data thus increasing the amount of matching files and directories that can be displayed. The amount will differ for each individual hard disk...depending on the length of directories. If GO runs out of memory it exits with an appropriate message.

#### Ver. 2.01

- -O mode added as some people were using GO in a batch file in a simmilar way to SWEEP
  eg. GO _.bak to find the first occurence of _.bak files. They would then be DELeted and GO \*.bak executed again. Long winded but apparently SWEEP + DEL can leave lost clusters sometimes !!
- Small bug fix in screen routine.

#### Ver. 2.00

- If there happens to be more than one matching file or directory name then GO will now give you the chance to pick which directory you GO to.

#### Ver. 1.16

- Recoded and recompiled with Microsoft Quick C.
- Programme is now an .EXE file.
- Custom Virtual Screen module used for help and Searching message.

#### Ver. 1.15

- Code tightened and execution sped up even more.

#### Ver. 1.13

- Displays search directories on screen. Converted to .COM file.

#### Ver. 1.10

- Looked at subdirectories first (faster) and will accept command lines that are the same as CD eg. GO ..\..\utils.
- GO will now run on ETHERNET and NOVELL networks.

#### Ver. 1.03 (Experimental)

- Used WindowPro library for the help screen. (NOT RELEASED too big !!! 60k )

#### Ver. 1.02

- Allowed GO to change to the parent directory by typing [GO ..]
  ie. GO then a space then two full stops.

#### Ver. 1.01

- Gave no print out of NEW DIRECTORY after HELP or not finding anything.

### DISCLAIMER

I use this programme all the time and I have endevored to make sure that there is nothing wrong with it and that no damage will be caused by its use.

However, as with all Public Domain programs NO RESPONSIBILITY can be taken
for any damages, direct or consequential.

                        Richard Hulse
                         07 July 1991
