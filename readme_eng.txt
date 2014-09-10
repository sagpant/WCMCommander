         Wal commander 0.16.2 GitHub Edition
    https://github.com/corporateshark/WalCommander
=======================================================

Dual panel file manager with

- terminal emulator
- built-on editor
- built-ontext viewer
- virtual file systems: smb, ftp, sftp
	
=======================================================
				
Install:

	Required libraries:
		libX11-dev
			
	Optional libaries:
		libssh2-dev 
		libsmbclient-dev
		libfreetype-dev

Build (FreeBSD, Linux, OS X, MinGW):

		cd wcm
		make all -B
		sudo make install
