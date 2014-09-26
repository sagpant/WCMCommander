         Wal commander 0.17.0 GitHub Edition
    https://github.com/corporateshark/WalCommander
=======================================================

Dual panel multi-platform open source file manager
for Windows, Linux, FreeBSD and OS X with

- terminal emulator
- built-in editor with syntax highlighting (c/c++, java, pascal, latex, glsl, perl, php, python, sh, sql, xml)
- built-in text viewer
- virtual file systems: smb, ftp, sftp
- file associations
	
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
