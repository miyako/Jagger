//%attributes = {"invisible":true}
$dict:=cs:C1710.Dict.new("jumandic-7.0-20130310")
If (Not:C34($dict.exists))
	$dict.install()
End if 

var $Jagger : cs:C1710.Jagger
$Jagger:=cs:C1710.Jagger.new()

$model:=Folder:C1567(fk desktop folder:K87:19).folder("custom-model")
$user:=File:C1566("/RESOURCES/user")  //csv,utf8,no empty lines
$JAG:=File:C1566("/RESOURCES/train.JAG")  //csv,utf8,no empty lines

$Jagger.train($model; $dict.file; $user; $JAG)  //new model is automatically selected

$text:="QUIT 4Dコマンドは、カレントの4Dアプリケーションを終了してデスクトップに戻ります。"
$status:=$Jagger.tokenize($text)

SET TEXT TO PASTEBOARD:C523(JSON Stringify:C1217($status; *))