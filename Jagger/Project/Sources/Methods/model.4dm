//%attributes = {"invisible":true}
If (False:C215)
	//create a zip archive of a model folder (c2i,da,fs,p2f)
	$model:=Folder:C1567("/RESOURCES/model/kyoto+kwdlc")
	$zip:=cs:C1710._Zip.new($model)
	$zip.create()
End if 

//download a model from github
$model:=cs:C1710.Model.new("kyoto+kwdlc")
If (Not:C34($model.exists))
	$model.install()
End if 