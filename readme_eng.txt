         Wal commander 0.16.1 GitHub Edition
    https://github.com/corporateshark/WalCommander
=======================================================

Dual pane file manager with

- internal terminal emulator
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

	Run once:
		cd libtester
		./libconf.create
		cd ..

Build:
			
		make clean
		make all
		sudo make install
