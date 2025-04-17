//%attributes = {"invisible":true}
var $Jagger : cs:C1710.Jagger
$Jagger:=cs:C1710.Jagger.new()

$model:=Folder:C1567(fk desktop folder:K87:19).folder("custom-model")

$dict:=File:C1566("/RESOURCES/custom-model/dict")  //csv,utf8,no empty lines
$user:=File:C1566("/RESOURCES/custom-model/user")  //csv,utf8,no empty lines
$JAG:=File:C1566("/RESOURCES/custom-model/train.JAG")  //csv,utf8,no empty lines

$Jagger.train($model; $dict; $user; $JAG)  //new model is automatically selected

$text:="QUIT 4Dコマンドは、カレントの4Dアプリケーションを終了してデスクトップに戻ります。"
$status:=$Jagger.tokenize($text)

SET TEXT TO PASTEBOARD:C523(JSON Stringify:C1217($status; *))