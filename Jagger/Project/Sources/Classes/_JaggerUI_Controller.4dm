Class extends _Jagger_Controller

Class constructor($CLI : cs:C1710._CLI)
	
	Super:C1705($CLI)
	
Function onDataError($worker : 4D:C1709.SystemWorker; $params : Object)
	
	Super:C1706.onDataError($worker; $params)
	
Function onData($worker : 4D:C1709.SystemWorker; $params : Object)
	
	Super:C1706.onData($worker; $params)
	
Function onResponse($worker : 4D:C1709.SystemWorker; $params : Object)
	
	Super:C1706.onResponse($worker; $params)
	
	If (Form:C1466#Null:C1517)
		
		Case of 
			: (This:C1470.instance.segmentationOnly)
				
				$col:=[]
				
				For each ($value; This:C1470.instance.data)
					$col.push({pos: $value})
				End for each 
				
			Else 
				
				$col:=This:C1470.instance.data
				
		End case 
		
		Form:C1466.tokens:={col: $col; sel: Null:C1517; item: Null:C1517; pos: Null:C1517}
		
	End if 
	
Function onTerminate($worker : 4D:C1709.SystemWorker; $params : Object)
	
	If (Form:C1466#Null:C1517)
		
		If (This:C1470.complete)
			
		End if 
		
	End if 