Class extends _HTTP

property name : Text
property folder : 4D:C1709.Folder
property file : 4D:C1709.File
property URL : Text

Class constructor($name : Text)
	
	Super:C1705()
	
	This:C1470.name:=$name
	//%W-550.26
	This:C1470._repositoryURL:="https://github.com/miyako/Jagger"
	This:C1470._repositoryReleasesURL:=This:C1470._repositoryURL+"/releases"
	This:C1470.URL:=This:C1470._repositoryReleasesURL+"/download/"+This:C1470._encodeURIComponent(This:C1470.name)+"/dict.zip"
	//%W+550.26
	
	var $project : Text
	$project:=File:C1566(Structure file:C489; fk platform path:K87:2).name
	var $folder : 4D:C1709.Folder
	$folder:=Folder:C1567(fk user preferences folder:K87:10).parent.folder($project).folder("dict").folder(This:C1470.name)
	$folder.create()
	
	This:C1470.folder:=$folder
	
	$file:=$folder.file("dict")
	
	This:C1470.file:=$file
	
Function install() : cs:C1710.Dict
	
	CALL WORKER:C1389("ghdownload"; This:C1470._download; This:C1470)
	
	return This:C1470
	
Function _download($dict : cs:C1710.Dict)
	
	$request:=4D:C1709.HTTPRequest.new($dict.URL; $dict)
	
Function onResponse($request : 4D:C1709.HTTPRequest; $event : Object)
	
	var $response : Object
	$response:=$request.response
	If ($response.status=200)
		If ($request.dataType="blob")
			$dst:=This:C1470.folder
			
			var $body : 4D:C1709.Blob
			$body:=$response.body
			$file:=This:C1470.folder.parent.file(This:C1470.name+".zip")
			$file.setContent($body)
			
			This:C1470.file.delete()
			$archive:=ZIP Read archive:C1637($file)
			var $files : Collection
			$files:=$archive.root.files(fk ignore invisible:K87:22)
			For each ($file; $files)
				$file.copyTo($dst)
			End for each 
		End if 
		
	End if 
	
Function get exists : Boolean
	
	return (This:C1470.file#Null:C1517) && (This:C1470.file.exists)