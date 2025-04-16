Class extends _URI

property method : Text
property timeout : Integer
property dataType : Text
property decodeData : Boolean

Class constructor()
	
	Super:C1705()
	
	This:C1470.method:="GET"
	This:C1470.timeout:=300  //seconds
	This:C1470.dataType:="blob"
	This:C1470.decodeData:=False:C215
	
Function onData($request : 4D:C1709.HTTPRequest; $event : Object)
	
Function onError($request : 4D:C1709.HTTPRequest; $event : Object)
	
	KILL WORKER:C1390
	
Function onHeaders($request : 4D:C1709.HTTPRequest; $event : Object)
	
Function onResponse($request : 4D:C1709.HTTPRequest; $event : Object)
	
Function onTerminate($request : 4D:C1709.HTTPRequest; $event : Object)
	
	KILL WORKER:C1390