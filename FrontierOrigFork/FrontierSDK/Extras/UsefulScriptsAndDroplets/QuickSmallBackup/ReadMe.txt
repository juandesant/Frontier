
Quick Small Backup script

Over time, I�ve evolved a script that conveniently backs up a Think C source code project. I put a copy of this script in each source code folder. Whenever I want a quick, small backup, I run this script. It requires a little setup. You need StuffIt 3.0 to get compressed archives. The payoff is easy backups with low cost in disk space. Dave Winer, 8/22/93

Setting up
---------

1.  Copy this script into a source code folder for one of your Think C projects.

2.  Create a folder somewhere else, call it Source Code Backups. Create an alias to this folder, move the alias into your source code folder. Rename the alias Backup Folder.

3.  Launch Frontier or Frontier Runtime. 

4.  If you want compressed backups, be sure you have StuffIt 3.0 or greater. If you�ve never run a Frontier script that drives StuffIt, launch the StuffIt.Frontier install file that came with StuffIt. The script will work even if you don�t have StuffIt.

5.  Double-click on the quickSmallBackup script.

How it works
------------

It copies everything but:
		1.  Any file or folder with �(don't back up)� in its name.
		2.  Think C project files (they�re huge, and reproducible)
		3.  Applications (they�re large, and reproducible)
		4.  (You can customize this as you wish, the criteria are in the filter subroutine)

...into your backups folder (pointed to by an alias in this folder named Backup Folder)

It creates a folder named �backup #xxx� where xxx is a serial number. It scans the folder for all files that contain the string �backup #�, pops the number off the end and comares it against the highest number it has seen so far. When the loop completes, it adds 1 to the highest number. That number is xxx.

After copying the folder with the files/folders that match the criteria above, it launches StuffIt and replaces the backup folder it created with a StuffIt archive with the same name, with �.sit� appended at the end.

� copyright 1993, UserLand Software, Inc. UserLand Software is the developer of the UserLand Frontier scripting system. The company is located at 400 Seaport Court, Redwood City, CA 94063. 415-369-6600, 415-369-6618 (fax). UserLand, Frontier, Frontier Runtime and Frontier Extras are trademarks of UserLand Software, Inc. Other product names may be trademarks or registered trademarks of their owners.

Email: userland.dts@applelink.apple.com. If you�re an AppleLink user, check out the UserLand Discussion Board under the Third Parties icon. CompuServe users enter GO USERLAND at any ! prompt. The UserLand Forum is in the Computing Support section on CompuServe. On America On-Line, enter the keyword USERLAND. Comments, questions and suggestions are welcome! 
