property model : 4D:C1709.Folder
property data : Collection

Class extends _CLI

property text : Object

Class constructor($controller : 4D:C1709.Class)
	
	Super:C1705("jagger"; $controller=Null:C1517 ? cs:C1710._Jagger_Controller : $controller)
	
	//%W-550.26
	
	$model:=cs:C1710._Model.new("kyoto+kwdlc")
	If (Not:C34($model.exists))
		$model.install()
	End if 
	
	This:C1470.JAGGER_DEFAULT_MODEL:=This:C1470.expand($model.folder)
	This:C1470._model:=This:C1470.JAGGER_DEFAULT_MODEL
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
	
Function words($text : Text; $segmentationOnly : Boolean) : Collection
	
	If ($text#"")
		$command:=This:C1470.escape(This:C1470.executablePath)
		$command+=" -m "
		If (Is macOS:C1572)
			$command+=This:C1470.escape(This:C1470.model.path)
		Else 
			$command+=This:C1470.escape(This:C1470.model.platformPath)
		End if 
		
		If ($segmentationOnly)
			$command+=" -w"
		End if 
		
		This:C1470.controller.execute($command)
		This:C1470.worker.postMessage($text)
		This:C1470.worker.closeInput()
		This:C1470.worker.wait()
		
		return This:C1470.data
		
	End if 