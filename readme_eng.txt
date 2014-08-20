				Wal commander 0.16.1


	Dual pane file manager 

	has:
	internal terminal emulator
	internal editow
	text viewer
	vfs: smb, ftp, sftp
	
	
	need external libs (only):
				libX11 
				libsmbclient 
				libssh2
				libfreetype
	
				
install:
		required libraries:
			libX11-dev
			libssh2-dev 
			libsmbclient-dev
			libfreetype-dev
			
		make clean

		make
		
		sudo make install
		
	ps:
		libssh2-dev, libsmbclient-dev, libfreetype-dev - optional libs


			
			
		