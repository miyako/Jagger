Class extends _HTTP

property name : Text
property folder : 4D:C1709.Folder
property URL : Text
property get : Boolean

Class constructor($name : Text)
	
	Super:C1705()
	
	This:C1470.name:=$name
	//%W-550.26
	This:C1470._repositoryURL:="https://github.com/miyako/Jagger"
	This:C1470._repositoryReleasesURL:=This:C1470._repositoryURL+"/releases"
	This:C1470.URL:=This:C1470._repositoryReleasesURL+"/download/"+This:C1470._encodeURIComponent(This:C1470.name)+"/model.zip"
	//%W+550.26
	
	var $project : Text
	$project:=File:C1566(Structure file:C489; fk platform path:K87:2).name
	var $folder : 4D:C1709.Folder
	$folder:=Folder:C1567(fk user preferences folder:K87:10).parent.folder($project).folder("model").folder(This:C1470.name)
	$folder.create()
	
	This:C1470.folder:=$folder
	
Function install() : cs:C1710._Model
	
	CALL WORKER:C1389("ghdownload"; This:C1470._download; This:C1470)
	
	return This:C1470
	
Function _download($model : cs:C1710._Model)
	
	$request:=4D:C1709.HTTPRequest.new($model.URL; $model)
	
Function onResponse($request : 4D:C1709.HTTPRequest; $event : Object)
	
	var $response : Object
	$response:=$request.response
	If ($response.status=200)
		If ($request.dataType="blob")
			$dst:=This:C1470.folder.parent
			
			var $body : 4D:C1709.Blob
			$body:=$response.body
			$file:=$dst.file(This:C1470.name+".zip")
			$file.setContent($body)
			
			This:C1470.folder.delete(Delete with contents:K24:24)
			$archive:=ZIP Read archive:C1637($file)
			var $folders : Collection
			$folders:=$archive.root.folders()
			If ($folders.length#0)
				$folders.first().copyTo($dst)
			End if 
		End if 
		
	End if 
	
Function get exists : Boolean
	
	If (This:C1470.folder.exists)
		
		var $file : Text
		For each ($file; ["patterns"; "patterns.c2i"; "patterns.da"; "patterns.fs"; "patterns.p2f"])
			If (This:C1470.folder.file($file).exists)
				continue
			Else 
				return False:C215
			End if 
		End for each 
	Else 
		return False:C215
	End if 
	
	return True:C214