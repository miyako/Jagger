Class extends _Jagger_Controller

Class constructor($CLI : cs:C1710._CLI)
	
	Super:C1705($CLI)
	
Function onDataError($worker : 4D:C1709.SystemWorker; $params : Object)
	
	Super:C1706.onDataError($worker; $params)
	
Function onData($worker : 4D:C1709.SystemWorker; $params : Object)
	
	Super:C1706.onData($worker; $params)
	
	If (Form:C1466#Null:C1517)
		
		$col:=This:C1470.instance.data
		
		Form:C1466.tokens:={col: $col; sel: Null:C1517; item: Null:C1517; pos: Null:C1517}
		
	End if 
	
Function onResponse($worker : 4D:C1709.SystemWorker; $params : Object)
	
Function onTerminate($worker : 4D:C1709.SystemWorker; $params : Object)
	
	If (Form:C1466#Null:C1517)
		
		This:C1470.instance.quit()
		
	End if 