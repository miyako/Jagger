property model : 4D:C1709.Folder

Class constructor($model : Object)
	
	If ((OB Instance of:C1731($model; 4D:C1709.Folder)) || (OB Instance of:C1731($model; 4D:C1709.File))) && ($model.exists)
		
		This:C1470.model:=$model
		
	End if 
	
Function create() : cs:C1710._Zip
	
	If (This:C1470.model#Null:C1517)
		
		var $zip : Object
		
		Case of 
			: (OB Instance of:C1731(This:C1470.model; 4D:C1709.Folder))
				$zip:={files: This:C1470.model.files(fk ignore invisible:K87:22)}
			: (OB Instance of:C1731(This:C1470.model; 4D:C1709.File))
				$zip:={files: [This:C1470.model]}
		End case 
		
		$zip.compression:=ZIP Compression LZMA:K91:9
		$zip.level:=5
		$dst:=This:C1470.model.parent.file(This:C1470.model.name+".zip")
		$status:=ZIP Create archive:C1640($zip; $dst; ZIP Without enclosing folder:K91:7)
		
	End if 
	
	return This:C1470