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
		libfreetype-dev			
		libssh2-1-dev
		libsmbclient-dev


Build (FreeBSD, Linux, OS X, MinGW):

		cd wcm
		make all -B
		sudo make install


Mac OS X installation way:

	Install Quartz from http://xquartz.macosforge.org/landing/
	Install brew http://brew.sh/
	brew install --HEAD https://gist.githubusercontent.com/KonstantinKuklin/9001d95fa30fecf3231c/raw/walcommander.rb
