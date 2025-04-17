//%attributes = {"invisible":true}
If (False:C215)
	$dictionary:=File:C1566("/RESOURCES/custom-model/dict")
	$zip:=cs:C1710._Zip.new($dictionary)
	$zip.create()
End if 

$dict:=cs:C1710.Dict.new("jumandic-7.0-20130310")
If (Not:C34($dict.exists))
	$dict.install()
End if 