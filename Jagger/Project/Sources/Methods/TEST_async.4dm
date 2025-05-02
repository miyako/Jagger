//%attributes = {"invisible":true}
#DECLARE($params : Object)

If (Count parameters:C259=0)
	
	CALL WORKER:C1389("J"; Current method name:C684; {})
	
Else 
	
	var J : cs:C1710.Jagger
	If (J=Null:C1517)
		J:=cs:C1710.Jagger.new()
	End if 
	
	$text:="Jagger は、辞書に基づく単語分割のための最長一致法と、機械学習の分類器の事前計算に着想を得た、特徴パターンに基づく高速・高精度・省メモリの形態素解析器です。"
	$status:=J.split($text; True:C214)
	
	//J.quit()
	
End if 