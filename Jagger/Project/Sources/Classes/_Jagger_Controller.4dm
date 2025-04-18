Class extends _CLI_Controller

Class constructor($CLI : cs:C1710._CLI)
	
	Super:C1705($CLI)
	
Function get instance() : cs:C1710.Jagger
	
	//%W-550.26
	return This:C1470._instance
	//%W+550.26
	
Function onData($worker : 4D:C1709.SystemWorker; $params : Object)
	
Function onDataError($worker : 4D:C1709.SystemWorker; $params : Object)
	
Function onResponse($worker : 4D:C1709.SystemWorker; $params : Object)
	
	$values:=[]
	
	If ($worker.dataType="text")
		Case of 
			: (This:C1470.instance.reloadModel)
				
				//%W-550.26
				This:C1470.instance.model:=This:C1470.instance._newModel
				//%W+550.26
				
			: (This:C1470.instance.segmentationOnly)
				$values:=Split string:C1554($worker.response; " "; sk trim spaces:K86:2 | sk ignore empty strings:K86:1)
			Else 
				var $lines : Collection
				$lines:=Split string:C1554($worker.response; This:C1470.instance.EOL)
				
				For each ($line; $lines)
					$v:=Split string:C1554($line; "\t")
					If ($v.length=2)
						$values.push({pos: $v[0]; dic: Split string:C1554($v[1]; ",")})
					Else 
						break
					End if 
				End for each 
		End case 
	End if 
	
	This:C1470.instance.data:=$values
	
Function onError($worker : 4D:C1709.SystemWorker; $params : Object)
	
Function onTerminate($worker : 4D:C1709.SystemWorker; $params : Object)
	