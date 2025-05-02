Class extends _CLI_Controller

property stdout : Text

Class constructor($CLI : cs:C1710._CLI)
	
	Super:C1705($CLI)
	
	This:C1470.stdout:=""
	
Function get instance() : cs:C1710.Jagger
	
	//%W-550.26
	return This:C1470._instance
	//%W+550.26
	
Function onData($worker : 4D:C1709.SystemWorker; $params : Object)
	
	If ($worker.dataType="text")
		
		This:C1470.stdout+=$params.data
		
		Case of 
			: (This:C1470.instance.segmentationOnly)
				$pos:=Position:C15(This:C1470.instance.EOL+This:C1470.instance.EOL; This:C1470.stdout; 1; $len; *)
			Else 
				$pos:=Position:C15("EOS"+This:C1470.instance.EOL+This:C1470.instance.EOL; This:C1470.stdout; 1; $len; *)
		End case 
		
		If ($pos#0)
			$values:=[]
			$data:=Substring:C12(This:C1470.stdout; 1; $pos+$len)
			This:C1470.stdout:=Substring:C12(This:C1470.stdout; $pos+$len)
			Case of 
				: (This:C1470.instance.segmentationOnly)
					$values:=Split string:C1554($data; This:C1470.instance.EOL; sk trim spaces:K86:2 | sk ignore empty strings:K86:1)
				Else 
					var $lines : Collection
					$lines:=Split string:C1554($data; This:C1470.instance.EOL; sk trim spaces:K86:2 | sk ignore empty strings:K86:1)
					
					For each ($line; $lines)
						$v:=Split string:C1554($line; "\t")
						If ($v.length=2)
							If (Length:C16($v[0])#0)
								$values.push({pos: $v[0]; dic: Split string:C1554($v[1]; ",")})
							End if 
						Else 
							break
						End if 
					End for each 
			End case 
			This:C1470.instance.data:=$values
		Else 
			
		End if 
	End if 
	
Function onDataError($worker : 4D:C1709.SystemWorker; $params : Object)
	
Function onResponse($worker : 4D:C1709.SystemWorker; $params : Object)
	
	If ($worker.dataType="text")
		Case of 
			: (This:C1470.instance.reloadModel)
				
				//%W-550.26
				This:C1470.instance.model:=This:C1470.instance._newModel
				//%W+550.26
				
		End case 
		
	End if 
	
Function onError($worker : 4D:C1709.SystemWorker; $params : Object)
	
Function onTerminate($worker : 4D:C1709.SystemWorker; $params : Object)
	