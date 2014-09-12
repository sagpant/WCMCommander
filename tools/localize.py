#!/usr/bin/python3

import os
import codecs

ExcludeDirs  = [ "cmake", "Debug", "libssh2" ]
ExcludeFiles = [ " " ]
ExcludeWords = [ "xxx" ]

TotalTextStrings = 0

Words = codecs.open( "../wcm/install-files/share/wcm/lang/ltext.ru", "r", "utf-8" ).readlines()
Dictionary = []

for Index, Line in enumerate(Words):
	if Line.find( "id " ) == 0:
		S = Line[ Line.find( "\"" )+1 : -2 : ]
		Dictionary.append( S )
#		print( S )

def ProcessLine(Index, Line, FileName):
	global TotalTextStrings
	Pos = Line.find( "_LT(" )
	Text = Line[Pos+4::]
	Pos = Text.find( "\"" )
	if Pos < 0: return
	Text = Text[Pos+1::]
	EndPos = Text.find( "\"" )
	OutStr = Text[:EndPos:]
#	print( "   ", Index, ":", OutStr )
#	print( OutStr )
	IsInDict = False
	for Word in Dictionary:
		if OutStr == Word: IsInDict = True
	if not IsInDict:
		print( "#" + FileName + ":" + str(Index) )
		print( "id \"" + OutStr + "\"" )
		print( "txt \"" + "\"" )
		print()
#		print( "   ", OutStr )
		TotalTextStrings += 1
	if Text.find( "_LT(" ) >= 0:
		ProcessLine( Index, Text, FileName )

def ScanFile(Name, Path):
	global TotalTextStrings
	FullName = os.path.join(Path,Name)
	FileNamePrinted = False
	FileLines = open( FullName ).read().split( '\n' )
	for Index, Line in enumerate(FileLines):
		Line = Line.strip()

		HasQuotes = Line.find( "\"" ) >= 0
		HasLOCSTR = Line.find( "_LT(" ) >= 0

		if HasQuotes and HasLOCSTR:
			Excluded = False
			for Word in ExcludeWords:
				if Line.find( Word ) >= 0:
					Excluded = True
					break
			if Excluded: continue

			if not FileNamePrinted:
#				print( FullName )
				FileNamePrinted = True
			ProcessLine( Index, Line, Name )		

def Scan(Path):
	for Item in os.listdir( Path ):
		ItemPath = os.path.join(Path, Item)
		if os.path.isdir(ItemPath): 
			if Item in ExcludeDirs: continue
			Scan( os.path.join( Path, Item ) )
		elif os.path.isfile( ItemPath ): 
			if Item in ExcludeFiles: continue
			IsCPP = Item.endswith( ".cpp" )
			IsH   = Item.endswith( ".h" )
			if IsCPP or IsH:
				ScanFile( Item, Path )

Scan( "../wcm" )

if TotalTextStrings > 0:
	print( "Total non-localized strings:", TotalTextStrings )
