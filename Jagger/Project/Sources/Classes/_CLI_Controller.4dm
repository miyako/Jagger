property dataType : Text
property encoding : Text
property variables : Object
property currentDirectory : 4D:C1709.Folder
property hideWindow : Boolean

Class constructor($CLI : cs:C1710._CLI)
	
	//use default event handler if not defined in subclass definition
	For each ($event; ["onData"; "onDataError"; "onError"; "onResponse"; "onTerminate"])
		If (Not:C34(OB Instance of:C1731(This:C1470[$event]; 4D:C1709.Function)))
			This:C1470[$event]:=This:C1470._onEvent
		End if 
	End for each 
	
	This:C1470.dataType:="text"
	This:C1470.encoding:="UTF-8"
	This:C1470.variables:={}
	This:C1470.currentDirectory:=Folder:C1567(Temporary folder:C486; fk platform path:K87:2)
	This:C1470.hideWindow:=True:C214
	
	//%W-550.26
	This:C1470._instance:=$CLI
	This:C1470._commands:=[]
	This:C1470._worker:=Null:C1517
	This:C1470._complete:=False:C215  //flag to indicate whether we have queued commands
	//%W+550.26
	
Function get commands()->$commands : Collection
	
	//%W-550.26
	$commands:=This:C1470._commands
	//%W+550.26
	
Function get complete()->$complete : Boolean
	
	//%W-550.26
	$complete:=This:C1470._complete
	//%W+550.26
	
Function get instance()->$instance : cs:C1710._CLI
	
	//%W-550.26
	$instance:=This:C1470._instance
	//%W+550.26
	
Function get worker()->$worker : 4D:C1709.SystemWorker
	
	//%W-550.26
	$worker:=This:C1470._worker
	//%W+550.26
	
	//MARK:-public methods
	
Function execute($command : Variant)
	
	var $commands : Collection
	
	Case of 
		: (Value type:C1509($command)=Is text:K8:3)
			$commands:=[$command]
		: (Value type:C1509($command)=Is collection:K8:32)
			$commands:=$command
	End case 
	
	If ($commands#Null:C1517) && ($commands.length#0)
		
		//%W-550.26
		This:C1470._commands.combine($commands)
		
		If (This:C1470._worker=Null:C1517)
			This:C1470._onResponse:=This:C1470.onResponse
			This:C1470.onResponse:=This:C1470._onExecute
			This:C1470._onTerminate:=This:C1470.onTerminate
			This:C1470.onTerminate:=This:C1470._onComplete
			This:C1470._execute()
		End if 
		//%W+550.26
		
	End if 
	
Function terminate()
	
	This:C1470._abort()
	
	//%W-550.26
	If (This:C1470._worker#Null:C1517)
		This:C1470._worker.terminate()
	End if 
	//%W+550.26
	
	This:C1470._terminate()
	
	//MARK:-private methods
	
Function _onEvent($worker : 4D:C1709.SystemWorker; $params : Object)
	
	Case of 
		: ($params.type="data") && ($worker.dataType="text")
			
		: ($params.type="data") && ($worker.dataType="blob")
			
		: ($params.type="error")
			
		: ($params.type="termination")
			
		: ($params.type="response")
			
	End case 
	
Function _onExecute($worker : 4D:C1709.SystemWorker; $params : Object)
	
	//%W-550.26
	If (This:C1470._commands.length=0)
		This:C1470._abort()
	Else 
		This:C1470._execute()
	End if 
	
	If (OB Instance of:C1731(This:C1470._onResponse; 4D:C1709.Function))
		This:C1470._onResponse.call(This:C1470; $worker; $params)
	End if 
	//%W+550.26
	
Function _execute()
	
	//%W-550.26
	This:C1470._complete:=False:C215
	This:C1470._worker:=4D:C1709.SystemWorker.new(This:C1470._commands.shift(); This:C1470)
	//%W+550.26
	
Function _onComplete($worker : 4D:C1709.SystemWorker; $params : Object)
	
	//%W-550.26
	If (OB Instance of:C1731(This:C1470._onTerminate; 4D:C1709.Function))
		This:C1470._onTerminate.call(This:C1470; $worker; $params)
	End if 
	//%W+550.26
	
	If (This:C1470.complete)
		This:C1470._terminate()
	End if 
	
Function _abort()
	
	//%W-550.26
	This:C1470._complete:=True:C214
	This:C1470._commands.clear()
	//%W+550.26
	
Function _terminate()
	
	//%W-550.26
	This:C1470.onResponse:=This:C1470._onResponse
	This:C1470.onTerminate:=This:C1470._onTerminate
	This:C1470._worker:=Null:C1517
	//%W+550.26