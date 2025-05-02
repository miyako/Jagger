Class extends _CLI

property model : 4D:C1709.Folder
property data : Collection
property segmentationOnly : Boolean
property reloadModel : Boolean

Class constructor($controller : 4D:C1709.Class)
	
	Super:C1705("jagger"; $controller=Null:C1517 ? cs:C1710._Jagger_Controller : $controller)
	
	//%W-550.26
	
	$model:=cs:C1710.Model.new("kyoto+kwdlc")
	If (Not:C34($model.exists))
		$model.install()
	End if 
	
	This:C1470.JAGGER_DEFAULT_MODEL:=This:C1470.expand($model.folder)
	This:C1470._model:=This:C1470.JAGGER_DEFAULT_MODEL
	
	Case of 
		: (Is macOS:C1572)
			This:C1470._executableNameForTraining:="train_jagger"
		: (Is Windows:C1573)
			This:C1470._executableNameForTraining:="train_jagger.exe"
	End case 
	
	This:C1470._executableFileForTraining:=File:C1566(This:C1470.currentDirectory.file(This:C1470._executableNameForTraining).path)
	
	If (This:C1470.executableFileForTraining.exists)
		Case of 
			: (Is macOS:C1572)
				This:C1470._executablePathForTraining:=This:C1470.currentDirectory.file(This:C1470.executableNameForTraining).path
			: (Is Windows:C1573)
				This:C1470._executablePathForTraining:=This:C1470.currentDirectory.file(This:C1470.executableNameForTraining).platformPath
		End case 
	End if 
	
	//%W+550.26
	
Function get executableNameForTraining() : Text
	
	//%W-550.26
	return This:C1470._executableNameForTraining
	//%W+550.26
	
Function get executableFileForTraining() : 4D:C1709.File
	
	//%W-550.26
	return This:C1470._executableFileForTraining
	//%W+550.26
	
Function get executablePathForTraining() : Text
	
	//%W-550.26
	return This:C1470._executablePathForTraining
	//%W+550.26
	
Function set model($model : 4D:C1709.Folder)
	
	If ($model=Null:C1517) || (Not:C34(OB Instance of:C1731($model; 4D:C1709.Folder)))
		return 
	End if 
	
	var $file : Text
	For each ($file; ["patterns"; "patterns.c2i"; "patterns.da"; "patterns.fs"; "patterns.p2f"])
		If ($model.file($file).exists)
			continue
		Else 
			return 
		End if 
	End for each 
	
	//%W-550.26
	This:C1470._model:=This:C1470.expand($model)
	//%W+550.26
	
Function get model() : 4D:C1709.Folder
	
	//%W-550.26
	return This:C1470._model
	//%W+550.26
	
Function terminate()
	
	This:C1470.controller.terminate()
	
Function get worker() : 4D:C1709.SystemWorker
	
	return This:C1470.controller.worker
	
Function get controller() : cs:C1710._Jagger_Controller
	
	//%W-550.26
	return This:C1470._controller
	//%W+550.26
	
Function tokenize($text : Text; $async : Boolean) : Object
	
	If ($async)
		return This:C1470._run($text; False:C215)
	Else 
		return This:C1470._get($text; False:C215)
	End if 
	
Function split($text : Text; $async : Boolean) : Object
	
	If ($async)
		return This:C1470._run($text; True:C214)
	Else 
		return This:C1470._get($text; True:C214)
	End if 
	
Function train($model : 4D:C1709.Folder; $dict : 4D:C1709.File; $user : 4D:C1709.File; $JAG : 4D:C1709.File; $async : Boolean)
	
	If ($model=Null:C1517) || (Not:C34(OB Instance of:C1731($model; 4D:C1709.Folder)))
		return 
	End if 
	
	$model.create()
	
	If ($dict=Null:C1517) || (Not:C34(OB Instance of:C1731($dict; 4D:C1709.File))) || (Not:C34($dict.exists))
		return 
	End if 
	
	If ($user=Null:C1517) || (Not:C34(OB Instance of:C1731($user; 4D:C1709.File))) || (Not:C34($user.exists))
		return 
	End if 
	
	If ($JAG=Null:C1517) || (Not:C34(OB Instance of:C1731($JAG; 4D:C1709.File))) || (Not:C34($JAG.exists))
		return 
	End if 
	
	$command:=This:C1470.escape(This:C1470.executablePathForTraining)
	$command+=" -m "
	If (Is macOS:C1572)
		$command+=This:C1470.escape(This:C1470.expand($model).path)
	Else 
		$command+=This:C1470.escape(This:C1470.expand($model).platformPath)
	End if 
	
	$command+=" -d "
	If (Is macOS:C1572)
		$command+=This:C1470.escape(This:C1470.expand($dict).path)
	Else 
		$command+=This:C1470.escape(This:C1470.expand($dict).platformPath)
	End if 
	
	$command+=" -u "
	If (Is macOS:C1572)
		$command+=This:C1470.escape(This:C1470.expand($user).path)
	Else 
		$command+=This:C1470.escape(This:C1470.expand($user).platformPath)
	End if 
	
	$command+=" "
	If (Is macOS:C1572)
		$command+=This:C1470.escape(This:C1470.expand($JAG).path)
	Else 
		$command+=This:C1470.escape(This:C1470.expand($JAG).platformPath)
	End if 
	
	//%W-550.26
	This:C1470._newModel:=$model
	//%W+550.26
	
	This:C1470.reloadModel:=True:C214
	This:C1470.controller.execute($command)
	This:C1470.worker.closeInput()
	
	If ($async)
		return 
	Else 
		This:C1470.worker.wait()
	End if 
	
Function quit() : cs:C1710.Jagger
	
	If (This:C1470.worker#Null:C1517) && (Not:C34(This:C1470.worker.terminated))
		This:C1470.worker.closeInput()
	End if 
	
	return This:C1470
	
Function _run($text : Text; $segmentationOnly : Boolean) : cs:C1710.Jagger
	
	If (This:C1470.worker=Null:C1517) || (This:C1470.worker.terminated)
		$command:=This:C1470.escape(This:C1470.executablePath)
		
		$command+=" -m "
		If (Is macOS:C1572)
			$command+=This:C1470.escape(This:C1470.model.path)
		Else 
			$command+=This:C1470.escape(This:C1470.model.platformPath)
		End if 
		
		If ($segmentationOnly)
			$command+=" -w "
		End if 
		
		This:C1470.reloadModel:=False:C215
		This:C1470.segmentationOnly:=$segmentationOnly
		This:C1470.controller.execute($command)
	End if 
	
	If ($text#"")
		This:C1470.worker.postMessage($text+"\n")
	End if 
	
	return This:C1470
	
Function _get($text : Text; $segmentationOnly : Boolean) : Object
	
	If ($text="")
		return {data: []}
	End if 
	
	This:C1470._run($text; $segmentationOnly)
	
	This:C1470.quit().worker.wait()
	
	return {data: This:C1470.data}