property text : Text

Class extends _Form

Class constructor
	
	Super:C1705()
	
	$window:=Open form window:C675("Jagger")
	DIALOG:C40("Jagger"; This:C1470; *)
	
Function onLoad()
	
	Form:C1466.Jagger:=cs:C1710.Jagger.new(cs:C1710._JaggerUI_Controller)
	
	Form:C1466.tokens:={col: []; sel: Null:C1517; item: Null:C1517; pos: Null:C1517}
	
	This:C1470.text:="Jagger は、辞書に基づく単語分割のための最長一致法と、機械学習の分類器の事前計算に着想を得た、特徴パターンに基づく高速・高精度・省メモリの形態素解析器です。"
	
	Form:C1466.Jagger4D:=cs:C1710.Jagger.new(cs:C1710._JaggerUI_Controller)
	
	$model:=Folder:C1567(fk desktop folder:K87:19).folder("custom-model")
	
	If ($model.exists)
		
		Form:C1466.Jagger4D.model:=$model
		
	Else 
		
		$dict:=cs:C1710.Dict.new("jumandic-7.0-20130310")
		
		If (Not:C34($dict.exists))
			$dict.install()
		End if 
		
		$model:=Folder:C1567(fk desktop folder:K87:19).folder("custom-model")
		$user:=File:C1566("/RESOURCES/user")  //csv,utf8,no empty lines
		$JAG:=File:C1566("/RESOURCES/train.JAG")  //csv,utf8,no empty lines
		
		Form:C1466.Jagger4D.train($model; $dict.file; $user; $JAG; True:C214)  //new model is automatically selected
		
	End if 
	
Function split($custom : Boolean)
	
	If ($custom)
		Form:C1466.Jagger4D.split(This:C1470.text; True:C214)
	Else 
		Form:C1466.Jagger.split(This:C1470.text; True:C214)
	End if 
	
Function tokenize($custom : Boolean)
	
	If ($custom)
		Form:C1466.Jagger4D.tokenize(This:C1470.text; True:C214)
	Else 
		Form:C1466.Jagger.tokenize(This:C1470.text; True:C214)
	End if 