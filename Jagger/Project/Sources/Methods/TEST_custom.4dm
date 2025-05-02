//%attributes = {"invisible":true}
var $Jagger : cs:C1710.Jagger
$Jagger:=cs:C1710.Jagger.new()

$model:=Folder:C1567(fk desktop folder:K87:19).folder("custom-model")

If ($model.exists)
	$Jagger.model:=$model
	$text:="QUIT 4Dコマンドは、カレントの4Dアプリケーションを終了してデスクトップに戻ります。"
	$status:=$Jagger.tokenize($text)
	SET TEXT TO PASTEBOARD:C523(JSON Stringify:C1217($status; *))
End if 